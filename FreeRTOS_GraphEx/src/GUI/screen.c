#include "screen.h"
#include <stddef.h>

#define NB_PISTE 4

static int XScreenSize;
static int YScreenSize;
static int ui_box_width;
static int ui_box_height;

void display_score(int score)
{
	//clear
	GUI_SetColor(COLOR_TOP_HEADER_BG);
	GUI_FillRect(60, 10, ui_box_width - 2, 10 + 16);

	GUI_SetColor(COLOR_TEXT_SCORE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_Font16B_ASCII);

	GUI_DispStringAt("SCORE:", 6, 10);
	GUI_DispDecAt(score, 60, 10, 4);
}

void display_musique_name(char * musiqueName)
{
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_Font16B_ASCII);
	GUI_SetColor(GUI_WHITE);

	GUI_RECT r;
	r.x0 = ui_box_width + 5;
	r.y0 = 3;
	r.x1 = XScreenSize - 5;
	r.y1 = ui_box_height;

	GUI_DispStringInRectWrap(musiqueName, &r, GUI_TA_LEFT | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);
}

void display_header()
{
	GUI_SetBkColor(COLOR_BG);

	GUI_SetColor(COLOR_TOP_HEADER_BG);
	GUI_FillRect(3, 3, XScreenSize - 1, ui_box_height);

	GUI_SetColor(COLOR_TOP_HEADER_BORDER);
	GUI_DrawRect(3, 3, XScreenSize - 1, ui_box_height);
	GUI_DrawLine(ui_box_width, 3, ui_box_width, ui_box_height);

	display_score(0);
	display_musique_name("Au clair de la lune");
}

void display_note_section()
{
	int step = XScreenSize / NB_PISTE;

	GUI_SetColor(COLOR_TILE_INACTIVE);
	for (int i = 0; i < NB_PISTE; i++)
	{
		int x0 = i * step + 1;
		int x1 = (i + 1) * step - 1;
		GUI_FillRect(x0, ui_box_height + 1, x1, YScreenSize - 3);
	}

	GUI_SetColor(COLOR_LANE_LINE);
	for (int i = 1; i < NB_PISTE; i++)
	{
		int x = i * step;
		GUI_DrawLine(x, ui_box_height + 1, x, YScreenSize - 3);
	}

	GUI_SetColor(COLOR_TOP_HEADER_BORDER);
	GUI_DrawLine(3, ui_box_height, 3, YScreenSize - 3);
	GUI_DrawLine(XScreenSize - 1, ui_box_height, XScreenSize - 1, YScreenSize - 3);
	GUI_DrawLine(3, YScreenSize - 3, XScreenSize - 1, YScreenSize - 3); ///horizontal line bas

	GUI_SetColor(COLOR_HIT_LINE);
	GUI_DrawLine(3, YScreenSize - 40, XScreenSize - 1, YScreenSize - 40); // note line
}

void Draw_note_line()
{
	GUI_SetColor(COLOR_HIT_LINE);
	GUI_DrawLine(3, YScreenSize - 40, XScreenSize - 6, YScreenSize - 40);
}

void Draw_bottom_line()
{
	GUI_SetColor(COLOR_TOP_HEADER_BORDER);
	GUI_DrawLine(3, YScreenSize - 3, XScreenSize - 1, YScreenSize - 3);
}

void clear_piste()
{
	int step = XScreenSize / NB_PISTE;

	GUI_SetColor(COLOR_TILE_INACTIVE);
	for (int i = 0; i < NB_PISTE; i++)
	{
		int x0 = i * step + 1;
		int x1 = (i + 1) * step - 1;
		GUI_FillRect(x0, ui_box_height + 1, x1, YScreenSize - 3);
	}
}

void clear_note(DrawNote_t val)
{
    int step = XScreenSize / NB_PISTE;

    GUI_RECT clipRect;
    clipRect.x0 = 0;
    clipRect.y0 = ui_box_height + 1;
    clipRect.x1 = XScreenSize;
    clipRect.y1 = YScreenSize - 3;

    WM_SetUserClipRect(&clipRect);

    int x_min = val.piste_num * step + 4;
    int x_max = (val.piste_num + 1) * step - 3;

    int y_min = val.y;
    int y_max = val.y + val.height;

    GUI_SetColor(COLOR_TILE_INACTIVE);
    GUI_FillRect(x_min, y_min, x_max, y_max);

    WM_SetUserClipRect(NULL);
}

void display_note(DrawNote_t val, int y)
{
	int step = XScreenSize / NB_PISTE;

	GUI_RECT clipRect;
	clipRect.x0 = 0;
	clipRect.y0 = ui_box_height + 1;
	clipRect.x1 = XScreenSize;
	clipRect.y1 = YScreenSize - 3;


	WM_SetUserClipRect(&clipRect);


	int x0 = val.piste_num * step + 4;
	int x1 = (val.piste_num + 1) * step - 3;
	int y0 = y;
	int y1 = y + val.height;

	GUI_SetColor(COLOR_TILE_FALLING);
	GUI_FillRect(x0, y0, x1, y1);

	WM_SetUserClipRect(NULL);
}

void display_init_screen()
{

	XScreenSize   = LCD_GetXSize();
	YScreenSize   = LCD_GetYSize();
	ui_box_width  = XScreenSize / 2;
	ui_box_height = YScreenSize / 7;

	display_header();
	display_note_section();
}

