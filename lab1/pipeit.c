#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE (1024)

/*
    Make a 2 pipe
    fork 2 child 
    child 1
        reroute stdout to pipe 1
        run ls
    child 2
        reroute stdin to pipe 1
        reroute stdout to pipe 2
        run sort
    wait for children
    write everything from pipe 2 to a file
    return proper exit code
    TODO: error checking
*/

int main(void) {
    int pipe1[2], pipe2[2];
    pid_t child_ls, child_sort;

    if (pipe(pipe1) == -1) {
        perror("pipe1 failed");
        return EXIT_FAILURE;
    }

    if (pipe(pipe2) == -1) {
        perror("pipe2 failed");
        return EXIT_FAILURE;
    }

    child_ls = fork();
    if (child_ls == -1) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (child_ls == 0) {
        fprintf(stderr, "ls Child Process pid : %d\n",getpid());
        close(pipe2[0]); // no need for pipe2
        close(pipe2[1]);
        
        close(pipe1[0]); //  not going to read
        dup2(pipe1[1], STDOUT_FILENO); // stdout into pipe
        close(pipe1[1]); // close after dup
        execl("/bin/ls", "ls", NULL);
        return EXIT_FAILURE;
    }
    else {
        child_sort = fork();
        if (child_sort == -1) {
            perror("fork failed");
            return EXIT_FAILURE;
        }

        if (child_sort == 0) {
            fprintf(stderr, "sort Child Process pid : %d\n", getpid());
            close(pipe1[1]); // close write end for pipe1
            dup2(pipe1[0], STDIN_FILENO);
            close(pipe1[0]); // Close read end after duplication

            close(pipe2[0]); // no read
            dup2(pipe2[1], STDOUT_FILENO);
            execl("/usr/bin/sort", "sort", "-r", NULL);
            return EXIT_FAILURE;
        }
        else {
            close(pipe1[0]); // Parent should close both ends
            close(pipe1[1]);

            close(pipe2[1]);

            int outfile_fd = open("outfile", O_CREAT | O_WRONLY, 0644);
            if (outfile_fd == -1) {
                perror("could make fd for outfile");
                return EXIT_FAILURE;
            }

            ssize_t nbytes;
            char c;
            while ((nbytes = read(pipe2[0], &c, 1)) > 0) {
                write(outfile_fd, &c, 1);
            }

            close(pipe2[0]);
            close(outfile_fd);

            int status;
            waitpid(child_ls, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                fprintf(stderr, "ls child exited successfully\n");
            }

            waitpid(child_sort, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                fprintf(stderr, "sort child exited successfully\n");
            }
            
            close(outfile_fd);
        }
        
    }
    
    fprintf(stderr, "DONE WITH EVERYTHING\n");
    fprintf(stdout, "DONE WITH EVERYTHING\n");

    return EXIT_SUCCESS;
}