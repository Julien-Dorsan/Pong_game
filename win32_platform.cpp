//run with ctrl+F10 stops execution at a certain line and displays the variables
//then F9 is used to put breakpoints and stop execution
//then F5 resumes it
//hovering over variables during runtime give their values
//hovering over pointers during runtime gives their momory location

//winmain doc for entry point for graphical windows based app
//https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
#include <random>
#include <iostream>
#include <chrono>
#include "utils.cpp"
//windows stuff (window, inputs, perf queries)
#include <windows.h>
///sound stuff
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

//directive for debug related code, #debug are used a bit everywhere
#define debug false

//game loop var
global_variable bool running = true;
global_variable bool activate_sound = true;

struct Render_State {
	//dimensions of the window
	int height, width;

	//pointer to make it persistant over different functions, not knowing the type is not an issue
	//void makes variables globals (stored in the data section)
	void* memory;

	//bitmap infostruct holds info like what the pixels | compression look like
	BITMAPINFO bitmap_info;
};

global_variable Render_State render_state;

//compilation :
//usually all the cpp files are compiled into intermediate .obj files and link them to get the exe
//here we're going to use unity build :
//cpp files are imported into one maste file that becomes .obj that becomes .exe
//easier file interation + faster compilation
//IMPORTANT for files that are not master : file -> properties -> general -> excluded from build = YES
#include "platform_common.cpp"
#include "renderer.cpp"
#include "game.cpp"
//class for window callback events, called when window wants to pass message
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//creates a value that should be used as a return value for a window procedure
	LRESULT result = 0;
	//https://learn.microsoft.com/en-us/windows/win32/winmsg/about-messages-and-message-queues
	//test the different user's iteraction with the window
	switch (uMsg) {
		//when evaluated both return 0
		//sent when a window or application should end
	case WM_CLOSE:
		//a window is being destroyed
	case WM_DESTROY: {
		running = false;
	}break;

				   //sent when size of the window is changed
				   //https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-size
	case WM_SIZE: {
		//rect contains pointer to RECT struct containing the up, bottom, left, right values of a given window
		RECT rect;
		//linking the rect struct to a specific window
		GetClientRect(hwnd, &rect);
		//calculations to get exact width | height
		render_state.width = rect.right - rect.left;
		render_state.height = rect.bottom - rect.top;

		//resizable buffer size
		int buffer_size = render_state.width * render_state.height * sizeof(unsigned int);

		//ensures dynamic memory allocation based on needs
		//if the buffer exists the memory is freed then realocated
		if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
		render_state.memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		//https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
		//contains information about the dimensions and color format of a device - independent bitmap
		render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
		render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
		render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
		render_state.bitmap_info.bmiHeader.biPlanes = 1;
		render_state.bitmap_info.bmiHeader.biBitCount = 32;
		render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;
	}break;

	default: {
		//this return passes all the parameters recieved
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	}
	return result;
}
//on first try error will occur
//To fix : app properties -> linker -> system -> 
//configuration : all configuration | paltform : all platform | subsytem : Windows
//https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	//optional functions call
	ShowCursor(FALSE);
	//Create Window Class
	//class creation
	//https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassa
	WNDCLASS window_class = {};
	//window will redraw vertically and horizontally when needed
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	//string to identify window_class not visible for the user
	//this creates an error to fix : project -> general -> advanced -> character set -> "use Multi-Byte Character Set"
	window_class.lpszClassName = "Game Window Class";
	//window callback function to recieve user interaction with it
	window_class.lpfnWndProc = window_callback;

	//Register Class
	//here a pointer is used to get to the memory location of window_class and all of its information
	//also no copy of window_class and the class itself are not directly passed that way
	//also here window_class can be null
	//pointers are fun!!
	//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassa
	RegisterClass(&window_class);
	//Create Window
	//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
	HWND window = CreateWindow(
		window_class.lpszClassName, /*lpClassName*/
		"My First Pong Game", /*lpWindowName*/
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, /*dwStyle*/
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1280, /*x(width)*/
		720,	/*y(height)*/
		0, /*nWidth*/
		0, /*nHeight*/
		hInstance, /*hWndParent*/
		0 /*lpParam?*/);
