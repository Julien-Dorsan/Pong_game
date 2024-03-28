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
#define global_variable static;
#define internal static;

//inline suggests that the compiler substitutes the code within the func def in place of each call of that func
inline int
clamp(int min, int val, int max) {
	//prevents overflowing
	if (val < min) return min;
	if (val > max) return max;
	return val;
}