/// <summary>
/// deprecated
/// </summary>
/// <param name="color"></param>
/// <returns></returns>
internal void
clear_screen(u32 color) {
	//current pixel pointer indicating pixel that we want to fill starting with the first one in the buffer
	u32* pixel = (u32*)render_state.memory;
	//iteration through all the height and width pixels
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			//here pixel is incremented to point to the next one
			//if not screen is just black
			//here playing with the color formulas make for interesting results
			*pixel++ = color;
		}
	}
}
/// <summary>
/// drawx a rectangle on the given pixels
/// </summary>
/// <param name="x0"></param>
/// <param name="y0"></param>
/// <param name="x1"></param>
/// <param name="y1"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
	//to avoid trying to access pixel coordinates that are outside of the window
	//the coordinates will be clamped to either 0 or the max height | width
	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);
	//instead of iterating starting from the first pixel of the screen
	//we start with x0, y0
	for (int y = y0; y < y1; y++) {
		//for every row current pixel is set to memory
		//then first pixel is offest by the first x pixel and the current y pixel * the width
		u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = color;
		}
	}
}

//here f is used to specify that a float should be obtained
global_variable float render_scale = 0.01f;
/// <summary>
/// Given that the arena is rendered on top,
/// randereing only the borders save ressources
/// </summary>
/// <param name="arena_x"></param>
/// <param name="arena_y"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_arena_borders(float arena_x, float arena_y, u32 color) {
	//transforms x and y values to pixel coordinates
	arena_x *= render_state.height * render_scale;
	arena_y *= render_state.height * render_scale;

	int x0 = (int)((float)render_state.width * .5f - arena_x);
	int x1 = (int)((float)render_state.width * .5f + arena_x);
	int y0 = (int)((float)render_state.height * .5f - arena_y);
	int y1 = (int)((float)render_state.height * .5f + arena_y);

	//4 rectangles covering empty screen space (between the arena and the window borders)
	draw_rect_in_pixels(0, 0, render_state.width, y0, color);
	draw_rect_in_pixels(0, y1, x1, render_state.height, color);
	draw_rect_in_pixels(0, y0, x0, y1, color);
	draw_rect_in_pixels(x1, y0, render_state.width, render_state.height, color);
}

/// <summary>
/// wrapper for draw_rect_in_pixel,
/// draws rectangle given that the origin is in the middle of the screen
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="half_size_x"></param>
/// <param name="half_size_y"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color) {
	//this makes the rect scale withe the window height
	x *= render_state.height * render_scale;
	y *= render_state.height * render_scale;
	half_size_x *= render_state.height * render_scale;
	half_size_y *= render_state.height * render_scale;

	//this offsets the coordinates to the center of the screen
	//.f is used to obtain a floatin point number and not an int after the division
	x += render_state.width / 2.f;
	y += render_state.height / 2.f;

	//change to pixel
	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;

	draw_rect_in_pixels(x0, y0, x1, y1, color);
}

/// <summary>
/// renders text using the rectangle privimites
/// uses ASCII encoding
/// </summary>
/// <param name="text">: crashes the program if any character is in lowercase</param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="size">: characters and spaces are different sizes which should be accounted throught the entire code</param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_text(const char* text, float x, float y, float size, u32 color) {
	float half_size = size * .5f;
	float original_y = y;
	//after recieving a string iterates through each character
	while (*text) {
		//in ANSI decimal 32 is a space
		//so if the character isn't a space it is rendered
		if (*text != 32) {
			const char** letter;
			//ANSI encoding stuff
			//hardcoded :
			if (*text == 58) letter = letters[26];
			//hardcoded +
			else if (*text == 43) letter = letters[27];
			//hardcoded -
			else if (*text == 45) letter = letters[28];
			//hardcoded !
			else if (*text == 33) letter = letters[29];
			//letter 'A' is 65 in decimal
			//so for A to map to 0, B to 1, c to 2 and so forth
			//'A' or 65 is subtracted to each letter
			else letter = letters[*text - 'A'];
			float original_x = x;
			//increments on each row of the letter array
			for (int i = 0; i < 7; i++) {
				const char* row = letter[i];
				//iterates over a single row
				while (*row) {
					//if a 0 is found a rectangle is drawn
					if (*row == '0') {
						draw_rect(x, y, half_size, half_size, color);
					}
					//x position is moved by an entire rectangle to the right (a size)
					x += size;
					//points to the next character in the row
					row++;
				}
				//when a row is fully drawn continues to the next one
				//y position is moved by an entire rectangle down(a size)
				y -= size;
				//resets the x position to keep the same structure (letter aligned on the same base x axis)
				x = original_x;
			}
		}
		//moves to the next letter in memory
		text++;
		//space between each letter
		//resets the y position to keep the same structure (letter aligned on the same base y axis)
		*text != 32 ? x += size * 6.f : x += size * 2.f;
		y = original_y;
	}
}

