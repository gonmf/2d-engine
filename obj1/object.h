#include "stdlib.h"
#include "string.h"
#include "assert.h"
#define MAX_SLOT_NAME 32
#define DEFAULT_OBJECT NULL
#define INHERITED NULL
typedef struct __holder_{
	void * data;
	short int size;
	char name[MAX_SLOT_NAME];
	struct __holder_ * next;
} holder_t;
typedef struct __object_{
	struct __object_ * super;
	holder_t * fields;
	holder_t * methods;
} object_t;
holder_t * make_method(char * name, void * func, holder_t * next);
object_t * make_object(object_t * base, holder_t * fields, holder_t * methods);
void * obj_invoke(object_t * this, const char * name);
int obj_verify(object_t * this);