#include "../SDK+/SDK.h"

void Indices::Initialize()
{
		// hooks
		create_move_idx = 21;
		paint_traverse_idx = 41;

		// ibaseclient
		get_client_classes = 8;

		// ivengineclient
		get_local_player = 12;
		get_screen_size = 5;
		is_in_game = 26;
		is_connected = 27;
		execute_cmd = 102;
		get_view_angles = 19;
		set_view_angles = 20;

		//onconnect
		onconnect = 30;

		// surface
		set_drawing_color = 11;
		set_text_color = 19;
		draw_filled_rectangle = 12;
		draw_outlined_rect = 14;
		draw_line = 15;
		draw_text_font = 17;
		draw_text_pos = 20;
		draw_render_text = 22;
		font_create = 66;
		set_font_glyph = 67;
		get_text_size = 75;
		get_screen_size_surface = 42;
		unlock_cursor = 61;
		lock_cursor = 62;
		get_cursor_pos = 96;
		set_cursor_always_visible = 52;

		// input_system
		enable_input = 7;

		// panel
		get_panel_name = 36;

		// c_usercmd
		command_number = 0x8;
		viewangles = 0x10;
		forwardmove = 0x1C;
		sidemove = 0x20;
		buttons = 0x28;
		mousedx = 0x3C;
		mousedy = 0x3E;

		// globals
		frame_time = 0x10;
		interval_per_tick = 0x1C;

		//ivdebug
		world_to_screen = 9;

		//w2sm
		world_to_screen_matrix = 36;

		// convar
		get_convar = 12;
}
