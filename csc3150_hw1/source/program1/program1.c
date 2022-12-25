#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	/* fork a child process */

	/* execute test program */

	/* wait for child process terminates */

	/* check child process'  termination status */
	pid_t pid;
	int status;
	printf("Process start to fork\n");
	pid = fork();
	if (pid < 0) {
		printf("Fork error!\n");
	} else if (pid == 0) {
		//
		char *arg[argc];
		for (int i = 0; i < argc - 1; i++)
			arg[i] = argv[i + 1];
		arg[argc - 1] = NULL;
		printf("I'm the Child Process, my pid = %d\n", getpid());
		printf("I'm the Parent Process, my pid = %d\n", getppid());

		printf("Child process start to execute test program:\n");

		execve(arg[0], arg, NULL);
		printf("end\n");
	} else {
		waitpid(pid, &status, WUNTRACED);

		if (WIFEXITED(status)) {
			printf("Parent receive SIGCHLD signal\n");
			printf("Normal termination with EXIT STATUS = %d\n",
			       WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			int sig_bit = WTERMSIG(status);
			char *info, *SIG;
			switch (sig_bit) {
			case 6:
				SIG = "SIGABRT", info = "aborted";
				break;
			case 14:
				SIG = "SIGALRM",
				info = "terminated by alarm signal";
				break;
			case 7:
				SIG = "SIGBUS", info = "bus error";
				break;
			case 8:
				SIG = "SIGFPE",
				info = "floating point exception";
				break;
			case 1:
				SIG = "SIGHUP", info = "hang up";
				break;
			case 4:
				SIG = "SIGILL", info = "illegal instruction";
				break;
			case 2:
				SIG = "SIGINT", info = "interupted";
				break;
			case 9:
				SIG = "SIGKILL", info = "killed";
				break;
			case 13:
				SIG = "SIGPIPE", info = "fail to write to pipe";
				break;
			case 3:
				SIG = "SIGQUIT", info = "quited";
				break;
			case 11:
				SIG = "SIGSEGV", info = "segmentation fault ";
				break;
			case 15:
				SIG = "SIGTERM",
				info = "terminated by termination signal";
				break;
			case 5:
				SIG = "SIGTRAP",
				info = "terminated by trap signal";
				break;
			}
			printf("Parent receive %s signal\n", SIG);

			printf("CHILD EXECUTION FAILED: %d (%s)\n", sig_bit,
			       info);
		} else if (WIFSTOPPED(status)) {
			printf("Parent receive SIGSTOP signal\n");
			printf("CHILD PROCESS STOPPED: %d\n", WSTOPSIG(status));
		} else {
			printf("CHILD PROCESS CONTINUED\n");
		}
		exit(0);
	}
	return 0;
}
