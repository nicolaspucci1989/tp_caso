#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LEER			0
#define ESCRIBIR	1	
#define TAMANIO		64	


int main()
{
	int descr[2];
	int bytesleidos;
	char mensaje[TAMANIO], *frase = "Un mensaje enviado.";	

	/* Crear el pipe */
	pipe(descr);
	if (fork() == 0) {
		/* Hijo 1 escribe */
		dup2(descr[ESCRIBIR], ESCRIBIR);
		close (descr[LEER]);
		write (descr[ESCRIBIR], frase, strlen(frase));
	} else {
		if (fork() == 0) {
			/* Hijo 2 lee */
			dup2(descr[LEER], LEER);
			close (descr[ESCRIBIR]);	
			bytesleidos = read (descr[LEER], mensaje, TAMANIO);
			mensaje[bytesleidos]='\0';
			printf ("Bytes leidos: %d\n", bytesleidos);
			printf ("Mensaje enviado por el Hijo: %s\n", mensaje);
		} else{
			/* Padre cierra descriptore y espera a los hijo */
			close (descr[LEER]);
			close (descr[ESCRIBIR]);
 			wait(0);		
 			wait(0);		
			return(0);
		} 
	} 
}
