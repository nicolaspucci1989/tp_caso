/* PThread Creation */ 

#include <stdio.h>
#include <pthread.h>

void *foo (void *arg) {		/* thread main */
	printf("Foobar!\n");
	return 0;
}

int main (void) {

	int i;
	pthread_t tid;
	
/*	pthread_attr_t attr;
	pthread_attr_init(&attr); // Required!!!
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&tid, &attr, foo, NULL); */
	pthread_create(&tid, NULL, foo, NULL);
	pthread_join(tid, NULL);

	return 0;
}


