#include "Counter.h"

volatile int counter = 0;

void * inc_counter(void * args) {
    int c = (int)args;
    counter += c;
    JS_COUNTER_LOG("counter %i inc %i", counter, c);
    return NULL;
}

int get_counter(void) {
    JS_COUNTER_LOG("counter : %i", __FUNCTION__, counter);
    return counter;
}

void * reset_counter() {
	counter = 1;
	return NULL;
}

int get_scaled_counter(int args) {
	return counter/args;
}