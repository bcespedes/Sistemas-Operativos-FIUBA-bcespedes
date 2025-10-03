#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char key[FNAMESIZE];
		char value[FNAMESIZE];

		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, block_contains(eargv[i], '='));

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags &
	    O_CREAT) {  // Si O_CREAT está como una de los parametros de flags
		fd = open(file,
		          flags,
		          S_IRUSR | S_IWUSR);  // ARRIBA DICE QUE LO USEMOS
	} else {
		fd = open(file, flags);  // Abrir sin permisos
	}

	if (fd < 0) {
		perror("Error opening folder");
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:

		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		if (execvp(e->argv[0], e->argv)) {
			perror("Error executing command.");
			free_command(cmd);
			exit(-1);
		}
		break;

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;

		if (strlen(r->out_file) > 0) {
			int fd_out = open_redir_fd(r->out_file,
			                           O_WRONLY | O_CREAT |
			                                   O_TRUNC);  // ESCRIBIR
			if (fd_out < 0) {
				perror("Open out_file error\n");
				exit(-1);
			}
			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		if (strlen(r->in_file) > 0) {
			int fd_in = open_redir_fd(r->in_file, O_RDONLY);  // LEER
			if (fd_in < 0) {
				perror("Open in_file error\n");
				exit(-1);
			}
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		if (strlen(r->err_file) > 0) {
			if (r->err_file[0] == '&' && r->err_file[1] == '1') {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				int fd_err = open_redir_fd(r->err_file,
				                           O_WRONLY | O_CREAT |
				                                   O_TRUNC);
				if (fd_err < 0) {
					perror("Open err_file error\n");
				}
				dup2(fd_err, STDERR_FILENO);
				close(fd_err);
			}
		}

		r->type = EXEC;
		exec_cmd((struct cmd *) r);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		int fd[2];

		if (pipe(fd) < 0) {
			perror("No se pudo crear la tuberia\n");
			close(fd[READ]);
			close(fd[WRITE]);
			free_command(parsed_pipe);
			exit(-1);
		}

		pid_t pid_l = fork();

		if (pid_l < 0) {
			perror("No se pudo crear el fork para hijo "
			       "izquierdo\n");
			close(fd[READ]);
			close(fd[WRITE]);
			free_command(parsed_pipe);
			exit(-1);
		}

		if (pid_l == 0) {
			close(fd[READ]);
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[WRITE]);
			exec_cmd(p->leftcmd);
		}

		pid_t pid_r = fork();

		if (pid_r < 0) {
			perror("No se pudo crear el fork para hijo "
			       "derecho\n");
			close(fd[READ]);
			close(fd[WRITE]);
			free_command(parsed_pipe);
			exit(-1);
		}

		if (pid_r == 0) {
			close(fd[WRITE]);
			dup2(fd[READ], STDIN_FILENO);
			close(fd[READ]);
			exec_cmd(p->rightcmd);
		}

		close(fd[READ]);
		close(fd[WRITE]);

		waitpid(pid_l, NULL, 0);
		waitpid(pid_r, NULL, 0);

		free_command(parsed_pipe);
		exit(0);

		break;
	}
	}
}
