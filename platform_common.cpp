struct Button_State {
	bool is_down;
	bool changed;
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_SPACE,
	
	//should be the last to then create an array
	BUTTON_COUNT,
};

struct Input {
	Button_State buttons[BUTTON_COUNT];
};