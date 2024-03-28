internal void
//deprecated
render_background() {
	//current pixel pointer indicating pixel that we want to fill starting with the first one in the buffer
	u32* pixel = (u32*)render_state.memory;
	//iteration through all the height and width pixels
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			//here pixel is incremented to point to the next one
			//if not screen is just black
			//here playing with the color formulas make for interesting results
			*pixel++ = 0xffffff;
		}
	}
}

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
		u32* pixel = (u32*)render_state.memory + x0 + y*render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = color;
		}
	}
}

//here f is used to specify that a float should be obtained
global_variable float render_scale = 0.01f;

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

