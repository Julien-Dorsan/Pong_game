//used to accurately track player inputs
//when compiled this version will be used instead of just is_down
// macro helpers
//-> is used when a pionter is pointing to a struct or a union
#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)
#define cap_max(a, b) (a < b && a++)
#define cap_min(a, b) (a > b && a--)
#define reset(a) (a = 0)
#define play_sound(a) (activate_sound && PlaySound(TEXT(a), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT))
#define play_sound_once(a) (activate_sound && PlaySound(TEXT(a), NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT))

//states of the program
//used for conditional rendereing
enum Gamestate {
	GS_MENU,
	GS_GAMEPLAY,
	GS_QUITMENU,
	GS_DIFFICULTYMENU,
	GS_RESETTRANSITION,
	GS_VICTORYSCREEN
};
//modifiers adding variations to the gameplay
enum Modifier {
	MOD_NORMAL,
	MOD_PORTAL,
	MOD_PHANTOM,
	MOD_VOID,
	MOD_BLACKOUT,

	//should be the last to then create an array
	MOD_COUNT
};

//strings mainly used for menu rendering stuff
const char* difficulties_strings[5] = { "NEWBIE", "EASY", "CASUAL", "HARD", "INSANE" };
const char* modifiers_strings[MOD_COUNT] = { "NORMAL", "PORTAL","PHANTOM", "VOID", "BLACKOUT" };

//Both are equal to the first element of the enum when initiated like that
//previous_gamestate is mainly used for menu related stuff (ex continue)
Gamestate current_gamestate;
Gamestate previous_gamestate;

//keep tracks of menu button selection
int active_button;
bool enemy_is_ai;
//ai difficulty setting
int difficulty = 1;
//current modifier
int modifier;
//distrance at which the ai will detect the ball and start actively moving
float detection_radius;
//tracks number of hits during a play
int hits;
//used in execute logic
int current_hit;
//initilized at 1 to prevent division by 0
int speed_multiplier = 1;
//modifier that unleashes a "special move"
bool execute = false;
//ensures that the execute is possible 1 time per hit and not 1 time per frame
bool execute_check;

//multiple var that aren't initialized at either 1 or 0
//or that are suceptible to change for gameplay tuning
struct Initial_Data {
	float players_pos_x = 80.f;
	float score_pos_x = 10.f;
	float score_pos_y = 40.f;
	float score_size = 1.f;
	float players_half_size_x = 3.f;
	float players_half_size_y = 10.f;
	float ball_speed_x = 100.f;
	float ball_half_size = 1.3f;
	float arena_half_size_x = 85.f;
	float arena_half_size_y = 45.f;
	float portal_half_size_x = 1.f;
	float portal_half_size_y = 5.f;
	float timer = 3.f;
	float victory_timer = 5.f;
	float portal_timer = 10.f;
	float phantom_timer = 5.f;
	int max_score = 5;
	float players_accelertion = 2000.f;
	u32 players_color = color.white;
};
Initial_Data initial_data;

//player's initial data
float player_1_speed, player_2_speed, player_1_acceleration, player_2_acceleration;
float player_1_pos_y, player_2_pos_y;
int player_1_score, player_2_score;
//ball's initial data
float ball_pos_x, ball_pos_y, ball_speed_y;
float ball_speed_x = initial_data.ball_speed_x;
//timer related stuff
float timer = initial_data.timer;
float blackout_timer;
float phantom_timer;
float portal_timer = initial_data.portal_timer;
float victory_timer = initial_data.victory_timer;
//void modifier
float void_expension_formula;
//portal modifier
//initialized at 0 to generate coords on first frame
float portal_1_pos_x, portal_1_pos_y;
float portal_2_pos_x, portal_2_pos_y;
int max_score = initial_data.max_score;
//ascension gamemod
bool ascension = false;

/// <summary>
/// handles gamestates changes flow
/// </summary>
/// <param name="curr_gamestate"></param>
/// <returns></returns>
internal void
set_next_gamestate(Gamestate curr_gamestate) {
	previous_gamestate = current_gamestate;
	current_gamestate = curr_gamestate;
	active_button = 0;
}

/// <summary>
/// handles gamestates changes flow
/// </summary>
/// <param name="prev_gamestate"></param>
/// <returns></returns>
internal void
set_previous_gamestate(Gamestate prev_gamestate) {
	current_gamestate = previous_gamestate;
	previous_gamestate = prev_gamestate;
	active_button = 0;
}

/// <summary>
/// simulates realistic physics for both players movements
/// based on the time between frames
/// </summary>
/// <param name="position"></param>
/// <param name="speed"></param>
/// <param name="acceleration"></param>
/// <param name="dt"></param>
/// <returns></returns>
internal void
//pointers for position and speed as we want to change those values
simulate_player(float* position, float* speed, float acceleration, float dt) {
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
	if (*position + initial_data.players_half_size_y > initial_data.arena_half_size_y) {
		*position = initial_data.arena_half_size_y - initial_data.players_half_size_y;
		reset(*speed);
	}
	else if (*position - initial_data.players_half_size_y < -initial_data.arena_half_size_y) {
		*position = -initial_data.arena_half_size_y + initial_data.players_half_size_y;
		reset(*speed);
	}

}

