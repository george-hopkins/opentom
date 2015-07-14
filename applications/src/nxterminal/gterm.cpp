// Copyright Timothy Miller, 1999

#include "gterm.hpp"

void GTerm::Update()
{
	update_changes();
}

void GTerm::ProcessInput(int len, unsigned char *data)
{
	int i;
	StateOption *last_state;

	data_len = len;
	input_data = data;

	while (data_len)
	{
		i = 0;
		while (current_state[i].byte != -1 && 
		       current_state[i].byte != *input_data) i++;

		// action must be allowed to redirect state change
		last_state = current_state+i;
		current_state = last_state->next_state;
		if (last_state->action) 
			(this->*(last_state->action))();
		input_data++;
		data_len--;
	}

	if (!(mode_flags & DEFERUPDATE) || (pending_scroll > scroll_bot-scroll_top))
		update_changes();
}

void GTerm::Reset()
{
	reset();
}

void GTerm::ExposeArea(int x, int y, int w, int h)
{
	int i;
	for (i=0; i<h; i++) changed_line(i+y, x, x+w-1);
	if (!(mode_flags & DEFERUPDATE)) update_changes();
}

void GTerm::ResizeTerminal(int w, int h)
{
	int cx, cy;
	clear_area(min(width,w), 0, GT_MAXWIDTH-1, GT_MAXHEIGHT-1);
	clear_area(0, min(height,h), min(width,w)-1, GT_MAXHEIGHT-1);
	width = w;
	height = h;
	scroll_bot = height-1;
	if (scroll_top >= height) scroll_top = 0;
	cx = min(width-1, cursor_x);
	cy = min(height-1, cursor_y);
	move_cursor(cx, cy);
}

GTerm::GTerm(int w, int h) : width(w), height(h)
{
	int i;

	doing_update = 0;

	// could make this dynamic
	text = new unsigned char[GT_MAXWIDTH*GT_MAXHEIGHT];
	memset(text, 0, sizeof(text));
	color = new unsigned short[GT_MAXWIDTH*GT_MAXHEIGHT];
	memset(color, 0, sizeof(color));

	for (i=0; i<GT_MAXHEIGHT; i++) {
		// make it draw whole terminal to start
		dirty_startx[i] = 0;
		dirty_endx[i] = GT_MAXWIDTH-1;
	}

	cursor_x = 0;
	cursor_y = 0;
	save_x = 0;
	save_y = 0;
	mode_flags = 0;
	reset();
}

GTerm::~GTerm()
{
	delete text;
	delete color;
}

/* End of File */
