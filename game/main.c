/*
 * Collision masks used:
 * COLLISION1 - Player ship
 * COLLISION2 - Player weapons
 * COLLISION3 - Enemy ships
 * COLLISION4 - Enemy weapons
 * COLLISION5 - Player shields
 * COLLISION6 - Powerup
 *
 * Collision detection is performed while painting.
 *
 * Instructions:
 * z to Fire
 * x to Fire alt
 * q to Quit
 * p to Pause
 */
#define ARROW_LEFT 113
#define ARROW_RIGHT 114
#define ARROW_UP 111
#define ARROW_DOWN 116
#define P 33
#define Q 24
#define X 53
#define Z 52
static engine_t * en;
static object_t * player;
static void (* stage)(u32) = &stage1;
static u8 lives = 3;
static u32 score;
int main(int argc, char * argv[]){
	/*
	 * Creating new engine
	 */
	en = new_engine(400, 600, NULL);
	en->auto_commit = 0;
	/*
	 * Add collision detection definitions
	 */
	add_collision_engine(en, COLLISION1 | COLLISION3, &ship_hit);
	add_collision_engine(en, COLLISION1 | COLLISION4, &ship_hit);
	add_collision_engine(en, COLLISION2 | COLLISION3, &enemy_destroyed);
	add_collision_engine(en, COLLISION4 | COLLISION5, &enemy_destroyed);
	add_collision_engine(en, COLLISION1 | COLLISION6, &powerup);
	/*
	 * Start infinite cycle with engine stepping at 40 FPS
	 */
	stage(en->paint_count);
	step_engine(en);
	u32 last = get_millis_clock();
	while(1){
		u8 freetime = 0;
		u32 newt = get_millis_clock();
		if(newt - last > 25){ // 40 FPS
			/*
			 * Paint the screen if needed and execute code associated with each
			 * object's movement and so on
			 */
			if(newt - last < 50){
				last = newt;
				step_engine(en);
				/*
				 * Lives remaining
				 */
				for(u8 i = 0; i < lives; ++i)
					draw_sprite_engine(en, new_sprite(6), 8 + 12 * i, 8);
				/*
				 * Score
				 */
				u32 tsc = score;
				u8 coutn = 0;
				do{
					u32 tscm = tsc % 10;
					tsc /= 10;
					draw_sprite_engine(en, new_sprite(tscm + 10), en->width - 12 - 10 * coutn, 5);
					++coutn;
				}while(tsc);
				/*
				 * Draw UI small things manually
				 */
				manual_commit_engine(en);
			}else{
				printf("Frame %u: Skipping %u frames.\n", en->paint_count, (newt - last) / 26); fflush(stdout);
				/*
				 * FRAMESKIPPING
				 */
				for(u8 skip = 0; skip < (newt - last) / 26; ++skip){
					++en->paint_count;
					run_objects_engine(en);
				}
				/*
				 * Manually correct smoothing
				 */
				for(u32 i = 0; i < en->width * en->height; ++i)
					en->smudge2[i] -= (en->smudge2[i] >= (newt - last) / 26) ? (newt - last) / 26 : en->smudge2[i];
				last = newt;
			}
			/*
			 * Check for IO action
			 */
			poll_keybd_engine(en, ARROW_LEFT, kbd_handler);
			poll_keybd_engine(en, ARROW_RIGHT, kbd_handler);
			poll_keybd_engine(en, ARROW_DOWN, kbd_handler);
			poll_keybd_engine(en, ARROW_UP, kbd_handler);
			poll_keybd_engine(en, Z, kbd_handler); // z - Fire
			poll_keybd_engine(en, X, kbd_handler); // x - Fire alt
			poll_keybd_engine(en, Q, kbd_handler); // q - Quit
			if(poll_keybd_engine(en, P, kbd_handler)) // p - Pause
				last = get_millis_clock();
			/*
			 * Load stage script, might decide to add enemies, move on to next
			 * stage, etc
			 */
			stage(en->paint_count);
		}else
			if(freetime == 0){
				freetime_engine(en);
				freetime = 1;
			}
	}
	return EXIT_SUCCESS;
}