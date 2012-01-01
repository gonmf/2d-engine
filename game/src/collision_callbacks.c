
static void powerup(object_t * obj1, object_t * obj2){
	if(obj1 == player && obj2->dispose_of == 0){
		obj2->dispose_of = 1;
		score += 5000;
	}else
		if(obj2 == player && obj1->dispose_of == 0){
			obj1->dispose_of = 1;
			score += 5000;
		}
}
static void ship_hit(object_t * obj1, object_t * obj2){
	if(lives == 0)
		end_game();
	--lives;
	force_next_paint_engine(en);
	if(obj1 == player && obj2->dispose_of == 0){
		player->left = player->start_left;
		player->top = player->start_top;
		obj2->dispose_of = 1;
		// Destroy all projectiles (not enemies)
		object_t * o = en->objects;
		while(o != NULL){
			if(o->mask & COLLISION4)
				o->dispose_of = 1;
			o = o->next;
		}
	}else
		if(obj2 == player && obj1->dispose_of == 0){
			player->left = player->start_left;
			player->top = player->start_top;
			obj1->dispose_of = 1;
			// Destroy all projectiles (not enemies)
			object_t * o = en->objects;
			while(o != NULL){
				if(o->mask & COLLISION4)
					o->dispose_of = 1;
				o = o->next;
			}
		}
}
static void enemy_destroyed(object_t * obj1, object_t * obj2){
	score += 100;
	obj1->dispose_of = 1;
	obj2->dispose_of = 1;
	object_t * o = obj1->id == 2 ? obj1 : obj2;
	if(o->id != 2)
		return;
	if(en->paint_count % 5 == 0)
		add_object_engine(en, new_object(5, 0, COLLISION6, 0, o->left, o->top, 1, pattern4));
}