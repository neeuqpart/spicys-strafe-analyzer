#pragma once
#include "drawing.h"
#include "../Interfaces/Interfaces.h"
#include "../zgui/zgui.hh"
#include "console.h"

unsigned long render::moon_font;

void render::setup_fonts()
{
	moon_font = Interfaces::surface->font_create();

	Interfaces::surface->set_font_glyph(moon_font, "Moon", 14, 400, 0, 0, font_flags::fontflag_none);
}

void render::draw_cursor()
{
	int x, y;
	Interfaces::surface->surface_get_cursor_pos(x, y);
	Interfaces::surface->set_drawing_color(255, 255, 255);
	Interfaces::surface->draw_filled_rectangle(x, y, 4, 4);
}

void render::line(int x, int y, int x2, int y2, color c) noexcept
{
	Interfaces::surface->set_drawing_color(c.r, c.g, c.b, c.a);
	Interfaces::surface->draw_line(x, y, x2, y2);
}

void render::circle(int x, int y, int r, int s, color col) noexcept
{
	float Step = M_PI * 2.0 / s;
	for (float a = 0; a < (M_PI * 2.0); a += Step) {
		float x1 = r * cos(a) + x;
		float y1 = r * sin(a) + y;
		float x2 = r * cos(a + Step) + x;
		float y2 = r * sin(a + Step) + y;
		Interfaces::surface->set_drawing_color(col.r, col.g, col.b, col.a);
		Interfaces::surface->draw_line(x1, y1, x2, y2);
	}
}

void render::rect(int x, int y, int x2, int y2, color c) noexcept 
{
	Interfaces::surface->set_drawing_color(c.r, c.g, c.b, c.a);
	Interfaces::surface->draw_outlined_rect(x, y, x2, y2);
}

void render::filled_rect(int x, int y, int w, int h, color c) noexcept 
{
	Interfaces::surface->set_drawing_color(c.r, c.g, c.b, c.a);
	Interfaces::surface->draw_filled_rectangle(x, y, w, h);
}

void render::line_zgui(int x, int y, int x2, int y2, zgui::color c) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	line(x, y, x2, y2, cc);
}

void render::rect_zgui(int x, int y, int x2, int y2, zgui::color c) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	rect(x, y, x2, y2, cc);
}

void render::filled_rect_zgui(int x, int y, int x2, int y2, zgui::color c) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	filled_rect(x, y, x2, y2, cc);
}

void render::text(int x, int y, zgui::color c, int font, bool center, const char* text) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	draw_string(x, y, cc, font, center, text);
}

void render::text(int x, int y, color c, int font, bool center, const char* text) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	draw_string(x, y, cc, font, center, text);
}

void render::s_text(int x, int y, color c, int font, bool center, const std::string& text) noexcept
{
	color cc = color(c.r, c.g, c.b, c.a);
	draw_string(x, y, cc, font, center, text.c_str());
}

void render::draw_string(int x, int y, color col, unsigned long font, bool center, const char* pszText, ...)
{
	if (pszText == nullptr || !font || !Interfaces::surface)
		return;

	wchar_t szString[1024] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, pszText, -1, szString, 1024);

	int width = 0, height = 0;
	Interfaces::surface->get_text_size(font, szString, width, height);

	if (center) Interfaces::surface->draw_text_pos(x - (width / 2), y);
	else Interfaces::surface->draw_text_pos(x, y);

	Interfaces::surface->draw_text_font(font);
	Interfaces::surface->set_text_color(col.r, col.g, col.b, col.a);
	Interfaces::surface->draw_render_text(szString, static_cast<int>(wcslen(szString)));
}

void render::DrawT(int X, int Y, color Color, int Font, bool Center, const char* _Input, ...)
{
	if (_Input == nullptr || !Font || !Interfaces::surface)
		return;

	wchar_t WideBuffer[1024] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, _Input, -1, WideBuffer, 1024);

	int Width = 0, Height = 0;
	if (Center)
		Interfaces::surface->get_text_size(Font, WideBuffer, Width, Height);

	Interfaces::surface->set_text_color(Color.r, Color.g, Color.b, Color.a);
	Interfaces::surface->draw_text_font(Font);
	Interfaces::surface->draw_text_pos(X - (Width / 2), Y);
	Interfaces::surface->draw_render_text(WideBuffer, static_cast<int>(wcslen(WideBuffer)));
}

void render::get_text_size(unsigned long font, const char* text, int& wide, int& tall) noexcept
{
	if (!text || !font || !Interfaces::surface) {
		wide = 0;
		tall = 0;
		return;
	}
	std::string str(text);
	std::wstring wstr(str.begin(), str.end());

	Interfaces::surface->get_text_size(font, wstr.c_str(), wide, tall);
}

void render::get_text_size(unsigned long font, std::string text, int& width, int& height) noexcept
{
	if (text.empty() || !font || !Interfaces::surface) {
		width = 0;
		height = 0;
		return;
	}
	std::string str(text.c_str());
	std::wstring wstr(str.begin(), str.end());

	Interfaces::surface->get_text_size(font, wstr.c_str(), width, height);
}

float render::get_frametime() noexcept 
{
	if (Interfaces::globals && Interfaces::globals->frame_time() > 0.0f)
		return Interfaces::globals->frame_time();
	return 1.0f / 60.0f;
}