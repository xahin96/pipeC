#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    // Initializing pipe 1 and 2 fb arrays
    int pipe1[2], pipe2[2], wc_pipe[2];
    // variables for forks
    pid_t pid1, pid2, pid3;

    // Creating both of the pipes and handling error
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Forking for the first child which will run the
    // 'ls -1 -t' command and write it to the pipe. Also handling error
    if ((pid1 = fork()) == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    // Handling the child process
    if (pid1 == 0) {
        // Closing the read part of pipe1 and whole pipe2 which won't be used
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        // Redirecting stdout
        dup2(pipe1[1], STDOUT_FILENO);
        // Done working with pipe
        close(pipe1[1]);

        // Executing the command
        execlp("ls", "ls", "-1", "-t", NULL);
        perror("Exec ls failed");
        exit(EXIT_FAILURE);
    }
        // Handling the parent process
    else {
        // Forking the second child for performing 'grep .c' while reading
        // from pipe 1 and will write the grep output to pipe 2. Also handling error
        if ((pid2 = fork()) == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        // Handling child process
        if (pid2 == 0) {
            // Closing the write end of pipe1 because we will only read from ls
            // and read end of pipe2 because we will write the output of grep
            // to pipe2 for further use
            close(pipe1[1]);
            close(pipe2[0]);

            // Redirecting stdin from pipe1 and  setting stdout to pipe2 for writing
            dup2(pipe1[0], STDIN_FILENO);
            dup2(pipe2[1], STDOUT_FILENO);
            // Closing the unused pipes
            close(pipe1[0]);
            close(pipe2[1]);

            // Executing the grep .c command to find all the files that
            // has the .c extension
            execlp("grep", "grep", ".c", NULL);
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
                dup2(wc_pipe[1], STDOUT_FILENO);
                close(wc_pipe[1]);
                close(wc_pipe[0]);
                execlp("wc", "wc", NULL);
                perror("Exec wc failed");
                exit(EXIT_FAILURE);
            } else { // Parent process
                // Close unused pipes
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[0]);
                close(pipe2[1]);
                close(wc_pipe[1]); // Close write end of wc pipe

                // Open output.txt for writing
                int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd == -1) {
                    perror("Failed to open output.txt");
                    exit(EXIT_FAILURE);
                }

                // Redirect stdin from wc pipe and stdout to output.txt
                dup2(wc_pipe[0], STDIN_FILENO);
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
                close(wc_pipe[0]);

                // Read from wc and write to output.txt
                char buffer[4096];
                ssize_t nread;
                while ((nread = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
                    if (write(STDOUT_FILENO, buffer, nread) != nread) {
                        perror("Write error to output.txt");
                        exit(EXIT_FAILURE);
                    }
                }

                // Wait for all child processes to finish
                wait(NULL);
                wait(NULL);
                wait(NULL);
            }
        }
    }

    return 0;
}
