#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

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
    int pipe_ls_sort[2], pipe_sort_file[2];
    pid_t child_ls, child_sort;

    if (pipe(pipe_ls_sort) == -1) {
        perror("pipe_ls_sort failed");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe_sort_file) == -1) {
        perror("pipe_sort_file failed");
        exit(EXIT_FAILURE);
    }

    child_ls = fork();
    if (child_ls == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (child_ls == 0) {
        fprintf(stderr, "ls Child Process pid : %d\n",getpid());
        close(pipe_sort_file[0]); // no need for pipe_sort_file
        close(pipe_sort_file[1]);
        
        close(pipe_ls_sort[0]); //  not going to read
        dup2(pipe_ls_sort[1], STDOUT_FILENO); // stdout into pipe
        close(pipe_ls_sort[1]); // close after dup
        execl("/bin/ls", "ls", NULL);
        exit(EXIT_FAILURE); 
    }


    child_sort = fork();
    if (child_sort == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (child_sort == 0) {
        fprintf(stderr, "sort Child Process pid : %d\n", getpid());
        close(pipe_ls_sort[1]); // close write end for pipe_ls_sort
        dup2(pipe_ls_sort[0], STDIN_FILENO);
        close(pipe_ls_sort[0]); // Close read end after duplication

        close(pipe_sort_file[0]); // no read
        dup2(pipe_sort_file[1], STDOUT_FILENO);
        close(pipe_sort_file[0]);
        execl("/usr/bin/sort", "sort", "-r", NULL);
        exit(EXIT_FAILURE);
    }

    close(pipe_ls_sort[0]); 
    close(pipe_ls_sort[1]);

    close(pipe_sort_file[1]);

    int outfile_fd = open("outfile", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (outfile_fd == -1) {
        perror("could make fd for outfile");
        exit(EXIT_FAILURE);
    }

    ssize_t nbytes;
    char c;
    while ((nbytes = read(pipe_sort_file[0], &c, 1)) > 0) {
        write(outfile_fd, &c, 1);
    }

    close(pipe_sort_file[0]);
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
    
    fprintf(stderr, "DONE WITH EVERYTHING\n");

    return EXIT_SUCCESS;
}
