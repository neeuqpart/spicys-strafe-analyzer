#pragma once
#include <cstdint>
#include <windows.h>

class ConVar {
public:
    float get_float() const
    {
        if (!this) return 10.0f;
        __try {
            // CSS x64 ConVar: m_pParent at 0x38, m_fValue at 0x54.
            // Replicated client variables read their registered parent's value.
            const auto parent = *reinterpret_cast<const ConVar* const*>(reinterpret_cast<uintptr_t>(this) + 0x38);
            if (!parent) return 10.0f;
            return *reinterpret_cast<const float*>(reinterpret_cast<uintptr_t>(parent) + 0x54);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return 10.0f;
        }
    }
};

class IConVar {
public:
    ConVar* get_convar(const char* name)
    {
        // VEngineCvar004::FindVar (five IAppSystem methods precede ICvar).
        using original_fn = ConVar* (*)(IConVar*, const char*);
        return (*reinterpret_cast<original_fn**>(this))[12](this, name);
    }
};
