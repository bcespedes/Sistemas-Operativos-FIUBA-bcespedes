#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


#define EXIT_ERROR -1
#ifndef NARGS
#define NARGS 4
#endif


pid_t
hacer_fork()
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("Error al hacer fork.");
		exit(EXIT_ERROR);
	}

	return pid;
}


void
leer_comandos(char **comandos_recibidos)
{
	pid_t pid = hacer_fork();

	if (pid == 0)

		// Caso hijo
		execvp(comandos_recibidos[0], comandos_recibidos);
	else

		// Caso padre
		wait(NULL);
}


void
liberar_memoria(char **comandos, int i)
{
	for (int j = 1; j <= i; j++) {
		free(comandos[j]);
		comandos[j] = NULL;
	}
}


void
validar_cantidad_argumentos(int cantidad_argumentos)
{
	if (cantidad_argumentos != 2) {
		perror("Se necesita solamente 1 argumento.");
		exit(EXIT_ERROR);
	}
}


int
verificar_numero_argumentos_alcanzado(char **comandos, int iterador)
{
	if (iterador == NARGS) {
		leer_comandos(comandos);
		liberar_memoria(comandos, iterador);
		iterador = 0;
	}

	iterador++;

	return iterador;
}


int
main(int argc, char *argv[])
{
	validar_cantidad_argumentos(argc);

	char *linea = NULL;
	size_t longitud = 0;
	ssize_t cantidad_caracteres;
	char *comandos[NARGS + 2] = { NULL };
	comandos[0] = argv[1];

	int iterador = 1;

	while ((cantidad_caracteres = getline(&linea, &longitud, stdin)) != EOF) {
		linea[cantidad_caracteres - 1] = '\0';
		comandos[iterador] = strdup(linea);
		iterador = verificar_numero_argumentos_alcanzado(comandos,
		                                                 iterador);
	}

	leer_comandos(comandos);
	liberar_memoria(comandos, iterador);
	free(linea);

	return 0;
}
