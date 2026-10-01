#include "utils.hpp"
c_utils g_utils;

uintptr_t c_utils::find_signature(const char* module, const char* signature)
{
	if (!module || !signature)
		return 0;

	std::vector<int> pattern;
	for (const char* cursor = signature; *cursor;) {
		if (*cursor == ' ') {
			++cursor;
			continue;
		}
		if (*cursor == '?') {
			pattern.push_back(-1);
			while (*cursor == '?')
				++cursor;
			continue;
		}
		auto hex_value = [](char value) -> int {
			if (value >= '0' && value <= '9') return value - '0';
			if (value >= 'A' && value <= 'F') return value - 'A' + 10;
			if (value >= 'a' && value <= 'f') return value - 'a' + 10;
			return -1;
		};
		const int high = hex_value(cursor[0]);
		const int low = hex_value(cursor[1]);
		if (high < 0 || low < 0)
			return 0;
		pattern.push_back((high << 4) | low);
		cursor += 2;
	}
	if (pattern.empty())
		return 0;

	const auto image_base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(module));
	if (!image_base)
		return 0;
	const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image_base);
	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;
	const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(image_base + dos->e_lfanew);
	if (nt->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	std::uintptr_t result = 0;
	const auto* section = IMAGE_FIRST_SECTION(nt);
	for (unsigned short index = 0; index < nt->FileHeader.NumberOfSections; ++index, ++section) {
		if ((section->Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0)
			continue;
		const auto start = image_base + section->VirtualAddress;
		const auto size = section->Misc.VirtualSize > section->SizeOfRawData
			? section->Misc.VirtualSize : section->SizeOfRawData;
		if (size < pattern.size())
			continue;
		for (std::size_t offset = 0; offset <= size - pattern.size(); ++offset) {
			const auto* bytes = reinterpret_cast<const std::uint8_t*>(start + offset);
			bool matches = true;
			for (std::size_t byte_index = 0; byte_index < pattern.size(); ++byte_index) {
				if (pattern[byte_index] >= 0 && bytes[byte_index] != pattern[byte_index]) {
					matches = false;
					break;
				}
			}
			if (!matches)
				continue;
			if (result != 0)
				return 0; // Ambiguous signatures must never select an arbitrary target.
			result = start + offset;
		}
	}
	return result;

	// Legacy x86 whole-image scanner retained below for historical context.
	const char* pat = signature;
	std::uintptr_t firstmatch = 0;
	const std::uintptr_t rangestart = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(module));
	MODULEINFO miModInfo;

	if (!rangestart || !GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(rangestart), &miModInfo, sizeof(MODULEINFO)))
		return 0;

	const std::uintptr_t rangeEnd = rangestart + miModInfo.SizeOfImage;

	for (std::uintptr_t pCur = rangestart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstmatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GET_BYTE(pat))
		{
			if (!firstmatch)
				firstmatch = pCur;

			if (!pat[2])
				return firstmatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;
		}
		else
		{
			pat = signature;
			firstmatch = 0;
		}
	}

	return 0u;
}

uintptr_t c_utils::resolve_rel32(uintptr_t instruction, std::size_t displacement_offset,
	std::size_t instruction_size) const
{
	const auto displacement = *reinterpret_cast<const std::int32_t*>(instruction + displacement_offset);
	return instruction + instruction_size + displacement;
}

bool c_utils::is_executable_address(uintptr_t address) const
{
	MEMORY_BASIC_INFORMATION information{};
	if (!address || VirtualQuery(reinterpret_cast<const void*>(address), &information, sizeof(information)) == 0)
		return false;
	const DWORD protection = information.Protect & 0xff;
	return information.State == MEM_COMMIT &&
		(protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
			protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY);
}

