/*
 * Custom Shell - Stage 1
 * Author: Crown Okusaga
 * Student ID: 23392331
 * Acknowledgement: This work complies with the DCU Academic Integrity Policy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

void execute_command(char **args);
void internal_command(char **args);
void print_prompt();
void batch_mode(char *filename);

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];

    // Set shell environment variable
    char path[1024];
    if (readlink("/proc/self/exe", path, sizeof(path) - 1) != -1) {
        setenv("shell", path, 1);
    }

    printf("Custom shell started. Type 'quit' to exit.\n");

    // Batch mode execution
    if (argc == 2) {
        batch_mode(argv[1]);
        return 0;
    }

    while (1) {
        print_prompt();

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            continue; // Handle Ctrl+D (EOF)
        }

        input[strcspn(input, "\n")] = 0; // Remove newline

        // Tokenize input
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue; // Ignore empty input

        internal_command(args);
    }
    return 0;
}

// Function to print shell prompt
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
    } else {
        perror("getcwd");
    }
}

// Function to handle internal commands
void internal_command(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            printf("%s\n", getenv("PWD"));
        } else if (chdir(args[1]) != 0) {
            perror("cd error");
        } else {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("PWD", cwd, 1);  // Correct memory handling
            }
        }
    } else if (strcmp(args[0], "clr") == 0) {
        printf("\033[H\033[J"); // Clear screen ANSI escape code
    } else if (strcmp(args[0], "dir") == 0) {
        DIR *d;
        struct dirent *dir;
        d = opendir(args[1] ? args[1] : ".");
        if (!d) {
            perror("dir error");
            return;
        }
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    } else if (strcmp(args[0], "environ") == 0) {
        extern char **environ;
        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            if (args[i][0] == '$') {  // Check if argument starts with '$'
                char *env_var = getenv(args[i] + 1);  // Get env var (skip '$')
                if (env_var) {
                    printf("%s ", env_var);
                } else {
                    printf(" ");  // Print space if variable doesn't exist
                }
            } else {
                printf("%s ", args[i]);  // Print normal text
            }
        }
        printf("\n"); // Only one newline at the end
    } else if (strcmp(args[0], "help") == 0) {
        system("more help.txt");
    } else if (strcmp(args[0], "pause") == 0) {
        printf("Press Enter to continue...");
        getchar();
    } else if (strcmp(args[0], "quit") == 0) {
        exit(0);
    } else {
        execute_command(args);
    }
}

// Function to execute external commands
void execute_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("exec failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed");
    } else {
        wait(NULL); // Wait for child to finish
    }
}

// Function to process batch file
void batch_mode(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Batch file error");
        return;
    }
    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *args[MAX_ARG_SIZE];
        int i = 0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        if (args[0] != NULL) {
            internal_command(args);
        }
    }
    fclose(file);
}