/// <summary>
/// collision between object handling
/// </summary>
/// <param name="b_pos_x"></param>
/// <param name="b_pos_y"></param>
/// <param name="b_half_size_x"></param>
/// <param name="b_half_size_y"></param>
/// <param name="p_pos_x"></param>
/// <param name="p_pos_y"></param>
/// <param name="p_half_size_x"></param>
/// <param name="p_half_size_y"></param>
/// <returns>true if collision otherwise false</returns>
internal bool
aabb_vs_aabb(
	float b_pos_x,
	float b_pos_y,
	float b_half_size,
	float p_pos_x,
	float p_pos_y,
	float p_half_size_x,
	float p_half_size_y
) {
	return (
		b_pos_x + b_half_size > p_pos_x - p_half_size_x
		&& b_pos_x - b_half_size < p_pos_x + p_half_size_x
		&& b_pos_y + b_half_size > p_pos_y - p_half_size_y
		&& b_pos_y - b_half_size < p_pos_y + p_half_size_y
		);
}

/// <summary>
/// a less complex subsistitue for aabb_vs_aabb
/// used in set_initial_data to determine if an object is in a given position
/// </summary>
/// <param name="pos_x"></param>
/// <param name="pos_y"></param>
/// <param name="check_pos_x"></param>
/// <param name="check_pos_y"></param>
/// <param name="offset_x"></param>
/// <param name="offset_y"></param>
/// <returns></returns>
internal bool
check_position(float pos_x, float pos_y, float check_pos_x, float check_pos_y, float offset_x, float offset_y) {
	return (
		pos_x + offset_x > check_pos_x
		&& pos_x - offset_x < check_pos_x
		&& pos_y + offset_y > check_pos_y
		&& pos_y - offset_y < check_pos_y
		);
}

/// <summary>
/// for MOD_PORTAL
/// generates random coordinates for 2 portals
/// each between origin and the players
/// </summary>
/// <param name="padding_x"></param>
/// <param name="padding_y"></param>
/// <returns></returns>
internal void
generate_portal_coordinates(float padding_x = 30, float padding_y = 10) {
	portal_1_pos_x = get_random_number(-initial_data.arena_half_size_x + initial_data.portal_half_size_x + padding_x, 0);
	portal_1_pos_y = get_random_number(-initial_data.arena_half_size_y + initial_data.portal_half_size_y, initial_data.arena_half_size_y - initial_data.portal_half_size_y);
	portal_2_pos_x = get_random_number(0, initial_data.arena_half_size_x - initial_data.portal_half_size_x - padding_x);
	portal_2_pos_y = get_random_number(-initial_data.arena_half_size_y + initial_data.portal_half_size_y, initial_data.arena_half_size_y - initial_data.portal_half_size_y);
}

/// <summary>
/// sets the data necessary for the game to always start in the same state
/// transitions are added for a smooth experience
/// for better results, players_acceleration and ball_speed need to be reset
/// </summary>
/// <param name="dt"></param>
/// <param name="reset_score"></param>
/// <param name="transition"></param>
/// <param name="next_gamestate"></param>
/// <param name="padding"></param>
/// <returns></returns>
internal void
set_initial_data(float dt, bool reset_score, bool transition, Gamestate next_gamestate, float padding = .5f) {
	if (transition) {
		//distances from current ball posx, posy to origin
		float dx = -ball_pos_x;
		float dy = -ball_pos_y;
		//Pythagore to find a nice straight line to the origin
		float distance = std::sqrt(dx * dx + dy * dy);
		// normalization for speed multipicators
		float dir_x = dx / distance;
		float dir_y = dy / distance;
		//padding is added to account for floats being not exactly 0 but quite close to it
		//moves the player to y=0 slowly depending on current y position
		if (player_1_pos_y > padding) {
			player_1_acceleration -= .1;
			simulate_player(&player_1_pos_y, &player_1_speed, player_1_acceleration * player_1_pos_y, dt);
		}
		else if (player_1_pos_y < -padding) {
			player_1_acceleration += .1;
			simulate_player(&player_1_pos_y, &player_1_speed, player_1_acceleration * -player_1_pos_y, dt);
		}
		if (player_2_pos_y > padding) {
			player_2_acceleration -= .1;
			simulate_player(&player_2_pos_y, &player_2_speed, player_2_acceleration * player_2_pos_y, dt);
		}
		else if (player_2_pos_y < -padding) {
			player_2_acceleration += .1;
			simulate_player(&player_2_pos_y, &player_2_speed, player_2_acceleration * -player_2_pos_y, dt);
		}
		//checks if the ball is within the padding (here distance can replace the coords)
		if (!(std::abs(dx) <= padding && std::abs(dy) <= padding)) {
			ball_speed_x++;
			ball_speed_y++;
			//another Pythagore application to get the total speed
			float speed = clamp(1, (std::sqrt(ball_speed_x * ball_speed_x + ball_speed_y * ball_speed_y)), 75);
			//update position based on speed and direction
			ball_pos_x += dir_x * speed * dt;
			ball_pos_y += dir_y * speed * dt;
		}
		//score is only reset after interacting in a menu
		if (player_2_score > 0 && previous_gamestate != GS_GAMEPLAY) player_2_score--;
		if (player_1_score > 0 && previous_gamestate != GS_GAMEPLAY) player_1_score--;
	}
	//bool or (bool and bool and bool and bool and (bool or bool) and bool)
	if ((!transition
		|| (check_position(initial_data.players_pos_x, player_1_pos_y, initial_data.players_pos_x, 0, padding, padding)
			&& check_position(-initial_data.players_pos_x, player_2_pos_y, -initial_data.players_pos_x, 0, padding, padding)
			&& check_position(ball_pos_x, ball_pos_y, 0, 0, padding, padding)
			&& ((player_1_score == 0 && player_2_score == 0) || previous_gamestate == GS_GAMEPLAY)
			&& timer <= 0))
		) {
		//hard reset to 0 to account for the margin of uncertainty created by the padding and the floats
		//and reset timers to orininal values
		reset(player_1_pos_y);
		reset(player_2_pos_y);
		reset(player_1_speed);
		reset(player_2_speed);
		reset(ball_speed_x);
		reset(ball_speed_y);
		reset(ball_pos_x);
		reset(ball_pos_y);
		reset(hits);
		reset(detection_radius);
		if (modifier == MOD_PORTAL) {
			portal_timer = initial_data.portal_timer;
			generate_portal_coordinates();
		}
		timer = initial_data.timer;
		victory_timer = initial_data.victory_timer;
		if (reset_score) { reset(player_1_score); reset(player_2_score); }
		//sends the ball randomly to a player
		ball_speed_x = get_random_number(0, 1) ? -initial_data.ball_speed_x : initial_data.ball_speed_x;
		set_next_gamestate(next_gamestate);
	}
	//decrements a timer to add a cooldown
	timer -= dt;
}

