#include "object.h"
#define POINTER_SIZE 4
/*
 * Default object functions
 */
static int _equals(object_t * this, object_t * that){
	return this == that;
}
static void _dispose(object_t * this){
	// TODO free fields and method(only holders)
	if(this->super != NULL)
		_dispose(this->super);
	this->super = NULL;
	free(this);
}
static object_t * _clone(object_t * this){
	// TODO
}
/*
 * Other functions
 */
static void * _obj_invoke(object_t * o, const char * name){
	assert(o != NULL);
	assert(name != NULL);
	holder_t * h = o->methods;
	while(h != NULL){
		if(strcmp(h->name, name) == 0)
			return h->data;
		h = h->next;
	}
	if(h == NULL && o->super != NULL)
		h = _obj_invoke(o->super, name);
	return h;
}
static holder_t * _make_holder(size_t size, char * name, holder_t * next){
	assert(size > 0);
	assert(name != NULL);
	int name_sz = strlen(name);
	assert(name_sz > 0 && name_sz < MAX_SLOT_NAME);
	holder_t * h = (holder_t *)malloc(sizeof(holder_t));
	h->data = malloc(size);
	h->size = size;
	strcpy(h->name, name);
	h->next = next;
	return h;
}
holder_t * make_method(char * name, void * func, holder_t * next){
	holder_t * h = _make_holder(POINTER_SIZE, name, next);
	h->data = func;
	if(next != NULL)
		assert(next->size == POINTER_SIZE);
	return h;
}
object_t * make_object(object_t * base, holder_t * fields, holder_t * methods){
	if(base == NULL){
		base = (object_t *)malloc(sizeof(object_t));
		base->super = NULL;
		base->fields = NULL;
		base->methods = make_method("equals", (void *)&_equals, make_method("dispose", (void *)&_dispose, make_method("clone", (void *)&_clone, NULL)));
	}
	object_t * o = (object_t *)malloc(sizeof(object_t));
	o->super = base;
	o->fields = fields;
	o->methods = methods;
	return o;
}
void * obj_invoke(object_t * this, const char * name){
	void * h = _obj_invoke(this, name);
	assert(h != NULL);
	return h;
}
static int _repeatable_holder_test(holder_t * h){
	holder_t * mh = h;
	while(mh != NULL){
		holder_t * th = mh->next;
		while(th != NULL){
			if(strcmp(th->name, mh->name) == 0)
				return 1;
			th = th->next;
		}
		mh = mh->next;
	}
	return 0;
}
int obj_verify(object_t * this){
	if(this == NULL)
		return 1;
	if(this->super != NULL && obj_verify(this->super))
		return 1;
	if(this->fields != NULL && _repeatable_holder_test(this->fields))
		return 1;
	if(this->methods != NULL && _repeatable_holder_test(this->methods))
		return 1;
	return 0;
}