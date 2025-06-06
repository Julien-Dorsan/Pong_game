//used to accurately track player inputs
//when compiled this version will be used instead of just is_down
// macro helpers
//-> is used when a pionter is pointing to a struct or a union
#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

//player's initial data
float player_pos_y = 0.f;
float player_pos_x = 80.f;
float speed = 50.f;
int half_size_x = 2;
int half_size_y = 10.f;


int size = 20;
int terrain[2][2] = { {50, 10}, {-50, 10} };
int terrain_width = 4.f;
int terrain_height = 200;


internal void
simulate_game(Input* input, float dt) {
	//clears the screen each execution to update the window content depending on the player's inputs
	clear_screen(0x000000);
	//tracks player's inputs;
	//thanks to dt speed is in units/seconds
	if (is_down(BUTTON_DOWN)) player_pos_y -= speed * dt;
	if (is_down(BUTTON_UP)) player_pos_y += speed * dt;
	if (pressed(BUTTON_SPACE)) {
		player_pos_x = 80.f;
		player_pos_y = 0.f;
	};
	
	// player object
	draw_rect(player_pos_x, player_pos_y, half_size_x, half_size_y, 0xffffff);
}