/// <summary>
/// distande at which the AI starts to actively move
/// is adjusted depending on the difficulty and the gamemod
/// </summary>
/// <returns></returns>
internal void
set_detection_radius() {
	//execute first to avoid setting the radius too far from player_2
	if (execute) {
		detection_radius = -initial_data.players_pos_x + initial_data.players_half_size_x + initial_data.ball_half_size + 1.f;
	}
	else if (modifier == MOD_VOID) {
		if (difficulty == 1) {
			detection_radius = (-initial_data.arena_half_size_x * 2) * 0.25;
		}
		else {
			//detection is at least equal to the wall origninal pos so works just fine
			detection_radius = -void_expension_formula;
		}
	}
	//acts as a "blind" mode, ai will only passively move
	else if ((modifier == MOD_VOID && blackout_timer <= 0.2)
		|| (modifier == MOD_PHANTOM && phantom_timer <= 0.5)) {
		detection_radius = -initial_data.arena_half_size_x;
	}
	//normal gamemod
	//will either be 1/4 1/2 3/4 or entire arena size
	else {
		switch (difficulty)
		{
		case 1:
			detection_radius = (-initial_data.arena_half_size_x * 2) * 0.25;
			break;
			//:/
		case 2:
			detection_radius = 0;
			break;
		case 3:
			detection_radius = (initial_data.arena_half_size_x / 2);
			break;
		default:
			detection_radius = initial_data.arena_half_size_x;
			break;
		}
	}
}

/// <summary>
/// AI simulating the movements of the second player
/// in singleplayer and ascension modes
/// </summary>
/// <param name="padding"></param>
/// <returns></returns>
internal void
simulate_ai(float padding = 2.f) {
	//sets the detection radius on first execution
	if (!detection_radius) {
		set_detection_radius();
	}
	//checks whether ball is above or below pos_y abd offsets the player position to prevent the AI from wiggling
	if (ball_pos_y > player_2_pos_y + padding)
	{
		speed_multiplier = 1;
	}
	else if (ball_pos_y < player_2_pos_y - padding)
	{
		speed_multiplier = -1;
	}
	//spices up the game when player_1 and AI just continuously hit the ball at ball_pos_y=0
	//only checks once per AI hit (execute_check)
	else if (ball_pos_y < std::abs(player_2_pos_y + padding) && ball_speed_y == 0 && ball_speed_x < 0 && difficulty > 3 && !execute_check) {
		int random_number = get_random_number(1, 10);
		if (random_number != 0) {
			speed_multiplier = 10 * random_number;
			execute = true;
			current_hit = hits;
			get_random_number(0, 1) == 0 ? speed_multiplier *= -1 : speed_multiplier;
			set_detection_radius();
		}
		execute_check = true;
	}
	//ball_speed_x < 0 ensures that the AI will plan movements when the ball moves toward it
	if (ball_speed_x < 0 && ball_pos_x <= detection_radius) {
		if (!execute) {
			//acceleration will be between +-1500 and +-2500
			player_2_acceleration += (1500 + 250 * (difficulty - 1)) * speed_multiplier;
		}
		else {
			player_2_acceleration += 1000 * speed_multiplier;
		}
	}
	//idle movment
	//pos and speed check might be verbose but it prevents the execute from breaking
	else if (ball_speed_y != 0 && ball_pos_y != 0) {
		if (speed_multiplier < 0) {
			player_2_acceleration += (250 * difficulty) * -1;
		}
		else
		{
			player_2_acceleration += (250 * difficulty) * 1;
		}
	}
	//resets execute state and detection radius to make AI move normally again
	if (execute && current_hit != hits) {
		reset(execute);
		//checks again for execute after a hit
		reset(execute_check);
		set_detection_radius();
	}
}

