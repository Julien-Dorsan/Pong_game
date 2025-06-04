//when compiled this version will be used instead of just is_down
#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_pos_y = 0.f;
float player_pos_x = 0.f;
int half_size_x = 2;
int half_size_y = 2;
int size = 20;

int terrain[2][2] = { {50, 10}, {-50, 10} };

int terrain_width = 4.f;
int terrain_height = 200;

internal void
simulate_game(Input* input) {
	clear_screen(0x000000);
	//-> is used when a pionter is pointing to a struct or a union
	if (is_down(BUTTON_DOWN)) player_pos_x -= .5f;
	if (is_down(BUTTON_LEFT)) player_pos_y -= .5f;
	if (is_down(BUTTON_RIGHT)) player_pos_y += .5f;
	if (is_down(BUTTON_UP)) player_pos_x += .5f;
	if (pressed(BUTTON_SPACE)) {
		player_pos_x = 0.f;
		player_pos_y = 0.f;
	};
}