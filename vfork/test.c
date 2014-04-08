#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * redis-cli del foo; gcc -o test test.c && ./test redis-cli set foo bar;echo $?;redis-cli get foo;rm -rf test
 */

int main(int argc, char *argv[])
{
    int r, child_status;
    pid_t child_pid = vfork();

    char *prog_argv[5];

    prog_argv[0] = "/usr/bin/redis-cli";
    prog_argv[1] = "set";
    prog_argv[2] = "foo";
    prog_argv[3] = "bar";
    prog_argv[4] = NULL;

    if (child_pid == 0) {
        execvp(prog_argv[0], prog_argv);
        printf("Unknown command\n");
        _exit(1);
    } else {
        child_pid = waitpid(-1, &child_status, WUNTRACED);
    }

    if (WIFEXITED(child_status) || WIFSIGNALED(child_status))
        _exit(EXIT_SUCCESS);

    _exit(1);
}
