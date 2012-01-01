#include "common.h"
static sprite_stack_t * sprite_stack_pool = NULL;
static sprite_stack_t * _alloc_sprite_stack(){
	if(sprite_stack_pool == NULL)
		return (sprite_stack_t *)malloc(sizeof(sprite_stack_t));
	sprite_stack_t * ret = sprite_stack_pool;
	sprite_stack_pool = sprite_stack_pool->next;
	return ret;
}
static void _destroy_sprite_stack(sprite_stack_t * ss){
	ss->next = sprite_stack_pool;
	sprite_stack_pool = ss;
}
engine_t * new_engine(u16 width, u16 height, sprite_t * bg){
	assert(width > 0 && height > 0);
	engine_t * en = (engine_t *)malloc(sizeof(engine_t));
	assert(en != NULL);
	en->width = width;
	en->height = height;
	en->obj_painted = (object_t **)malloc(sizeof(object_t *) * width * height);
	assert(en->obj_painted != NULL);
	en->smudge1 = (u8 *)malloc(sizeof(u8) * width * height);
	assert(en->smudge1 != NULL);
	en->smudge2 = (u8 *)malloc(sizeof(u8) * width * height);
	assert(en->smudge2 != NULL);
	en->painted = (u32 *)malloc(sizeof(u32) * width * height);
	assert(en->painted != NULL);
	en->topaint = (u32 *)malloc(sizeof(u32) * width * height);
	assert(en->topaint != NULL);
	en->topaint_flags = (u32 *)malloc(sizeof(u32) * width * height);
	assert(en->topaint_flags != NULL);
	en->dsp = XOpenDisplay(NULL);
	assert(en->dsp != NULL);
	en->win = XCreateSimpleWindow(en->dsp, DefaultRootWindow(en->dsp), 50, 50, width, height, 0, BACKGROUND, BACKGROUND);
	XMapWindow(en->dsp, en->win);
	en->gc = XCreateGC(en->dsp, en->win, 0, NULL);
	XSelectInput(en->dsp, en->win, ButtonPressMask);
	en->double_buffer = XCreatePixmap(en->dsp, en->win, width, height, 24);
	en->background = bg;
	en->objects = NULL;
	en->collision_events = NULL;
	for(u32 i = 0; i < width * height; ++i){
		en->smudge1[i] = en->smudge2[i] = 0;
		en->painted[i] = BACKGROUND;
	}
	en->paint_count = 0;
	en->auto_commit = 1;
	XSetForeground(en->dsp, en->gc, en->last_rgb = BACKGROUND);
	return en;
}
object_t * new_object(u32 class_id, u32 unique_id, u32 mask, u8 keep_alive, i16 pos_x, i16 pos_y, u8 smudge, void (* func)(object_t *)){
	object_t * ob = alloc_object();
	ob->id = unique_id;
	ob->sprites = _alloc_sprite_stack();
	ob->sprites->next = NULL;
	ob->sprites->sprite = new_sprite(class_id);
	ob->left = pos_x;
	ob->top = pos_y;
	ob->mask = mask;
	ob->next = NULL;
	ob->keep_alive = keep_alive;
	ob->dispose_of = 0;
	ob->age = 0;
	ob->start_left = ob->left;
	ob->start_top = ob->top;
	ob->func = func;
	ob->smudge = smudge;
	return ob;
}
static object_t * object_pool = NULL;
object_t * alloc_object(){
	if(object_pool == NULL)
		return (object_t *)malloc(sizeof(object_t));
	object_t * ret = object_pool;
	object_pool = object_pool->next;
	return ret;
}
void destroy_object(object_t * ob){
	assert(ob != NULL);
	ob->next = object_pool;
	object_pool = ob;
}
static void _destroy_object(object_t * ob){
	ob->next = object_pool;
	object_pool = ob;
}
void manual_commit_engine(engine_t * en){
	assert(en != NULL);
	assert(en->auto_commit == 0);
	XFlush(en->dsp);
}
void force_next_paint_engine(engine_t * en){
	assert(en != NULL);
	en->painted[0] ^= 0x00ffffff;
	en->smudge1[0] = 1;
	en->smudge2[0] = 1;
	en->painted[en->width * en->height - 1] ^= 0x00ffffff;
	en->smudge1[en->width * en->height - 1] = 1;
	en->smudge2[en->width * en->height - 1] = 1;
}
static void _force_next_paint_engine(engine_t * en){
	en->painted[0] ^= 0x00ffffff;
	en->smudge1[0] = 1;
	en->smudge2[0] = 1;
	en->painted[en->width * en->height - 1] ^= 0x00ffffff;
	en->smudge1[en->width * en->height - 1] = 1;
	en->smudge2[en->width * en->height - 1] = 1;
}
void paint_engine(engine_t * en){
	assert(en != NULL);
	// Refresh whole screen periodically
	if(en->paint_count % 400 == 0) // once every 10 seconds
		_force_next_paint_engine(en);
	// Fill topaint buffer applying average of past with background
	for(u32 i = 0; i < en->height * en->width; ++i){
		en->obj_painted[i] = NULL;
		u32 bg = en->background == NULL ? BACKGROUND : en->background->data[i];
		if(en->smudge2[i] > 0){
			u8 sn = en->smudge1[i] + 1;
			u8 sm = en->smudge2[i];
			u8 red = ((((en->painted[i] >> 16) & 0xff) * sm) / sn + ((bg >> 16) & 0xff) / sn) & 0xff;
			u8 green = ((((en->painted[i] >> 8) & 0xff) * sm) / sn + ((bg >> 8) & 0xff) / sn) & 0xff;
			u8 blue = (((en->painted[i] & 0xff) * sm) / sn + (bg & 0xff) / sn) & 0xff;
			en->topaint[i] = (red << 16) | (green << 8) | blue;
			--en->smudge2[i];
		}else
			en->topaint[i] = bg;
		en->topaint_flags[i] = 0;
	}
	object_t * ob = en->objects;
	while(ob != NULL){
		u8 ob_painted = 0;
		sprite_t * sp = ob->sprites->sprite;
		// Cycling through multiple sprites puting the first on the end
		if(ob->sprites->next != NULL){
			sprite_stack_t * inv = ob->sprites;
			ob->sprites = ob->sprites->next;
			inv->next = NULL;
			sprite_stack_t * tmp = ob->sprites;
			while(tmp->next != NULL)
				tmp = tmp->next;
			tmp->next = inv;
		}
		assert(sp != NULL);
		for(i16 spr_top = ob->top < 0 ? ob->top * -1 : 0; spr_top < sp->height; ++spr_top){
			i16 scr_top = spr_top + ob->top;
			if(scr_top >= en->height)
				break;
			for(i16 spr_left = ob->left < 0 ? ob->left * -1 : 0; spr_left < sp->width; ++spr_left){
				i16 scr_left = spr_left + ob->left;
				if(scr_left >= en->width)
					break;
				u32 rgb = sp->data[spr_top * sp->width + spr_left];
				if(rgb == TRANSPARENT)
					continue;
				if(en->topaint_flags[scr_top * en->width + scr_left]){
					// Scan for collisions
					collision_t * pevts = en->collision_events;
					while(pevts != NULL && ob->dispose_of == 0){
						if(pevts->mask_combination == (en->topaint_flags[scr_top * en->width + scr_left] | ob->mask))
							if(en->obj_painted[scr_top * en->width + scr_left]->dispose_of == 0)
								pevts->func(en->obj_painted[scr_top * en->width + scr_left], ob);
						pevts = pevts->next;
					}
				}
				if(ob->dispose_of == 0){
					en->obj_painted[scr_top * en->width + scr_left] = ob;
					en->topaint[scr_top * en->width + scr_left] = rgb;
					en->topaint_flags[scr_top * en->width + scr_left] = ob->mask;
				}else
					en->topaint[scr_top * en->width + scr_left] = rgb;
				en->smudge1[scr_top * en->width + scr_left] = en->smudge2[scr_top * en->width + scr_left] = ob->smudge;
				ob_painted = 1;
			}
		}
		if(ob_painted == 0 && ob->keep_alive == 0)
			ob->dispose_of = 1;
		ob = ob->next;
	}
	u16 min_left = en->width;
	u16 max_left = 0;
	u16 min_top = en->height;
	u16 max_top = 0;
	for(u16 i = 0; i < en->height; ++i)
		for(u16 j = 0; j < en->width; ++j){
			if(en->topaint[i * en->width + j] != en->painted[i * en->width + j]){
				en->painted[i * en->width + j] = en->topaint[i * en->width + j];
				if(i < min_top)
					min_top = i;
				if(i > max_top)
					max_top = i;
				if(j < min_left)
					min_left = j;
				if(j > max_left)
					max_left = j;
			}
		}
	if((min_left <= max_left) && (min_top <= max_top)){
		for(u16 i = min_top; i <= max_top; ++i)
			for(u16 j = min_left; j <= max_left; ++j){
				u32 rgb = en->painted[i * en->width + j];
				if(rgb != en->last_rgb){
					en->last_rgb = rgb;
					XSetForeground(en->dsp, en->gc, rgb);
				}
				XDrawPoint(en->dsp, en->double_buffer, en->gc, j, i);
			}
		XCopyArea(en->dsp, en->double_buffer, en->win, en->gc, min_left, min_top, max_left + 1, max_top + 1, min_left, min_top);
		if(en->auto_commit)
			XFlush(en->dsp);
	}
	++en->paint_count;
}
static void _paint_engine(engine_t * en){
	// Refresh whole screen periodically
	if(en->paint_count % 400 == 0) // once every 10 seconds
		_force_next_paint_engine(en);
	// Fill topaint buffer applying average of past with background
	for(u32 i = 0; i < en->height * en->width; ++i){
		en->obj_painted[i] = NULL;
		u32 bg = en->background == NULL ? BACKGROUND : en->background->data[i];
		if(en->smudge2[i] > 0){
			u8 sn = en->smudge1[i] + 1;
			u8 sm = en->smudge2[i];
			u8 red = ((((en->painted[i] >> 16) & 0xff) * sm) / sn + ((bg >> 16) & 0xff) / sn) & 0xff;
			u8 green = ((((en->painted[i] >> 8) & 0xff) * sm) / sn + ((bg >> 8) & 0xff) / sn) & 0xff;
			u8 blue = (((en->painted[i] & 0xff) * sm) / sn + (bg & 0xff) / sn) & 0xff;
			en->topaint[i] = (red << 16) | (green << 8) | blue;
			--en->smudge2[i];
		}else
			en->topaint[i] = bg;
		en->topaint_flags[i] = 0;
	}
	object_t * ob = en->objects;
	while(ob != NULL){
		u8 ob_painted = 0;
		sprite_t * sp = ob->sprites->sprite;
		// Cycling through multiple sprites puting the first on the end
		if(ob->sprites->next != NULL){
			sprite_stack_t * inv = ob->sprites;
			ob->sprites = ob->sprites->next;
			inv->next = NULL;
			sprite_stack_t * tmp = ob->sprites;
			while(tmp->next != NULL)
				tmp = tmp->next;
			tmp->next = inv;
		}
		assert(sp != NULL);
		for(i16 spr_top = ob->top < 0 ? ob->top * -1 : 0; spr_top < sp->height; ++spr_top){
			i16 scr_top = spr_top + ob->top;
			if(scr_top >= en->height)
				break;
			for(i16 spr_left = ob->left < 0 ? ob->left * -1 : 0; spr_left < sp->width; ++spr_left){
				i16 scr_left = spr_left + ob->left;
				if(scr_left >= en->width)
					break;
				u32 rgb = sp->data[spr_top * sp->width + spr_left];
				if(rgb == TRANSPARENT)
					continue;
				if(en->topaint_flags[scr_top * en->width + scr_left]){
					// Scan for collisions
					collision_t * pevts = en->collision_events;
					while(pevts != NULL && ob->dispose_of == 0){
						if(pevts->mask_combination == (en->topaint_flags[scr_top * en->width + scr_left] | ob->mask))
							if(en->obj_painted[scr_top * en->width + scr_left]->dispose_of == 0)
								pevts->func(en->obj_painted[scr_top * en->width + scr_left], ob);
						pevts = pevts->next;
					}
				}
				if(ob->dispose_of == 0){
					en->obj_painted[scr_top * en->width + scr_left] = ob;
					en->topaint[scr_top * en->width + scr_left] = rgb;
					en->topaint_flags[scr_top * en->width + scr_left] = ob->mask;
				}else
					en->topaint[scr_top * en->width + scr_left] = rgb;
				en->smudge1[scr_top * en->width + scr_left] = en->smudge2[scr_top * en->width + scr_left] = ob->smudge;
				ob_painted = 1;
			}
		}
		if(ob_painted == 0 && ob->keep_alive == 0)
			ob->dispose_of = 1;
		ob = ob->next;
	}
	u16 min_left = en->width;
	u16 max_left = 0;
	u16 min_top = en->height;
	u16 max_top = 0;
	for(u16 i = 0; i < en->height; ++i)
		for(u16 j = 0; j < en->width; ++j){
			if(en->topaint[i * en->width + j] != en->painted[i * en->width + j]){
				en->painted[i * en->width + j] = en->topaint[i * en->width + j];
				if(i < min_top)
					min_top = i;
				if(i > max_top)
					max_top = i;
				if(j < min_left)
					min_left = j;
				if(j > max_left)
					max_left = j;
			}
		}
	if((min_left <= max_left) && (min_top <= max_top)){
		for(u16 i = min_top; i <= max_top; ++i)
			for(u16 j = min_left; j <= max_left; ++j){
				u32 rgb = en->painted[i * en->width + j];
				if(rgb != en->last_rgb){
					en->last_rgb = rgb;
					XSetForeground(en->dsp, en->gc, rgb);
				}
				XDrawPoint(en->dsp, en->double_buffer, en->gc, j, i);
			}
		XCopyArea(en->dsp, en->double_buffer, en->win, en->gc, min_left, min_top, max_left + 1, max_top + 1, min_left, min_top);
		if(en->auto_commit)
			XFlush(en->dsp);
	}
	++en->paint_count;
}
u8 poll_keybd_engine(engine_t * en, u8 keycode, void (* func)(u8)){
	assert(en != NULL);
	char kbd_matrix[32];
	for(u8 i = 0; i < 32; ++i)
		kbd_matrix[i] = 0;
	XQueryKeymap(en->dsp, kbd_matrix);
	if((kbd_matrix[keycode / 8] >> (keycode % 8)) & 1){
		if(func != NULL)
			func(keycode);
		return 1;
	}
	return 0;
}
u8 poll_mouse_engine(engine_t * en, void (* func)(u32)){
	assert(en != NULL);
	XEvent ev;
	if(XCheckWindowEvent(en->dsp, en->win, ButtonPressMask, &ev)){
		if(func != NULL)
			func(ev.xbutton.y * en->width + ev.xbutton.x);
		return 1;
	}
	return 0;
}
void add_object_engine(engine_t * en, object_t * ob){
	assert(en != NULL);
	assert(ob != NULL);
	ob->next = en->objects;
	en->objects = ob;
}
static void _free_sprite_stack(sprite_stack_t * ss){
	if(ss == NULL);
		return;
	_free_sprite_stack(ss->next);
	_destroy_sprite_stack(ss);
}
void run_objects_engine(engine_t * en){
	assert(en != NULL);
	if(en->objects == NULL)
		return;
	object_t * o = en->objects;
	u16 to_clean = 0;
	while(o != NULL){
		if(o->dispose_of)
			to_clean++;
		else{
			o->age = o->age + 1;
			if(o->func != NULL)
				o->func(o);
		}
		o = o->next;
	}
	if(!to_clean)
		return;
	// clear head
	while(en->objects != NULL && en->objects->dispose_of){
		en->objects = en->objects->next;
		to_clean--;
	}
	// clear body
	object_t * prev = en->objects;
	if(prev == NULL)
		return;
	o = prev->next;
	while(to_clean){
		if(o->dispose_of){
			--to_clean;
			prev->next = o->next;
			object_t * tmp = o;
			o = o->next;
			_free_sprite_stack(o->sprites);
			_destroy_object(tmp);
			continue;
		}
		prev = o;
		o = o->next;
	}
}
static void _run_objects_engine(engine_t * en){
	if(en->objects == NULL)
		return;
	object_t * o = en->objects;
	u16 to_clean = 0;
	while(o != NULL){
		if(o->dispose_of)
			to_clean++;
		else{
			o->age = o->age + 1;
			if(o->func != NULL)
				o->func(o);
		}
		o = o->next;
	}
	if(!to_clean)
		return;
	// clear head
	while(en->objects != NULL && en->objects->dispose_of){
		en->objects = en->objects->next;
		to_clean--;
	}
	// clear body
	object_t * prev = en->objects;
	if(prev == NULL)
		return;
	o = prev->next;
	while(to_clean){
		if(o->dispose_of){
			--to_clean;
			prev->next = o->next;
			object_t * tmp = o;
			o = o->next;
			_free_sprite_stack(o->sprites);
			_destroy_object(tmp);
			continue;
		}
		prev = o;
		o = o->next;
	}
}
void add_collision_engine(engine_t * en, u32 mask_combo, void (* func)(object_t *, object_t *)){
	assert(en != NULL);
	assert(func != NULL);
	collision_t * evt = (collision_t *)malloc(sizeof(collision_t));
	evt->mask_combination = mask_combo;
	evt->func = func;
	evt->next = en->collision_events;
	en->collision_events = evt;
}
void add_sprite_object(object_t * obj, sprite_t * sprite){
	assert(obj != NULL);
	assert(sprite != NULL);
	sprite_stack_t * tmp = _alloc_sprite_stack();
	tmp->sprite = sprite;
	tmp->next = obj->sprites;
	obj->sprites = tmp;
}
void step_engine(engine_t * en){
	_paint_engine(en);
	_run_objects_engine(en);
}
/*
 * Drawing trecniques and effects
 * Don't use at the same time as game object rendering
 */