/// <summary>
/// Draws numbers using the rectangle primitives
/// </summary>
/// <param name="number"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="size"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_number(int number, float x, float y, float size, u32 color) {
	float half_size = size * .5f;

	bool drew_number = false;
	while (number || !drew_number) {
		drew_number = true;

		int digit = number % 10;
		number = number / 10;

		switch (digit) {
		case 0: {
			draw_rect(x - size, y, half_size, 2.5f * size, color);
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			draw_rect(x, y + size * 2.f, half_size, half_size, color);
			draw_rect(x, y - size * 2.f, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 1: {
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			x -= size * 2.f;
		} break;

		case 2: {
			draw_rect(x, y + size * 2.f, 1.5f * size, half_size, color);
			draw_rect(x, y, 1.5f * size, half_size, color);
			draw_rect(x, y - size * 2.f, 1.5f * size, half_size, color);
			draw_rect(x + size, y + size, half_size, half_size, color);
			draw_rect(x - size, y - size, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 3: {
			draw_rect(x - half_size, y + size * 2.f, size, half_size, color);
			draw_rect(x - half_size, y, size, half_size, color);
			draw_rect(x - half_size, y - size * 2.f, size, half_size, color);
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			x -= size * 4.f;
		} break;

		case 4: {
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			draw_rect(x - size, y + size, half_size, 1.5f * size, color);
			draw_rect(x, y, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 5: {
			draw_rect(x, y + size * 2.f, 1.5f * size, half_size, color);
			draw_rect(x, y, 1.5f * size, half_size, color);
			draw_rect(x, y - size * 2.f, 1.5f * size, half_size, color);
			draw_rect(x - size, y + size, half_size, half_size, color);
			draw_rect(x + size, y - size, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 6: {
			draw_rect(x + half_size, y + size * 2.f, size, half_size, color);
			draw_rect(x + half_size, y, size, half_size, color);
			draw_rect(x + half_size, y - size * 2.f, size, half_size, color);
			draw_rect(x - size, y, half_size, 2.5f * size, color);
			draw_rect(x + size, y - size, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 7: {
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			draw_rect(x - half_size, y + size * 2.f, size, half_size, color);
			x -= size * 4.f;
		} break;

		case 8: {
			draw_rect(x - size, y, half_size, 2.5f * size, color);
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			draw_rect(x, y + size * 2.f, half_size, half_size, color);
			draw_rect(x, y - size * 2.f, half_size, half_size, color);
			draw_rect(x, y, half_size, half_size, color);
			x -= size * 4.f;
		} break;

		case 9: {
			draw_rect(x - half_size, y + size * 2.f, size, half_size, color);
			draw_rect(x - half_size, y, size, half_size, color);
			draw_rect(x - half_size, y - size * 2.f, size, half_size, color);
			draw_rect(x + size, y, half_size, 2.5f * size, color);
			draw_rect(x - size, y + size, half_size, half_size, color);
			x -= size * 4.f;
		} break;
		}

	}
}
/// <summary>
/// wrapper for the draw text function
/// accounts for the string size to center the text
/// </summary>
/// <param name="text">: crashes the program if any character is in lowercase</param>
/// <param name="y"></param>
/// <param name="size"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_text_centered(const char* text, float y, float size, u32 color) {
	float text_width = 0;
	//different pointer is used to let the original one intact
	const char* text_string = text;
	while (*text_string) {
		*text_string != 32 ? text_width += size * 6.f : text_width += size * 2.f;
		text_string++;
	}
	//would need to pass the width of the window origin wasn't in the middle
	draw_text(text, -text_width * 0.5f, y, size, color);
}

/// <summary>
/// yet another wrapper, this time for draw_text_centered
/// main goal is to make the selectable text more consitant
/// with colors and behaviours
/// </summary>
/// <param name="y"></param>
/// <param name="text_size"></param>
/// <param name="text">: crashes the program if any character is in lowercase</param>
/// <param name="active">: whether the user hovers the button or not</param>
/// <param name="inactive_color">: optional</param>
/// <param name="active_color">: optional</param>
/// <returns></returns>
internal void
draw_button(float y, float text_size, const char* text, bool active, u32 inactive_color = color.inactive, u32 active_color = color.active) {
	active ? draw_text_centered(text, y, text_size, active_color) : draw_text_centered(text, y, text_size, inactive_color);
}

/// <summary>
/// draws a circle to the coordinates in pixels
/// </summary>
/// <param name="x0"></param>
/// <param name="y0"></param>
/// <param name="x1"></param>
/// <param name="y1"></param>
/// <param name="cx"></param>
/// <param name="cy"></param>
/// <param name="radius"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_circle_in_pixels(int x0, int y0, int x1, int y1, int cx, int cy, int radius, u32 color) {
	//clamps coordinates to stay within the window
	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);

	//Iterates through the pixels within a rectangle around the circle
	for (int y = y0; y < y1; y++) {
		u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
		for (int x = x0; x < x1; x++) {
			//distance squared from the center to the current pixel
			int dx = x - cx;
			int dy = y - cy;
			//circle equation to check if the pixel belongs to the circle
			if (dx * dx + dy * dy <= radius * radius) {
				//then draws the correct pixels
				*pixel = color;
			}
			//moves to the next pixel in the row
			pixel++;
		}
	}
}

/// <summary>
/// wrapper for draw_circle_in_pixels
/// to draw a circle to given coordinates
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="radius"></param>
/// <param name="color"></param>
/// <returns></returns>
internal void
draw_circle(float x, float y, float radius, u32 color) {
	//circle scaling
	x *= render_state.height * render_scale;
	y *= render_state.height * render_scale;
	radius *= render_state.height * render_scale;
	//offset to the center of the screen
	x += render_state.width / 2.f;
	y += render_state.height / 2.f;
	//circle's bounding box
	int x0 = (int)(x - radius);
	int y0 = (int)(y - radius);
	int x1 = (int)(x + radius);
	int y1 = (int)(y + radius);
	//draws the circle in the given bounding box
	draw_circle_in_pixels(x0, y0, x1, y1, (int)x, (int)y, (int)radius, color);
}

/// <summary>
/// renders the key components of the game : player, ball and score.
/// makes the rendereing of the game more consistant through multiples function calls
/// </summary>
/// <param name="p1_py"></param>
/// <param name="p1_s"></param>
/// <param name="p1_color"></param>
/// <param name="p2_py"></param>
/// <param name="p2_s"></param>
/// <param name="p2_color"></param>
/// <param name="p_px"></param>
/// <param name="p_hsx"></param>
/// <param name="p_hsy"></param>
/// <param name="b_px"></param>
/// <param name="b_py"></param>
/// <param name="b_r"></param>
/// <param name="b_color"></param>
/// <param name="s_px"></param>
/// <param name="s_py"></param>
/// <param name="s_size"></param>
/// <param name="p1_s_color"></param>
/// <param name="p2_s_color"></param>
/// <returns></returns>
internal void
render_game(
	float p1_py,
	float p1_s,
	u32 p1_color,
	float p2_py,
	float p2_s,
	u32 p2_color,
	float p_px,
	float p_hsx,
	float p_hsy,
	float b_px,
	float b_py,
	float b_r,
	u32 b_color,
	float s_px,
	float s_py,
	float s_size,
	u32 p1_s_color,
	u32 p2_s_color
) {
	//rendering process
	//scores
	draw_number(p2_s, -s_px, s_py, s_size, p2_s_color);
	draw_number(p1_s, s_px, s_py, s_size, p1_s_color);
	// player 1 object
	draw_rect(p_px, p1_py, p_hsx, p_hsy, p1_color);
	//player 2 object
	draw_rect(-p_px, p2_py, p_hsx, p_hsy, p2_color);
	//ball object
	draw_circle(b_px, b_py, b_r, b_color);
}