/// <summary>
/// simulates the entire game in the game loop
/// </summary>
/// <param name="input">key pressed</param>
/// <param name="dt">time between frames, necessary for physics appications</param>
/// <returns></returns>
internal void
simulate_game(Input* input, float dt) {
	//clear the screen each execution to update the window content depending on the player's inputs
	draw_arena_borders(initial_data.arena_half_size_x, initial_data.arena_half_size_y, color.black);
	//playing area
	draw_rect(0, 0, initial_data.arena_half_size_x, initial_data.arena_half_size_y, color.main);
#if debug
	//debug related values scattered all around the window
	draw_rect(0, 0, 2 * initial_data.arena_half_size_x, .09, 0x00ff00);
	draw_rect(0, 0, .1, 2 * initial_data.arena_half_size_y, 0x00ff00);
	draw_rect(detection_radius, 0, .1, 2 * initial_data.arena_half_size_y, 0xff0000);
	draw_number(difficulty, -initial_data.arena_half_size_x, initial_data.arena_half_size_y, 1, color.white);
	draw_number(hits, 0, -initial_data.arena_half_size_y, 1, color.white);
	draw_number(execute_check, initial_data.arena_half_size_x, -initial_data.arena_half_size_y + 20, 1, color.white);
	draw_number(execute, initial_data.arena_half_size_x, -initial_data.arena_half_size_y + 10, 1, color.white);
	draw_number(std::abs(speed_multiplier), initial_data.arena_half_size_x, -initial_data.arena_half_size_y, 1, speed_multiplier < 0 ? color.lose : color.win);
	draw_number(std::abs(player_2_speed), -initial_data.arena_half_size_x + 5, -initial_data.arena_half_size_y, 1, player_2_speed < 0 ? color.lose : color.win);
	draw_number(std::abs(player_2_acceleration), -initial_data.arena_half_size_x + 25, -initial_data.arena_half_size_y, 1, player_2_acceleration < 0 ? color.lose : color.win);
#endif
	//displays a menu at any given moment
	if (pressed(BUTTON_ESCAPE) && current_gamestate != GS_QUITMENU) set_next_gamestate(GS_QUITMENU);

	//sets the behaviour of the game loop depending on the current_gamestate
	switch (current_gamestate)
	{
	case GS_MENU:
	{
		//Button selection
		if (pressed(BUTTON_UP)) active_button > 0 ? active_button-- : active_button = 3, play_sound(sound.select);
		if (pressed(BUTTON_DOWN)) active_button < 3 ? active_button++ : active_button = 0, play_sound(sound.select);

		//Button action
		if (pressed(BUTTON_ENTER)) {
			play_sound(sound.enter);
			//some resets for a proper menu display
			max_score = initial_data.max_score;
			difficulty = 1;
			reset(modifier);
			reset(ascension);
			reset(enemy_is_ai);
			switch (active_button)
			{
			case 0:
				// sets whether player_2 is ai or human
				enemy_is_ai = true;
				set_next_gamestate(GS_DIFFICULTYMENU);
				break;
			case 1:
				enemy_is_ai = false;
				set_next_gamestate(GS_DIFFICULTYMENU);
				break;
			case 2:
				enemy_is_ai = true;
				ascension = true;
				max_score = initial_data.max_score;
				set_next_gamestate(GS_DIFFICULTYMENU);
				break;
			case 3:
				activate_sound = !activate_sound;
				break;
			}
		}
		//Menu items rendering
		draw_button(15, 1, "SINGLEPLAYER", active_button == 0);
		draw_button(5, 1, "MULTIPLAYER", active_button == 1);
		draw_button(-5, 1, "ASCENSION", active_button == 2);
		activate_sound ? draw_button(-15, 1, "SOUND ON", active_button == 3) : draw_button(-15, 1, "SOUND OFF", active_button == 3);
	}
	break;
	case GS_GAMEPLAY:
	{
		//"natural" decceleration
		reset(player_1_acceleration);
		reset(player_2_acceleration);
		//tracks players inputs;
		if (is_down(BUTTON_UP)) player_1_acceleration += initial_data.players_accelertion;
		if (is_down(BUTTON_DOWN)) player_1_acceleration -= initial_data.players_accelertion;
#if debug
		//AI player 1
		if (ball_pos_y < player_1_pos_y) player_1_acceleration -= 5000;
		if (ball_pos_y > player_1_pos_y) player_1_acceleration += 5000;
#endif
		//human 2nd player
		if (!enemy_is_ai) {
			if (is_down(BUTTON_Z)) player_2_acceleration += initial_data.players_accelertion;
			if (is_down(BUTTON_S)) player_2_acceleration -= initial_data.players_accelertion;
		}
		else {
			simulate_ai();
		}
		if (pressed(BUTTON_RIGHT)) ball_speed_x *= 1.5;
		if (pressed(BUTTON_LEFT)) ball_speed_x *= 0.5;
#if debug
		//instant win
		if (pressed(BUTTON_SPACE)) {
			player_1_score = max_score - 1;
			ball_pos_x = -initial_data.arena_half_size_x;
		}
		//difficulty hot changer
		if (pressed(BUTTON_Q)) { cap_min(difficulty, 1); set_detection_radius(); }
		if (pressed(BUTTON_D)) { cap_max(difficulty, 5); set_detection_radius(); }
#endif
		//players logic
		simulate_player(&player_1_pos_y, &player_1_speed, player_1_acceleration, dt);
		simulate_player(&player_2_pos_y, &player_2_speed, player_2_acceleration, dt);
		//Simulate Ball
		{
			//ball's movements equations
			ball_pos_x += ball_speed_x * dt * pow(hits + 1, 0.2);
			ball_pos_y += ball_speed_y * dt * pow(hits + 1, 0.1);
			//player ball collision
			//AABB vs AABB (axis aligned bounding box) collision
			//checks collision on all 4 sides of the player
			//though it seems unecessary in this case checking right side collision for player 1
			//prevents the game for restarting when moving to last ball position after the ball has passed the player
			//p1 and ball
			if (aabb_vs_aabb(
				ball_pos_x,
				ball_pos_y,
				initial_data.ball_half_size,
				initial_data.players_pos_x,
				player_1_pos_y,
				initial_data.players_half_size_x,
				initial_data.players_half_size_y
			)
				) {
				play_sound(sound.beep);
				//caps ball posistion to to player pos (while accounting for half_sizes)
				ball_pos_x = initial_data.players_pos_x - initial_data.players_half_size_x - initial_data.ball_half_size;
				//sends the ball in the opposite direction
				ball_speed_x *= -1;
				//ball y speed = y speed of the player + how far from the center of the player the ball is hit
				ball_speed_y = (ball_pos_y - player_1_pos_y) * 2 + player_1_speed * .75f;
				hits++;
			}
			//p2 and ball
			else if (aabb_vs_aabb(
				ball_pos_x,
				ball_pos_y,
				initial_data.ball_half_size,
				-initial_data.players_pos_x,
				player_2_pos_y,
				initial_data.players_half_size_x,
				initial_data.players_half_size_y
			)) {
				play_sound(sound.beep);
				ball_pos_x = -initial_data.players_pos_x + initial_data.players_half_size_x + initial_data.ball_half_size;
				ball_speed_x *= -1;
				ball_speed_y = (ball_pos_y - player_2_pos_y) * 2 + player_2_speed * .75f;
				hits++;
			}
			//ball collision with the arena (up and down)
			//ball position set to the limit if greater of lower than arena half size
			if (ball_pos_y + initial_data.ball_half_size > initial_data.arena_half_size_y) {
				play_sound(sound.wall);
				ball_pos_y = initial_data.arena_half_size_y - initial_data.ball_half_size;
				ball_speed_y *= -0.8;
			}
			else if (ball_pos_y - initial_data.ball_half_size < -initial_data.arena_half_size_y) {
				play_sound(sound.wall);
				ball_pos_y = -initial_data.arena_half_size_y + initial_data.ball_half_size;
				ball_speed_y *= -0.8;
			}
			//arena collision (left and right)
			//ball position x reset after a collision with left or right side of the arena
			//checks ball collision on left and right side
			if (ball_pos_x + initial_data.ball_half_size > initial_data.arena_half_size_x
				|| ball_pos_x - initial_data.ball_half_size < -initial_data.arena_half_size_x) {
				//resets needed for smooth transitions
				reset(player_1_acceleration);
				reset(player_2_acceleration);
				reset(ball_speed_x);
				reset(ball_speed_y);
				//p1's side collision
				if (ball_pos_x + initial_data.ball_half_size > initial_data.arena_half_size_x) {
					//oppsite player score increamentation
					player_2_score++;
					//ends the game if a player has reached max_score or transition to next round
					player_2_score == max_score ? set_next_gamestate(GS_VICTORYSCREEN) : set_next_gamestate(GS_RESETTRANSITION);
				}
				//p2's side collision
				else {
					player_1_score++;
					player_1_score == max_score ? set_next_gamestate(GS_VICTORYSCREEN) : set_next_gamestate(GS_RESETTRANSITION);
				}
			}
		}
		render_game(
			player_1_pos_y,
			player_1_score,
			initial_data.players_color,
			player_2_pos_y,
			player_2_score,
			initial_data.players_color,
			initial_data.players_pos_x,
			initial_data.players_half_size_x,
			initial_data.players_half_size_y,
			ball_pos_x,
			ball_pos_y,
			initial_data.ball_half_size,
			color.white,
			initial_data.score_pos_x,
			initial_data.score_pos_y,
			initial_data.score_size,
			player_1_score == max_score - 1 ? color.gold : color.white,
			player_2_score == max_score - 1 ? color.gold : color.white
		);
		//displays the ascension difficulty and possiblyo the modifier at the bottom of the screen
		if (ascension) {
			if (modifier == MOD_NORMAL) {
				draw_text_centered(difficulties_strings[difficulty - 1], -initial_data.arena_half_size_y - 1, .4, color.white);
			}
			else
			{
				draw_text_centered(difficulties_strings[difficulty - 1], -initial_data.arena_half_size_y - 0.8, .25, color.white);
				draw_text_centered(modifiers_strings[modifier], -initial_data.arena_half_size_y - 3, .25, color.white);
			}
		}
		if (modifier == MOD_VOID) {
			//updtates on current hits
			void_expension_formula = pow(hits, 0.8) * 1.1;
			set_detection_radius();
			//THE VOID
			draw_rect(0, 0, void_expension_formula, initial_data.arena_half_size_y, color.black);
			//sadly scores have to be rendered on top of the render_game funtion
			//to avoid either having them behind the void or having to make hte
			//function calls even longer
			if (void_expension_formula > initial_data.score_pos_x - initial_data.score_size / 2) {
				draw_number(player_2_score, -initial_data.score_pos_x, initial_data.score_pos_y, initial_data.score_size, player_2_score == max_score - 1 ? color.gold : color.white);
				draw_number(player_1_score, initial_data.score_pos_x, initial_data.score_pos_y, initial_data.score_size, player_1_score == max_score - 1 ? color.gold : color.white);
			}
		}
		//modifiers dependent logic
		if (modifier == MOD_PORTAL) {
			//resets the timer and generate new portals coordinates when time runs out
			if (portal_timer <= 0) {
				portal_timer = initial_data.portal_timer;
				generate_portal_coordinates();
			}
			//collision handling between the ball and both sides of the portals
			{
				//left portal left to right movement
				if (aabb_vs_aabb(
					ball_pos_x + initial_data.ball_half_size,
					ball_pos_y,
					initial_data.ball_half_size,
					portal_1_pos_x - initial_data.portal_half_size_x,
					portal_1_pos_y,
					initial_data.portal_half_size_x,
					initial_data.portal_half_size_y
				) && ball_speed_x > 0)
				{
					play_sound(sound.portal);
					//in execute mod ai awaits the ball at y=0 before moving and reset its state
					//however if the ball goes through the portal ai behavior is not updated
					//this fixes it
					if (execute) { reset(execute); set_detection_radius(); }
					//sets the ball coordinates to the other portal opposite side
					ball_pos_x = portal_2_pos_x, ball_pos_y = portal_2_pos_y;
				}
				//left portal right to left movement
				if (aabb_vs_aabb(
					ball_pos_x - initial_data.ball_half_size,
					ball_pos_y,
					initial_data.ball_half_size,
					portal_1_pos_x + initial_data.portal_half_size_x,
					portal_1_pos_y,
					initial_data.portal_half_size_x,
					initial_data.portal_half_size_y)
					&& ball_speed_x < 0)
				{
					play_sound(sound.portal);
					if (execute) { reset(execute); set_detection_radius(); }
					ball_pos_x = portal_2_pos_x, ball_pos_y = portal_2_pos_y;
				}
				//right portal left to right movement
				if (aabb_vs_aabb(
					ball_pos_x + initial_data.ball_half_size,
					ball_pos_y,
					initial_data.ball_half_size,
					portal_2_pos_x - initial_data.portal_half_size_x,
					portal_2_pos_y,
					initial_data.portal_half_size_x,
					initial_data.portal_half_size_y
				) && ball_speed_x > 0)
				{
					play_sound(sound.portal);
					if (execute) { reset(execute); set_detection_radius(); }
					ball_pos_x = portal_1_pos_x + initial_data.ball_half_size, ball_pos_y = portal_1_pos_y;
				}
				//right portal right to left movement
				if (aabb_vs_aabb(
					ball_pos_x - initial_data.ball_half_size,
					ball_pos_y,
					initial_data.ball_half_size,
					portal_2_pos_x + initial_data.portal_half_size_x,
					portal_2_pos_y,
					initial_data.portal_half_size_x,
					initial_data.portal_half_size_y
				) && ball_speed_x < 0)
				{
					play_sound(sound.portal);
					if (execute) { reset(execute); set_detection_radius(); }
					ball_pos_x = portal_1_pos_x - initial_data.ball_half_size, ball_pos_y = portal_1_pos_y;
				}
			}
			//portals rendering
			draw_rect(
				portal_1_pos_x,
				portal_1_pos_y,
				initial_data.portal_half_size_x,
				initial_data.portal_half_size_y,
				color.gold
			);
			draw_rect(
				portal_2_pos_x,
				portal_2_pos_y,
				initial_data.portal_half_size_x,
				initial_data.portal_half_size_y,
				color.gold
			);
			portal_timer -= dt;
		}
		if (modifier == MOD_BLACKOUT) {
			if (blackout_timer <= 0.2) {
				set_detection_radius();
				//overlays the entire window in black
				draw_rect(0, 0, 2 * initial_data.arena_half_size_x, 2 * initial_data.arena_half_size_y, color.black);
				//resets the timer when it runs out
				if (blackout_timer <= 0) {
					blackout_timer = get_random_number(5 - hits / 20, 10 - hits / 20);
					set_detection_radius();
				}
			}
			blackout_timer -= dt;
		}
		if (modifier == MOD_PHANTOM) {
			if (phantom_timer <= 0.5) {
				set_detection_radius();
				//let's just overlay the ball with another one the color of the arena
				//to avoid modifying the whole rendering process
				draw_circle(ball_pos_x, ball_pos_y, initial_data.ball_half_size, color.main);
				//resets the timer when it runs out
				if (phantom_timer <= 0) {
					phantom_timer = get_random_number(2, 5);
					set_detection_radius();
				}
			}
			phantom_timer -= dt;
		}
	}
	break;
	case GS_QUITMENU:
	{
		if (previous_gamestate != GS_MENU && previous_gamestate != GS_DIFFICULTYMENU) {
			//Button selection
			if (pressed(BUTTON_UP)) active_button > 0 ? active_button-- : active_button = 4, play_sound(sound.select);
			if (pressed(BUTTON_DOWN)) active_button < 4 ? active_button++ : active_button = 0, play_sound(sound.select);
			//Button action
			if (pressed(BUTTON_ENTER)) {
				play_sound(sound.enter);
				//CONTINUE
				switch (active_button)
				{
					//continue
				case 0:
					//when continuing during a reset transition players score were reset
					//prevents score reset when continuing during a reset
					//pretends player was not in a menu at all
					if (previous_gamestate == GS_RESETTRANSITION) {
						set_previous_gamestate(GS_GAMEPLAY);
					}
					else {
						set_previous_gamestate(GS_QUITMENU);
					}
					break;
					//restart
				case 1:
					//resets needed for smooth transitions
					reset(player_1_acceleration);
					reset(player_2_acceleration);
					reset(ball_speed_x);
					reset(ball_speed_y);
					set_next_gamestate(GS_RESETTRANSITION);
					break;
					//main menu
				case 2:
					set_next_gamestate(GS_MENU);
					break;
				case 3:
					activate_sound = !activate_sound;
					break;
					//quit
				case 4:
					running = false;
					break;
				default:
					break;
				}
			}
			//MENU rendering
			draw_button(20, 1, "CONTINUE", active_button == 0);
			draw_button(10, 1, "RESTART", active_button == 1);
			draw_button(0, 1, "MAIN MENU", active_button == 2);
			activate_sound ? draw_button(-10, 1, "SOUND ON", active_button == 3) : draw_button(-10, 1, "SOUND OFF", active_button == 3);
			draw_button(-20, 1, "QUIT", active_button == 4);
		}
		else {
			//Button selection
			if (pressed(BUTTON_UP) || pressed(BUTTON_DOWN)) {
				active_button = ~active_button, play_sound(sound.select);
			}
			//Button action
			if (pressed(BUTTON_ENTER)) {
				play_sound(sound.enter);
				if (!active_button) {
					set_previous_gamestate(GS_QUITMENU);
				}
				else {
					running = false;
				}
			}
			//MENU RENDERING
			draw_button(5, 1, "CONTINUE", ~active_button);
			draw_button(-5, 1, "QUIT GAME", active_button);
		}
	}
	break;
	case GS_DIFFICULTYMENU:
	{
		//Button selection
		if (pressed(BUTTON_UP)) active_button > 0 ? active_button-- : active_button = 2, play_sound(sound.select);
		if (pressed(BUTTON_DOWN)) active_button < 2 ? active_button++ : active_button = 0, play_sound(sound.select);
		//difficulty change
		if (!ascension && enemy_is_ai) {
			if (active_button == 0 && pressed(BUTTON_RIGHT)) if(cap_max(difficulty, 5)) play_sound(sound.select);
			if (active_button == 0 && pressed(BUTTON_LEFT)) if(cap_min(difficulty, 1)) play_sound(sound.select);
		}
		//modifier change
		if (active_button == 1 && pressed(BUTTON_RIGHT)) if(cap_max(modifier, MOD_COUNT - 1)) play_sound(sound.select);
		if (active_button == 1 && pressed(BUTTON_LEFT)) if(cap_min(modifier, 0)) play_sound(sound.select);
		//max_score change
		if (!ascension) {
			if (active_button == 2 && pressed(BUTTON_RIGHT)) if (max_score < 25) max_score += 5, play_sound(sound.select);
			if (active_button == 2 && pressed(BUTTON_LEFT)) if (max_score > 5) max_score -= 5, play_sound(sound.select);
		}
		//starts the game
		if (pressed(BUTTON_ENTER)) {
			play_sound(sound.enter);
			set_initial_data(dt, true, false, GS_RESETTRANSITION);
		}
		//string concatenation seems to be the hardest thing ever ¯\_(ツ)_/¯
		//I JUST LOVE TERNAY OPERATOR
		//RENDERING
		draw_text("-", -40, 10, 1, difficulty != 1 ? color.active : color.inactive);
		draw_button(10, 1, difficulties_strings[difficulty - 1], active_button == 0, color.inactive,
			(!ascension && enemy_is_ai) ? color.active : color.disabled);
		draw_text("+", 40, 10, 1, (ascension || !enemy_is_ai) ? color.inactive : (difficulty != 5 ? color.active : color.inactive));

		draw_text("-", -40, 0, 1, modifier != 0 ? color.active : color.inactive);
		draw_button(0, 1, modifiers_strings[modifier], active_button == 1);
		draw_text("+", 40, 0, 1, modifier != MOD_COUNT - 1 ? color.active : color.inactive);

		draw_text("-", -40, -10, 1, max_score != initial_data.max_score ? color.active : color.inactive);
		draw_text("BEST OF : ", -25, -10, 1, active_button == 2 ? (!ascension ? color.active : color.disabled) : color.inactive);
		draw_number(max_score, 30, -13, 1.5, active_button == 2 ? (!ascension ? color.active : color.disabled) : color.inactive);
		draw_text("+", 40, -10, 1, ascension ? color.inactive : (max_score != 25 ? color.active : color.inactive));
	}
	break;
	case GS_RESETTRANSITION:
	{
		//the illusion of free choice
		//players acceleration and ball speed need to be reset before this function
		//this condition will probably only be true in ascension mod
		if (player_1_score == max_score || player_2_score == max_score) {
			reset(player_1_acceleration);
			reset(player_2_acceleration);
			reset(ball_speed_x);
			reset(ball_speed_y);
			set_initial_data(dt, false, true, GS_GAMEPLAY);
		}
		else {
			set_initial_data(dt, false, true, GS_GAMEPLAY);
		}
		//renders gameplay elements while transitioning
		render_game(
			player_1_pos_y,
			player_1_score,
			color.disabled,
			player_2_pos_y,
			player_2_score,
			color.disabled,
			initial_data.players_pos_x,
			initial_data.players_half_size_x,
			initial_data.players_half_size_y,
			ball_pos_x,
			ball_pos_y,
			initial_data.ball_half_size,
			color.disabled,
			initial_data.score_pos_x,
			initial_data.score_pos_y,
			initial_data.score_size,
			player_1_score == max_score - 1 ? color.gold : color.white,
			player_2_score == max_score - 1 ? color.gold : color.white
		);
		//displays the ascension difficulty at the bottom of the screen
		if (ascension) {
			if (modifier == MOD_NORMAL) {
				draw_text_centered(difficulties_strings[difficulty - 1], -initial_data.arena_half_size_y - 1, .4, color.white);
			}
			else
			{
				draw_text_centered(difficulties_strings[difficulty - 1], -initial_data.arena_half_size_y - 0.8, .25, color.white);
				draw_text_centered(modifiers_strings[modifier], -initial_data.arena_half_size_y - 3, .25, color.white);
			}
		}
	}
	break;
	case GS_VICTORYSCREEN:
	{
		//ascension strings display after an ascension after player_1 win
		if (ascension && difficulty == 5 && player_1_score > player_2_score) {
			play_sound_once(sound.ascended);
			draw_text_centered("CONGRATULATIONS PLAYER ONE", 5, 1, color.win);
			switch (modifier)
			{
			case MOD_PORTAL:
				draw_text_centered("YOU HAVE CONQUERED SPACE AND TIME", -5, 1, color.win);
				break;
			case MOD_PHANTOM:
				draw_text_centered("YOU HAVE CONQUERED INVISIBILIY", -5, 1, color.win);
				break;
			case MOD_VOID:
				draw_text_centered("YOU HAVE CONQUERED THE VOID", -5, 1, color.win);
				break;
			case MOD_BLACKOUT:
				draw_text_centered("YOU HAVE CONQURERED DARKNESS", -5, 1, color.win);
				break;
			default:
				draw_text_centered("YOU HAVE ASCENDED", -5, 1, color.win);
				break;
			}
		}
		//other win strings
		else {
			if (player_1_score > player_2_score) {
				play_sound_once(sound.win);
				draw_text_centered("PLAYER ONE WINS!", 20, 1.5, color.white);
			}
			else {
				enemy_is_ai ? play_sound_once(sound.game_over) : play_sound_once(sound.win);
				draw_text_centered("PLAYER TWO WINS!", 20, 1.5, color.white);
			}
			//scores display
			draw_number(player_2_score, -20, -20, 2, player_2_score == max_score ? color.win : color.white);
			draw_text("-", -3, -15, 2, color.white);
			draw_number(player_1_score, 20, -20, 2, player_1_score == max_score ? color.win : color.white);
		}
		if (victory_timer <= 0) {
			if (!ascension) {
				set_next_gamestate(GS_MENU);
			}
			else {
				//between ascension rounds transition
				if (difficulty < 5 && player_1_score > player_2_score) {
					difficulty++;
					max_score = difficulty * 5;
					set_next_gamestate(GS_RESETTRANSITION);
				}
				//timer gets doubled for ascension victories
				else
				{
					//let's flip a doubled secods timer out of thin air
					if (victory_timer <= -initial_data.victory_timer) set_next_gamestate(GS_MENU);
				}
			}
		}
		victory_timer -= dt;
	}
	break;
	}
}

//call it a day

//optional 
// ai changes
// true ascension
// proper screen scaling