void draw_sprite_engine(engine_t * en, sprite_t * sprite, u16 left, u16 top){
	assert(en != NULL);
	assert(sprite != NULL);
	for(u16 i = 0; i < sprite->height; ++i)
		for(u16 j = 0; j < sprite->width; ++j){
			u32 rgb = sprite->data[i * sprite->width + j] == TRANSPARENT ?
			en->painted[(i + top) * en->width + j + left] :
			sprite->data[i * sprite->width + j];
			if(rgb != en->last_rgb)
				XSetForeground(en->dsp, en->gc, en->last_rgb = rgb);
			XDrawPoint(en->dsp, en->double_buffer, en->gc, j, i);
		}
	XCopyArea(en->dsp, en->double_buffer, en->win, en->gc, 0, 0, sprite->width, sprite->height, left, top);
	if(en->auto_commit)
		XFlush(en->dsp);
}
void fade_in_sprite_engine(engine_t * en, sprite_t * sprite, u16 left, u16 top, u8 opacity){
	assert(en != NULL);
	assert(sprite != NULL);
	assert(opacity <= 100);
	u8 mopa = 100 - opacity;
	for(u16 i = 0; i < sprite->height; ++i)
		for(u16 j = 0; j < sprite->width; ++j){
			u32 sp = sprite->data[i * sprite->width + j];
			u32 bg = en->painted[(i + top) * en->width + j + left];
			if(sp != TRANSPARENT){
				u8 red = ((((bg >> 16) & 0xff) * mopa) / 100 + (((sp >> 16) & 0xff) * opacity) / 100) & 0xff;
				u8 green = ((((bg >> 8) & 0xff) * mopa) / 100 + (((sp >> 8) & 0xff) * opacity) / 100) & 0xff;
				u8 blue = (((bg & 0xff) * mopa) / 100 + ((sp & 0xff) * opacity) / 100) & 0xff;
				bg = (red << 16) | (green << 8) | blue;
			}
			en->painted[(i + top) * en->width + j + left] = bg;
			if(en->last_rgb != bg)
				XSetForeground(en->dsp, en->gc, en->last_rgb = bg);
			XDrawPoint(en->dsp, en->double_buffer, en->gc, j, i);
		}
	XCopyArea(en->dsp, en->double_buffer, en->win, en->gc, 0, 0, sprite->width, sprite->height, left, top);
	if(en->auto_commit)
		XFlush(en->dsp);
}
void freetime_engine(engine_t * en){
	object_t * o;
	u16 to_clean;
	object_t * prev;
	object_t * tmp;
	assert(en != NULL);
	switch(en->paint_count % 16){
		case 0: // Dispose of objects flagged for disposal
			if(en->objects == NULL)
				break;
			o = en->objects;
			to_clean = 0;
			while(o != NULL){
				if(o->dispose_of)
					to_clean++;
				o = o->next;
			}
			if(!to_clean)
				break;
			// clear head
			while(en->objects != NULL && en->objects->dispose_of){
				en->objects = en->objects->next;
				to_clean--;
			}
			// clear body
			prev = en->objects;
			if(prev == NULL)
				return;
			o = prev->next;
			while(to_clean){
				if(o->dispose_of){
					--to_clean;
					prev->next = o->next;
					tmp = o;
					o = o->next;
					_free_sprite_stack(o->sprites);
					_destroy_object(tmp);
					continue;
				}
				prev = o;
				o = o->next;
			}
			break;
		case 8: // Set last painted color
			XSetForeground(en->dsp, en->gc, en->last_rgb = BACKGROUND);
			break;
	}
}