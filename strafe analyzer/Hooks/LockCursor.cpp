#include "hooks.h"

void Hooks::LockCursor::Hook(ISurface* self)
{
	if (!Hooks::runtime_ready.load(std::memory_order_acquire))
	{
		LockCursor_original(self);
		return;
	}

    if (menu_open)
    {
        self->unlock_cursor();
        return;
    }

    LockCursor_original(self);
}
