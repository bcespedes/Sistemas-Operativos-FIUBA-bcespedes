#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"


char prompt[PRMTLEN] = { 0 };


// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}


static void
sigchld_handler()
{
	pid_t pid;
	int status;

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
#ifndef SHELL_NO_INTERACTIVE
		if (isatty(1))
			fprintf_debug(
			        stdout,
			        "%s	Program terminated [PID: %d]%s\n",
			        COLOR_BLUE,
			        pid,
			        COLOR_RESET);
#endif
	}
}


static void
set_sigchld_handlers()
{
	struct sigaction sa;

	sa.sa_handler = &sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("Error in sigaction.");
		exit(-1);
	}
}


// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}


int
main(void)
{
	set_sigchld_handlers();

	init_shell();

	run_shell();

	return 0;
}
