#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>


#define EXIT_ERROR -1
#define MAX_PATH 256
#define MAX_COMM 256
#define PROC_PATH "/proc"


int
es_numero(const char *str)
{
	while (*str) {
		if (!isdigit(*str))
			return 0;
		str++;
	}
	return 1;
}


void
validar_cantidad_argumentos(int cantidad_argumentos)
{
	if (cantidad_argumentos != 1) {
		perror("No se esperan argumentos para este comando.");
		exit(EXIT_ERROR);
	}
}


void
leer_comando(int pid)
{
	char ruta[MAX_PATH];
	char comando[MAX_COMM];
	int fd;
	ssize_t bytes_leidos;

	snprintf(ruta, sizeof(ruta), "/proc/%d/comm", pid);

	fd = open(ruta, O_RDONLY);
	if (fd < 0) {
		perror("Error al hacer open.");
		exit(EXIT_ERROR);
	}

	bytes_leidos = read(fd, comando, MAX_COMM - 1);
	if (bytes_leidos == -1) {
		perror("Error al leer.");
		strcpy(comando, "NULL");
		close(fd);
		exit(EXIT_ERROR);
	}

	comando[bytes_leidos] = '\0';
	if (comando[bytes_leidos - 1] == '\n')
		comando[bytes_leidos - 1] = '\0';

	printf("    %d %s\n", pid, comando);

	close(fd);
}


void
recorrer_procesos()
{
	DIR *directorio_proceso;
	struct dirent *entrada;

	directorio_proceso = opendir(PROC_PATH);
	if (directorio_proceso == NULL) {
		perror("Error al abrir directorio de proceso");
		exit(EXIT_FAILURE);
	}

	printf("    PID COMMAND\n");

	while ((entrada = readdir(directorio_proceso)) != NULL) {
		if (es_numero(entrada->d_name)) {
			int pid = atoi(entrada->d_name);
			leer_comando(pid);
		}
	}

	closedir(directorio_proceso);
}


int
main(int argc, char *argv[])
{
	argv[1] = NULL;  // Puse esto para evitar el warning al compilar ya que no se usaba el argv.
	validar_cantidad_argumentos(argc);
	recorrer_procesos();
	return 0;
}