
static void pattern4(object_t * obj){
	obj->top += 2;
}
static void pattern1(object_t * obj){
	obj->left += obj->age % 60 > 30 ? 2 : -2;
	obj->top += 2;
}
static void pattern3(object_t * obj){
	obj->left += obj->age % 60 > 30 ? -2 : 2;
	obj->top += 2;
}
static void pattern2(object_t * obj){
	obj->top -= 7;
}
static void pattern5(object_t * obj){
	obj->left -= 1;
	obj->top -= 7;
}
static void pattern6(object_t * obj){
	obj->left += 1;
	obj->top -= 7;
}
static void movement0(object_t * obj){
	if((obj->age % 60) > 29)
		obj->top += 2;
	else
		obj->top -= 2;	
}
static void sideways1(object_t * obj){
	movement0(obj);
	if(obj->age % 50 == 0)
		add_object_engine(en, new_object(3, 3, COLLISION4, 0, obj->left, obj->top + 6, 3, &pattern1));
	else
		if(obj->age % 25 == 0)
			add_object_engine(en, new_object(3, 3, COLLISION4, 0, obj->left, obj->top + 6, 3, &pattern3));
}
static void sideways2(object_t * obj){
	movement0(obj);
	if(obj->age % 50 == 0)
		add_object_engine(en, new_object(3, 3, COLLISION4, 0, obj->left, obj->top + 6, 3, &pattern3));
	else
		if(obj->age % 25 == 0)
			add_object_engine(en, new_object(3, 3, COLLISION4, 0, obj->left, obj->top + 6, 3, &pattern1));
}
static void shields1(object_t * obj){
	double angle = (double)(obj->age % 360);
	angle *= 7;
	obj->left = player->left + (u16)(cos(radians_to_degrees(angle)) * 25);
	obj->top = player->top - (u16)(sin(radians_to_degrees(angle)) * 25) + (player->sprites->sprite->height - player->sprites->sprite->width) / 2;
}
static void shields2(object_t * obj){
	double angle = (double)((obj->age + 180) % 360);
	angle *= 7;
	obj->left = player->left + (u16)(cos(radians_to_degrees(angle)) * 25);
	obj->top = player->top - (u16)(sin(radians_to_degrees(angle)) * 25) + (player->sprites->sprite->height - player->sprites->sprite->width) / 2;
}
static void auto_pilot(object_t * o){
	if(o->age > 30){
		o->start_top -= 50;
		o->func = NULL;
	}else
		if(o->age > 20)
			o->top -= 3;
		else
			o->top -= 2;
}