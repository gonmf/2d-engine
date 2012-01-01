/*
 * Keyboard callback function responsible for moving players object and some
 * other controls
 */
static u32 last_fired = 0; // To delay firing
static void kbd_handler(u8 keycode){
	u32 newt;
	switch(keycode){
		case X: // X Fire alt
			newt = get_millis_clock();
			if(newt - last_fired < 125) // 8 shots/sec
				break;
			last_fired = newt;
			add_object_engine(en, new_object(1, 4, COLLISION2, 0, player->left + 2, player->top + 8, 0, &pattern5));
			add_object_engine(en, new_object(1, 4, COLLISION2, 0, player->left + 9, player->top + 8, 0, &pattern6));
			break;
		case Z: // z Fire
			newt = get_millis_clock();
			if(newt - last_fired < 125) // 8 shots/sec
				break;
			last_fired = newt;
			add_object_engine(en, new_object(1, 4, COLLISION2, 0, player->left + 2, player->top + 8, 0, &pattern2));
			add_object_engine(en, new_object(1, 4, COLLISION2, 0, player->left + 9, player->top + 8, 0, &pattern2));
			break;
		case ARROW_UP: // arrow up
			if(player->top > 0)
				player->top = player->top - 3;
			break;
		case ARROW_LEFT: // arrow left
			if(player->left > 0)
				player->left = player->left - 3;
			break;
		case ARROW_RIGHT: // arrow right
			if(player->left < en->width - player->sprites->sprite->width)
				player->left = player->left + 3;
			break;
		case ARROW_DOWN: // arrow down
			if(player->top < en->height - player->sprites->sprite->height)
				player->top = player->top + 3;
			break;
		case P: // p
			pause_game();
			break;
		case Q: // q
			exit(EXIT_SUCCESS);
	}
}
static void pause_game(){
	sprite_t * img = new_sprite(7);
	u32 last;
	for(u16 i = 1; i <= 10; ++i){
		fade_in_sprite_engine(en, img, en->width / 2 - (img->width / 2), en->height / 2 - (img->height / 2), i * 10);
		last = get_millis_clock();
		while(get_millis_clock() - last < 100)
			if(poll_keybd_engine(en, Q, NULL))
				kbd_handler(Q);
	}
	while(!poll_keybd_engine(en, P, NULL)) // p - Unpause
		if(poll_keybd_engine(en, Q, NULL))
				kbd_handler(Q);
	last = get_millis_clock();
	while(get_millis_clock() - last < 200);
}
static void end_game(){
	sprite_t * img = new_sprite(8);
	u32 last;
	for(u16 i = 1; i <= 10; ++i){
		fade_in_sprite_engine(en, img, en->width / 2 - (img->width / 2), en->height / 2 - (img->height / 2), i * 10);
		last = get_millis_clock();
		while(get_millis_clock() - last < 100);
	}
	last = get_millis_clock();
	while(get_millis_clock() - last < 3000);
	exit(EXIT_SUCCESS);
}
static inline double radians_to_degrees(double radians){
	return radians * 0.0174532925;
}