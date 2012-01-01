#include "object.h"
#include "stdio.h"
/*
 * Example
 *
 * Lacking set/get fields
 */
static int some_method(int a, int b){
	return a + b;
}
int main(int argc, char * argv[]){
	holder_t * new_methods = make_method("some_method", &some_method, NULL);
	object_t * extended_class = make_object(DEFAULT_OBJECT, INHERITED, new_methods);
/*
 * Still need an assembly function that bypassed parameter testing, dynamically
 * loading the method would be nice as well.
 */
	int (* func)(int, int) = obj_invoke(extended_class, "some_method");
	assert(func(1, 2) == 1 + 2);
	printf("%d\n", obj_verify(extended_class));
	assert(obj_verify(extended_class) == 0);
	return 0;
}