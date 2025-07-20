//used to accurately track player inputs
//when compiled this version will be used instead of just is_down
// macro helpers
//-> is used when a pionter is pointing to a struct or a union
#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

enum Gamemod {
	GM_MENU,
	GM_GAMEPLAY
};

Gamemod current_gamemod;
int hovered_button;
bool enemy_is_ai;

//player's initial data
float player_1_speed, player_2_speed, player_1_acceleration, player_2_acceleration;
float player_1_pos_x = 80, player_1_pos_y = 0, player_2_pos_x = -80, player_2_pos_y = 0;
float player_half_size_x = 2, player_half_size_y = 10;
int player_1_score, player_2_score;
//arena's initial data
float arena_half_size_x = 85, arena_half_size_y = 45;
//ball's initial data
float ball_position_x, ball_position_y, ball_speed_x = 100, ball_speed_y;
float ball_half_size_x = 1, ball_half_size_y = 1;

internal void
//pointers for position and speed as we want to change those values
simulate_player(float *position, float *speed, float acceleration, float dt) {
	//frictions to deccelerate
	acceleration -= *speed * 10.f;
	//movement equations
	//thanks to dt speed is in units/seconds
	*position = *position + *speed * dt + acceleration * dt * dt * .5f;
	*speed = *speed + acceleration * dt;

	//1-D collision detection
	//if player position is > or < than arena bounds, it is set to the bound themselves
	//half_sizes!! to account for the perimeter and not the center only
	//of course this works because the shape of the objects are fairly simple !
	if (*position + player_half_size_y > arena_half_size_y) {
		*position = arena_half_size_y - player_half_size_y;
		*speed = 0;
	}
	else if (*position - player_half_size_y < -arena_half_size_y) {
		*position = -arena_half_size_y + player_half_size_y;
		*speed = 0;
	}

}

internal bool
aabb_vs_aabb(float b_pos_x, float b_pos_y, float b_half_size_x, float b_half_size_y,
	float p_pos_x, float p_pos_y, float p_half_size_x, float p_half_size_y) {
	return (
		b_pos_x + b_half_size_x > p_pos_x - p_half_size_x
		&& b_pos_x - b_half_size_x < p_pos_x + p_half_size_x
		&& b_pos_y + b_half_size_y > p_pos_y - p_half_size_y
		&& b_pos_y - b_half_size_y < p_pos_y + p_half_size_y
		);
}

internal void
simulate_game(Input* input, float dt) {
	//clears the screen each execution to update the window content depending on the player's inputs
	//playing area
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x191970);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xffffff);

	//GAME
	if (current_gamemod == GM_GAMEPLAY) {

		//initial player acceleration
		player_1_acceleration = 0, player_2_acceleration = 0;

		//tracks player's inputs;
		if (is_down(BUTTON_UP)) player_1_acceleration += 2000;
		if (is_down(BUTTON_DOWN)) player_1_acceleration -= 2000;
		//takes effect before compilation in the prepocessor
		if (!enemy_is_ai) {
		if (is_down(BUTTON_Z)) player_2_acceleration += 2000;
		if (is_down(BUTTON_S)) player_2_acceleration -= 2000;
	} else {
		//offsets the player position to prevent the AI from wiggling 
		if (ball_position_y > player_2_pos_y + 2.f && ball_position_x < 0) player_2_acceleration += 1500;
		if (ball_position_y < player_2_pos_y - 2.f && ball_position_x < 0) player_2_acceleration -= 1500;
	}

		//debug
		if (pressed(BUTTON_RIGHT)) ball_speed_x *= 1.5;
		if (pressed(BUTTON_LEFT)) ball_speed_x *= 0.5;
		if (pressed(BUTTON_SPACE)) {
			ball_position_x = 0.f, ball_position_y = 0, ball_speed_y = 0;
			if (ball_speed_x < 1) ball_speed_x = 100;
		};

		//players logic
		simulate_player(&player_1_pos_y, &player_1_speed, player_1_acceleration, dt);
		simulate_player(&player_2_pos_y, &player_2_speed, player_2_acceleration, dt);


		//Simulate Ball
		{
			//ball's movements equations
			ball_position_x += ball_speed_x * dt;
			ball_position_y += ball_speed_y * dt;

			//AABB vs AABB (axis aligned bounding box) collision
			//checks collision on all 4 sides of the player
			//though it seems unecessary in this case checking right side collision for player 1
			//prevents the game for restarting when moving to last ball position after the ball has passed the player
			if (aabb_vs_aabb(ball_position_x, ball_position_y, ball_half_size_x, ball_half_size_y, player_1_pos_x, player_1_pos_y, player_half_size_x, player_half_size_y)
				) {
				ball_position_x = player_1_pos_x - player_half_size_x - ball_half_size_x;
				ball_speed_x *= -1;
				//ball y speed = y speed of the player + how far from the center of the player the ball is hit
				ball_speed_y = (ball_position_y - player_1_pos_y) * 2 + player_1_speed * .75f;
			}
			else if (aabb_vs_aabb(ball_position_x, ball_position_y, ball_half_size_x, ball_half_size_y, player_2_pos_x, player_2_pos_y, player_half_size_x, player_half_size_y)) {
				ball_position_x = player_2_pos_x + player_half_size_x + ball_half_size_x;
				ball_speed_x *= -1;
				ball_speed_y = (ball_position_y - player_2_pos_y) * 2 + player_2_speed * .75f;
			}

			//ball collision with the arena
			//ball position set to the limit if greater of lower than arena half size
			if (ball_position_y + ball_half_size_y > arena_half_size_y) {
				ball_position_y = arena_half_size_y - ball_half_size_y;
				ball_speed_y *= -1;
			}
			else if (ball_position_y - ball_half_size_y < -arena_half_size_y) {
				ball_position_y = -arena_half_size_y + ball_half_size_y;
				ball_speed_y *= -1;
			}

			//ball position x reset after a collision with left or right side of the arena
			//player 1's side collision
			if (ball_position_x + ball_half_size_x > arena_half_size_x) {
				ball_position_x = 0, ball_position_y = 0;
				ball_speed_x *= -1, ball_speed_y = 0;
				player_2_score++;
			}
			//player 2's side collision
			else if (ball_position_x - ball_half_size_x < -arena_half_size_x) {
				ball_position_x = 0, ball_position_y = 0;
				ball_speed_x *= -1, ball_speed_y = 0;
				player_1_score++;
			}
		}

		draw_number(player_2_score, -10, 40, 1.f, 0xffffff);
		draw_number(player_1_score, 0, 40, 1.f, 0xffffff);


		//Rendering
		// player object
		draw_rect(player_1_pos_x, player_1_pos_y, player_half_size_x, player_half_size_y, 0xffffff);
		//oponent object
		draw_rect(player_2_pos_x, player_2_pos_y, player_half_size_x, player_half_size_y, 0xffffff);
		//ball object
		draw_rect(ball_position_x, ball_position_y, ball_half_size_x, ball_half_size_y, 0xffffff);

	}
	//MENU
	else {
		//Button selection
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hovered_button = ~hovered_button;
		}

		//Button action
		if (pressed(BUTTON_ENTER)) {
			current_gamemod = GM_GAMEPLAY;
			enemy_is_ai = hovered_button ? 0 : 1;
		}
		//Button color feedback
		if (!hovered_button) {
			draw_text("SINGLEPLAYER", -80, -10, 1, 0x00ffff);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xffffff);
		}
		else {
			draw_text("SINGLEPLAYER", -80, -10, 1, 0xffffff);
			draw_text("MULTIPLAYER", 20, -10, 1, 0x00ffff);
		}
	}
}