#if !debug
	//Make the window fullscreen
	{
		//Overlapped flag set to false
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		//get monitor info
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		//set the window to be on top and height and width are screen dimension
		SetWindowPos(window,
			HWND_TOP,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
#endif

	//gets the window device context
	HDC hdc = GetDC(window);

	//creation/initialisation ? of the input struct
	Input input = {};

	//variables to track framerate / time between frame
	//time in one frame (assumed to be 60 for the first one)
	float delta_time = 0.016666f;
	//CPU time
	LARGE_INTEGER frame_begin_time;
	//pointer for CPU time variable
	QueryPerformanceCounter(&frame_begin_time);

	//returns how many cycle can be ran in 1 second
	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}

	//game loop
	while (running) {
		//Input
		MSG message;

		//reset of button change at the begining of each frame
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}

		//gets all the pending messages
		// then translates it to characters
		// then dispatches it to a window procedure
		//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagea
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			switch (message.message) {
				//If an iteraction with a button is detected this fires
			case WM_KEYUP:
			case WM_KEYDOWN: {
				//stands for virtual key
				//https://learn.microsoft.com/fr-fr/windows/win32/inputdev/virtual-key-codes
				u32 vk_code = (u32)message.wParam;
				bool is_down = ((message.lParam & (1 << 31)) == 0);
				//changed tracks only the initial input and not hold input
#define process_button(b, vk)\
	case vk: {\
	input.buttons[b].changed = is_down != input.buttons[b].is_down;\
	input.buttons[b].is_down = is_down;\
	}break;

						//will evaluate only the button related to the program
						//Single quotes to pass a char and not a const char
				switch (vk_code) {
					process_button(BUTTON_UP, VK_UP);
					process_button(BUTTON_DOWN, VK_DOWN);
					process_button(BUTTON_LEFT, VK_LEFT);
					process_button(BUTTON_RIGHT, VK_RIGHT);
					process_button(BUTTON_Z, 'Z');
					process_button(BUTTON_S, 'S');
					process_button(BUTTON_ENTER, VK_RETURN);
					process_button(BUTTON_ESCAPE, VK_ESCAPE);
#if debug
					process_button(BUTTON_SPACE, VK_SPACE);
					process_button(BUTTON_Q, 'Q');
					process_button(BUTTON_D, 'D');
#endif
				}

			}break;
						   //any message that is not related to the stated case above
			default: {
				//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage
				TranslateMessage(&message);
				//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage
				DispatchMessage(&message);
			}
			}
		}

		//Simulates the game inside the created window
		//takes the user's inputs and "framerate" as args
		simulate_game(&input, delta_time);

		//Render
		//at first this will render a black screen because the memory is 0. meaning?
		//arg1 : device context = the window used
		//https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		//CPU time
		LARGE_INTEGER frame_end_time;
		//pointer for CPU time variable
		QueryPerformanceCounter(&frame_end_time);
		//time elapsed between end frame and begin frame as an signed 64bit integer
		//divided by performance frequency to convert CPU time to seconds
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		//sets new begin time to track next frame
		frame_begin_time = frame_end_time;
	}
}
//rendering process :
//get a buffer in memory
//	first we need to know the size of the window to allocate the necessary memory (dynamic memory)
//	buffer contains width * height pixels
//	a pixel will be an unsigned 32 bits int 8red | 8green | 8blue | 8bits to pad to 32
//	to dynamically allocate this memory it will be on the heap
//	when the window is resized buffer is deleted then recreated (VirtualAlloc)
//	see callback WM_SIZE case for reference
//use it to process the pixels and the colors
//send it to the window to dipslay it