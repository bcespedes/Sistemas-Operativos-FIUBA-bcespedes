#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>


#define EXIT_ERROR -1
#define TIMER_SIGNAL (SIGRTMIN + 1)


void
validar_cantidad_argumentos(int argc)
{
	if (argc < 3) {
		perror("Se necesitan al menos 2 argumentos.");
		exit(EXIT_ERROR);
	}
}


void
matar_proceso(int *pid)
{
	if (pid == NULL)
		return;

	if (kill(*pid, SIGTERM) == -1) {
		perror("Error al terminar el proceso.");
		exit(EXIT_ERROR);
	}

	printf("Comando finalizado por exceder el timeout.\n");
	wait(NULL);
}


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
manejador_de_signals(int signal, siginfo_t *signal_info, void *contexto)
{
	(void) signal;
	(void) contexto;
	matar_proceso((int *) (signal_info->si_value.sival_ptr));
}


timer_t
crear_timer(int *pid_comando)
{
	struct sigaction signal_action;

	signal_action.sa_flags = SA_SIGINFO;
	signal_action.sa_sigaction = manejador_de_signals;
	sigemptyset(&signal_action.sa_mask);

	if (sigaction(TIMER_SIGNAL, &signal_action, NULL) == -1) {
		perror("Error al configurar el sighandler.");
		exit(EXIT_ERROR);
	}

	timer_t id_timer;
	struct sigevent signal_event;

	signal_event.sigev_notify = SIGEV_SIGNAL;
	signal_event.sigev_signo = TIMER_SIGNAL;
	signal_event.sigev_value.sival_ptr = pid_comando;

	if (timer_create(CLOCK_MONOTONIC, &signal_event, &id_timer) == -1) {
		perror("Error al crear el timer.");
		exit(EXIT_ERROR);
	}

	return id_timer;
}


void
configurar_timer(int duracion, timer_t *id_timer)
{
	struct itimerspec timer_specs;

	timer_specs.it_value.tv_sec = duracion;
	timer_specs.it_value.tv_nsec = 0;
	timer_specs.it_interval.tv_sec = 0;
	timer_specs.it_interval.tv_nsec = 0;

	if (timer_settime(*id_timer, 0, &timer_specs, NULL) == -1) {
		perror("Error al configurar el timer.");
		exit(EXIT_ERROR);
	}
}


void
ejecutar_comando_con_timeout(char *comando, char **argumentos, int duracion)
{
	int pid = hacer_fork();

	if (pid == 0) {
		// Caso hijo
		int ejecucion = execvp(comando, argumentos);
		if (ejecucion < 0) {
			perror("Error al ejecutar el comando.");
			exit(EXIT_ERROR);
		}
	} else {
		// Caso padre
		timer_t timerid = crear_timer(&pid);
		configurar_timer(duracion, &timerid);
		wait(NULL);
		if (timer_delete(timerid) == -1) {
			perror("Error al eliminar el timer.");
			exit(EXIT_ERROR);
		}
	}
}


int
main(int argc, char *argv[])
{
	validar_cantidad_argumentos(argc);

	int duracion = atoi(argv[1]);
	if (duracion <= 0) {
		perror("La duracion debe ser positiva.");
		exit(EXIT_ERROR);
	}

	ejecutar_comando_con_timeout(argv[2], &argv[2], duracion);

	return 0;
}