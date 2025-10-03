#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


#define EXIT_ERROR -1
#define READ 0
#define WRITE 1
#define END_OF_FILE 0


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
crear_pipe(int vector_pipe[2])
{
	if (pipe(vector_pipe) < 0) {
		perror("Error al crear pipe.");
		exit(EXIT_ERROR);
	}
}


void
escribir(int pipe_in, int num)
{
	ssize_t bytes = write(pipe_in, &num, sizeof(num));
	if (bytes == -1) {
		perror("Error al escribir.");
		exit(EXIT_ERROR);
	}
}


void
generar_numeros(int pipe_in, int numero_limite)
{
	for (int numero_generado = 2; numero_generado <= numero_limite;
	     numero_generado++)
		escribir(pipe_in, numero_generado);
}


void
filtrar_numeros_primos(int arriba_in, int pipe_in, int numero_primo)
{
	int numero;

	while (read(arriba_in, &numero, sizeof(numero)) > END_OF_FILE)
		if (numero % numero_primo != 0)
			escribir(pipe_in, numero);
}


void
validar_cantidad_argumentos(int cantidad_argumentos)
{
	if (cantidad_argumentos != 2) {
		perror("Se necesita solamente 1 argumento.");
		exit(EXIT_ERROR);
	}
}


void
filtro(int arriba_in)
{
	int numero_primo;

	if (read(arriba_in, &numero_primo, sizeof(numero_primo)) == END_OF_FILE) {
		close(arriba_in);
		return;
	}

	printf("primo %d\n", numero_primo);

	int pipe_filtro[2];
	crear_pipe(pipe_filtro);

	pid_t proximo_filtro = hacer_fork();
	if (proximo_filtro == 0) {
		// Caso hijo
		close(pipe_filtro[WRITE]);
		close(arriba_in);
		filtro(pipe_filtro[READ]);
		close(pipe_filtro[READ]);
		exit(EXIT_SUCCESS);
	} else {
		// Caso padre
		close(pipe_filtro[READ]);
		filtrar_numeros_primos(arriba_in, pipe_filtro[WRITE], numero_primo);
		close(arriba_in);
		close(pipe_filtro[WRITE]);
		wait(NULL);
		exit(EXIT_SUCCESS);
	}
}


int
main(int argc, char *argv[])
{
	validar_cantidad_argumentos(argc);

	int numero_limite = atoi(argv[1]);

	int pipe_inicial[2];
	crear_pipe(pipe_inicial);

	pid_t proceso_inicial = hacer_fork();
	if (proceso_inicial == 0) {
		// Caso hijo
		close(pipe_inicial[READ]);
		generar_numeros(pipe_inicial[WRITE], numero_limite);
		close(pipe_inicial[WRITE]);
		exit(EXIT_SUCCESS);
	} else {
		// Caso padre
		close(pipe_inicial[WRITE]);
		filtro(pipe_inicial[READ]);
		close(pipe_inicial[READ]);
		wait(NULL);
		exit(EXIT_SUCCESS);
	}

	return 0;
}
