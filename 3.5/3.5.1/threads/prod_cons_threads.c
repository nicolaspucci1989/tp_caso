#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 3		/* Buffer compartido */

int buffer[BUF_SIZE];  	/* Buffer compartido */
int add = 0;  			/* Lugar donde agregar el proximo elemento */
int rem = 0;  			/* Lugar dondne remover el proximo elemento */
int num = 0;  			/* number elements in buffer */

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock para buffer */
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* consumidor espera en esta condicion */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* productor espera en esta condicion */

void *productor (void *param);
void *consumidor (void *param);

int main(int argc, char *argv[]) {

	pthread_t tid1, tid2;  /* thread identifiers */
	int i;

	/* create the threads; may be any number, in general */
	if(pthread_create(&tid1, NULL, productor, NULL) != 0) {
		fprintf(stderr, "No se pudo crear el thread productor\n");
		exit(1);
	}

	if(pthread_create(&tid2, NULL, consumidor, NULL) != 0) {
		fprintf(stderr, "No se pudo crear el thread consumidor\n");
		exit(1);
	}

	/* esperar a que terminen los threads creados */
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf("Padre terminando\n");

	return 0;
}

void *productor(void *param) {

	int i;
	for (i=1; i<=20; i++) {
		
		/* Insertar en el buffer */
		pthread_mutex_lock (&m);	
			if (num > BUF_SIZE) {
				exit(1);  /* overflow */
			}

			while (num == BUF_SIZE) {  /* bloquear si el buffer esta lleno */
				pthread_cond_wait (&c_prod, &m);
			}
			
			/* si el buffer no esta lleno agregar un elemento */
			buffer[add] = i;
			add = (add+1) % BUF_SIZE;
			num++;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_cons);
		printf ("productor: inserto %d\n", i);
		fflush (stdout);
	}

	printf("productor terminando\n");
	fflush(stdout);
	return 0;
}

/* el consumidor nunca termina */
void *consumidor(void *param) {

	int i;

	while(1) {

		pthread_mutex_lock (&m);
			if (num < 0) {
				exit(1);
			} /* underflow */

			while (num == 0) {  /* bloquear si el buffer esta vacio */
				pthread_cond_wait (&c_cons, &m);
			}

			/* si el buffer no esta vacion, remover un elemento */
			i = buffer[rem];
			rem = (rem+1) % BUF_SIZE;
			num--;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_prod);
		printf ("Valor consumido %d\n", i);  fflush(stdout);
	}
	return 0;
}
