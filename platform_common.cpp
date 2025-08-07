//inputs states
struct Button_State {
	bool is_down;
	bool changed;
};

//available inputs
enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_Z,
	BUTTON_S,
	BUTTON_ENTER,
	BUTTON_ESCAPE,
#if debug
	BUTTON_SPACE,
	BUTTON_Q,
	BUTTON_D,
#endif
	
	//should be the last to then create an array
	BUTTON_COUNT,
};

struct Input {
	Button_State buttons[BUTTON_COUNT];
};