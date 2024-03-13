#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid1, pid2, pid3;

    // Create pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Fork first child (for 'cat W24.txt')
    if ((pid1 = fork()) == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) { // Child process
        // Close read end of pipe1 and write end of pipe2
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        // Redirect stdout to pipe1
        dup2(pipe1[1], STDOUT_FILENO);
        // Close unused pipes
        close(pipe1[1]);

        // Execute cat
        execlp("cat", "cat", "W24.txt", NULL);
        perror("Exec cat failed");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Fork second child (for 'grep 01234')
        if ((pid2 = fork()) == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) { // Child process
            // Close write end of pipe1 and read end of pipe2
            close(pipe1[1]);
            close(pipe2[0]);

            // Redirect stdin from pipe1 and stdout to pipe2
            dup2(pipe1[0], STDIN_FILENO);
            dup2(pipe2[1], STDOUT_FILENO);
            // Close unused pipes
            close(pipe1[0]);
            close(pipe2[1]);

            // Execute grep
            execlp("grep", "grep", "YZ", NULL);
            perror("Exec grep failed");
            exit(EXIT_FAILURE);
        } else { // Parent process
            // Fork third child (for 'wc')
            if ((pid3 = fork()) == -1) {
                perror("Fork failed");
                exit(EXIT_FAILURE);
            }

            if (pid3 == 0) { // Child process
                // Close write end of pipe2
                close(pipe2[1]);

                // Redirect stdin from pipe2
                dup2(pipe2[0], STDIN_FILENO);
                // Close unused pipes
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[0]);

                // Execute wc
                execlp("wc", "wc", NULL);
                perror("Exec wc failed");
                exit(EXIT_FAILURE);
            } else { // Parent process
                // Close unused pipes
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[0]);
                close(pipe2[1]);

                // Wait for all child processes to finish
                wait(NULL);
                wait(NULL);
                wait(NULL);
            }
        }
    }

    return 0;
}
