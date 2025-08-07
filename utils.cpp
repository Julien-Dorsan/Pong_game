//(u) : unsigned var 0 <= val <=max;
//(s) : signed var min <= val <= max
//also size in bits is indicated, usefull!!
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;

//space for vars with these ids will allocated through the lifetime of the program
#define global_variable static
#define internal static

//inline suggests that the compiler substitutes the code within the func def in place of each call of that func
/// <summary>
/// clamps given value betweeen a min and a max
/// </summary>
/// <param name="min"></param>
/// <param name="val"></param>
/// <param name="max"></param>
/// <returns></returns>
inline int
clamp(int min, int val, int max) {
	//prevents overflowing
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

/// <summary>
/// time-based seed random number generator using the Mersenne Twister engine
/// </summary>
/// <param name="min">included in the range</param>
/// <param name="max">included in the range</param>
/// <returns></returns>
int get_random_number(int min, int max) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(generator);
}

//structs for recurring data
struct Color {
	u32 main = 0x191970;
	u32 sub = 0x000000;
	u32 inactive = 0x808080;
	u32 active = 0x00ffff;
	u32 disabled = 0xD3D3D3;
	u32 white = 0xffffff;
	u32 black = 0x0000000;
	u32 win = 0xBFF4BE;
	u32 lose = 0xff6242;
	u32 gold = 0xFFD700;
};

Color color;

struct Sound {
	const char* beep = ".\\sound\\beep.wav";
	const char* portal = ".\\sound\\portal.wav";
	const char* wall = ".\\sound\\wall.wav";
	const char* ascended = ".\\sound\\ascended.wav";
	const char* select = ".\\sound\\select.wav";
	const char* enter = ".\\sound\\enter.wav";
	const char* game_over = ".\\sound\\game_over.wav";
	const char* win = ".\\sound\\win.wav";
};

Sound sound;

//2d array of stings for letters with each 0 representing a filled space
//! string = pointer to a character (char*)
const char* letters[][7] = {
	" 00",
	"0  0",
	"0  0",
	"0000",
	"0  0",
	"0  0",
	"0  0",

	"000",
	"0  0",
	"0  0",
	"000",
	"0  0",
	"0  0",
	"000",

	" 000",
	"0",
	"0",
	"0",
	"0",
	"0",
	" 000",

	"000",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"000",

	"0000",
	"0",
	"0",
	"000",
	"0",
	"0",
	"0000",

	"0000",
	"0",
	"0",
	"000",
	"0",
	"0",
	"0",

	" 000",
	"0",
	"0",
	"0 00",
	"0  0",
	"0  0",
	" 000",

	"0  0",
	"0  0",
	"0  0",
	"0000",
	"0  0",
	"0  0",
	"0  0",

	"000",
	" 0",
	" 0",
	" 0",
	" 0",
	" 0",
	"000",

	" 000",
	"   0",
	"   0",
	"   0",
	"0  0",
	"0  0",
	" 000",

	"0  0",
	"0  0",
	"0 0",
	"00",
	"0 0",
	"0  0",
	"0  0",

	"0",
	"0",
	"0",
	"0",
	"0",
	"0",
	"0000",

	"00 00",
	"0 0 0",
	"0 0 0",
	"0   0",
	"0   0",
	"0   0",
	"0   0",

	"00  0",
	"0 0 0",
	"0 0 0",
	"0 0 0",
	"0 0 0",
	"0 0 0",
	"0  00",

	"0000",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"0000",

	" 000",
	"0  0",
	"0  0",
	"000",
	"0",
	"0",
	"0",

	" 000 ",
	"0   0",
	"0   0",
	"0   0",
	"0 0 0",
	"0  0 ",
	" 00 0",

	"000",
	"0  0",
	"0  0",
	"000",
	"0  0",
	"0  0",
	"0  0",

	" 000",
	"0",
	"0 ",
	" 00",
	"   0",
	"   0",
	"000 ",

	"000",
	" 0",
	" 0",
	" 0",
	" 0",
	" 0",
	" 0",

	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	"0  0",
	" 00",

	"0   0",
	"0   0",
	"0   0",
	"0   0",
	"0   0",
	" 0 0",
	"  0",

	"0   0 ",
	"0   0",
	"0   0",
	"0 0 0",
	"0 0 0",
	"0 0 0",
	" 0 0 ",

	"0   0",
	"0   0",
	" 0 0",
	"  0",
	" 0 0",
	"0   0",
	"0   0",

	"0   0",
	"0   0",
	" 0 0",
	"  0",
	"  0",
	"  0",
	"  0",

	"0000",
	"   0",
	"  0",
	" 0",
	"0",
	"0",
	"0000",

	"    ",
	" 00 ",
	" 00 ",
	"    ",
	" 00 ",
	" 00 ",
	"    ",

	"   ",
	" 0 ",
	" 0 ",
	"000",
	" 0 ",
	" 0 ",
	"   ",

	"    ",
	"    ",
	"    ",
	"0000",
	"    ",
	"    ",
	"    ",

	" 00 ",
	" 00 ",
	" 00 ",
	" 00 ",
	"    ",
	" 00 ",
	" 00 ",

};