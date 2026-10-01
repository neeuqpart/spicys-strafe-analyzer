#include <Windows.h>
#include <TlHelp32.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
struct Process {
    DWORD id{};
    std::wstring image_path;
};

std::wstring to_lower(std::wstring value)
{
    for (auto& character : value)
        character = static_cast<wchar_t>(towlower(character));
    return value;
}

bool is_css_process(const std::wstring& image_path)
{
    const auto lower_path = to_lower(image_path);
    return lower_path.ends_with(L"\\cstrike_win64.exe") &&
        lower_path.find(L"counter-strike source") != std::wstring::npos;
}

std::optional<std::wstring> image_path_for(DWORD process_id)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
    if (!process)
        return std::nullopt;

    std::wstring path(32768, L'\0');
    DWORD length = static_cast<DWORD>(path.size());
    const bool succeeded = QueryFullProcessImageNameW(process, 0, path.data(), &length) != FALSE;
    CloseHandle(process);
    if (!succeeded)
        return std::nullopt;
    path.resize(length);
    return path;
}

std::vector<Process> find_css_processes()
{
    std::vector<Process> results;
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return results;

    PROCESSENTRY32W entry{ sizeof(entry) };
    for (BOOL found = Process32FirstW(snapshot, &entry); found; found = Process32NextW(snapshot, &entry)) {
        if (_wcsicmp(entry.szExeFile, L"cstrike_win64.exe") != 0)
            continue;
        const auto image_path = image_path_for(entry.th32ProcessID);
        if (image_path && is_css_process(*image_path))
            results.push_back({ entry.th32ProcessID, *image_path });
    }
    CloseHandle(snapshot);
    return results;
}

bool same_architecture(HANDLE process)
{
    USHORT current_process_machine = IMAGE_FILE_MACHINE_UNKNOWN;
    USHORT current_native_machine = IMAGE_FILE_MACHINE_UNKNOWN;
    USHORT target_process_machine = IMAGE_FILE_MACHINE_UNKNOWN;
    USHORT target_native_machine = IMAGE_FILE_MACHINE_UNKNOWN;

    if (!IsWow64Process2(GetCurrentProcess(), &current_process_machine, &current_native_machine) ||
        !IsWow64Process2(process, &target_process_machine, &target_native_machine))
        return false;

    const USHORT current = current_process_machine == IMAGE_FILE_MACHINE_UNKNOWN ? current_native_machine : current_process_machine;
    const USHORT target = target_process_machine == IMAGE_FILE_MACHINE_UNKNOWN ? target_native_machine : target_process_machine;
    return current == target;
}

bool inject(DWORD process_id, const std::filesystem::path& dll_path)
{
    const auto absolute_path = std::filesystem::absolute(dll_path);
    if (!std::filesystem::is_regular_file(absolute_path)) {
        std::wcerr << L"DLL not found: " << absolute_path << L'\n';
        return false;
    }

    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, process_id);
    if (!process) {
        std::wcerr << L"Unable to open CSS (error " << GetLastError() << L").\n";
        return false;
    }
    if (!same_architecture(process)) {
        std::wcerr << L"Loader and CSS must both be x64.\n";
        CloseHandle(process);
        return false;
    }

    const std::wstring dll_name = absolute_path.wstring();
    const SIZE_T bytes = (dll_name.size() + 1) * sizeof(wchar_t);
    void* remote_name = VirtualAllocEx(process, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_name) {
        std::wcerr << L"Unable to allocate memory in CSS (error " << GetLastError() << L").\n";
        CloseHandle(process);
        return false;
    }

    bool succeeded = false;
    if (WriteProcessMemory(process, remote_name, dll_name.c_str(), bytes, nullptr)) {
        const auto load_library = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        HANDLE thread = load_library ? CreateRemoteThread(process, nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(load_library), remote_name, 0, nullptr) : nullptr;
        if (thread) {
            WaitForSingleObject(thread, INFINITE);
            DWORD module_handle = 0;
            succeeded = GetExitCodeThread(thread, &module_handle) && module_handle != 0;
            CloseHandle(thread);
        }
    }

    if (!succeeded)
        std::wcerr << L"Injection failed (error " << GetLastError() << L").\n";
    VirtualFreeEx(process, remote_name, 0, MEM_RELEASE);
    CloseHandle(process);
    return succeeded;
}

void print_usage(const wchar_t* executable)
{
    std::wcout << L"Usage: " << executable << L" [path-to-dll]\n";
}
}

int wmain(int argc, wchar_t* argv[])
{
    std::wstring executable_path(32768, L'\0');
    const DWORD executable_length = GetModuleFileNameW(nullptr, executable_path.data(), static_cast<DWORD>(executable_path.size()));
    if (!executable_length || executable_length == executable_path.size()) {
        std::wcerr << L"Unable to determine the loader location.\n";
        return 1;
    }
    executable_path.resize(executable_length);
    const std::filesystem::path default_dll = std::filesystem::path(executable_path).parent_path() / L"strafe analyzer.dll";
    const std::filesystem::path dll_path = argc == 2 ? argv[1] : default_dll;
    if (argc > 2) {
        print_usage(argv[0]);
        return 2;
    }

    const auto processes = find_css_processes();
    if (processes.empty()) {
        std::wcerr << L"Counter-Strike: Source was not found. Start its x64 version first.\n";
        return 1;
    }
    if (processes.size() > 1) {
        std::wcerr << L"More than one CSS process is running; close the extras and retry.\n";
        return 1;
    }

    std::wcout << L"Loading " << std::filesystem::absolute(dll_path) << L" into\n"
        << processes.front().image_path << L" (PID " << processes.front().id << L").\n";
    if (!inject(processes.front().id, dll_path))
        return 1;

    std::wcout << L"Loaded successfully.\n";
    return 0;
}
