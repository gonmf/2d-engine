static void stage1(u32 step){
	if(step == 0){
		paint_engine(en);
		u8 prev_auto_commit = en->auto_commit;
		en->auto_commit = 1;
		sprite_t * img = new_sprite(20);
		u32 last;
		for(u16 i = 1; i <= 10; ++i){
			fade_in_sprite_engine(en, img, en->width / 2 - (img->width / 2), en->height / 2 - (img->height / 2), i * 10);
			last = get_millis_clock();
			while(get_millis_clock() - last < 100)
				poll_keybd_engine(en, Q, &kbd_handler);
		}
		en->auto_commit = prev_auto_commit;
		last = get_millis_clock();
		while(get_millis_clock() - last < 3000)
			poll_keybd_engine(en, Q, &kbd_handler);
		/*
		 * Add players object
		 */
		add_object_engine(en, player = new_object(0, 1, COLLISION1, 1, en->width / 2 - new_sprite(0)->width / 2, en->height - 2, 2, &auto_pilot));
//		add_sprite_object(player, new_sprite(9));
		/*
		 * Add 2 player shields
		 */
		add_object_engine(en, new_object(4, 5, COLLISION5, 1, player->left, player->top, 0, &shields1));
		add_object_engine(en, new_object(4, 5, COLLISION5, 1, player->left, player->top, 0, &shields2));
		/*
		 * Add some silly enemies
		 */
		add_object_engine(en, new_object(2, 2, COLLISION3, 1, 80 - 6, 80, 1, &sideways2));
		add_object_engine(en, new_object(2, 2, COLLISION3, 1, 120 - 6, 90, 1, &sideways1));
		add_object_engine(en, new_object(2, 2, COLLISION3, 1, 200 - 6, 100, 1, &sideways2));
		add_object_engine(en, new_object(2, 2, COLLISION3, 1, 280 - 6, 90, 1, &sideways1));
		add_object_engine(en, new_object(2, 2, COLLISION3, 1, 320 - 6, 80, 1, &sideways2));
	}
}