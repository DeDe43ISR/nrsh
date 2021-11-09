#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int nrsh_cd(char **args);
int nrsh_help(char **args);
int nrsh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};  

int (*builtin_func[]) (char**) = {
    &nrsh_cd,
    &nrsh_help,
    &nrsh_exit
};

int nrsh_builtin_sum() {
    return sizeof(builtin_str) / sizeof(char *);
}

int nrsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "nrsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("nrsh: cd error");
        }
    }
    return 0;
}

int nrsh_help(char **args) {
    int i;

    printf("DeDe Narco Shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < nrsh_builtin_sum(); i++) {
        printf(" %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other programs.\n");
    return 0;
}

int nrsh_exit(char **args) {
    exit(0);
}
int nrsh_launch(char **args) {
    pid_t pid;
    int status, exit_status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("nrsh: fork failed 1");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("nrsh: fork failed 2");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
        return exit_status;
    }
    return 0;
}

int nrsh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 0;
    }

    for (i = 0; i < nrsh_builtin_sum(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return nrsh_launch(args);
}

#define NRSH_RL_BUFSIZE 1024

char *nrsh_read_line(void) {
    int bufsize = NRSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "nrsh: line buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if ( c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += NRSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "nrsh: line buffer allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define NRSH_TOK_BUFSIZE 64
#define NRSH_TOK_DELIM " \t\r\n\a"
char **nrsh_split_line(char *line) {
    
    int bufsize = NRSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "nrsh: tokens buffer allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, NRSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += NRSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "nrsh: tokens buffer allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, NRSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

#define NRSH_COM_BUFSIZE 1024
int nrsh_run(char **args) {

    int bufsize = NRSH_COM_BUFSIZE;
    char **command = malloc(bufsize * sizeof(char*));
    int args_size = 0;
    int place = 0, command_counter = 0;
    int last = 0, run = 0;
    int status = -1;

    if (!command) {
        fprintf(stderr, "nrsh: tokens buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (args_size = 0; args[args_size] != NULL; args_size++);

    for (int counter = 0; counter <= args_size; counter++) {
        if (args[counter] == NULL) {
            command[command_counter] = NULL;
            if ((last == 1 && (status == 0 || status == -1)) || last == 2 && (status == 1 || status == -1) || last == 0) {
                status = nrsh_execute(command);
            }
            if ((last == 1 && (status == 1 || status == -1)) || last == 2 && (status == 0 || status == -1)) {
                continue;
            }
            return status;
        } else if (strcmp(args[counter], "&&") == 0) {
            command[command_counter] = NULL;
            if ((last == 1 && (status == 0 || status == -1)) || last == 2 && (status == 1 || status == -1)) {
                status = nrsh_execute(command);
                command_counter = 0;
                last = 0;
            }
            if ((last == 1 && (status == 1 || status == -1)) || last == 2 && (status == 0 || status == -1)) {
                command_counter = 0;
                last = 0;
                status = 0;
            }
            last = 1;
        } else if (strcmp(args[counter], "||") == 0) {
            command[command_counter] = NULL;
            if ((last == 1 && (status == 0 || status == -1)) || last == 2 && (status == 1 || status == -1)) {
                status = nrsh_execute(command);
                command_counter = 0;
                last = 0;
            }
            if ((last == 1 && (status == 1 || status == -1)) || last == 2 && (status == 0 || status == -1)) {
                command_counter = 0;
                last = 0;
                status = 0;
            }
            last = 2;
        } else if (strcmp(args[counter], ";") == 0) {
            command[command_counter] = NULL;
            status = nrsh_execute(command);
            command_counter = 0;
        } else {
            command[command_counter] = args[counter];
            command_counter++;
        }
    }
    free(command);

}

void nrsh_loop(void) {

    char *line;
    char **args;
    int status;

    while (1)  {
        printf("$ ");
        line = nrsh_read_line();
        args = nrsh_split_line(line);
        status = nrsh_run(args);

        free(line);
        free(args);
    }
}

int main(int agrc, char **argv) {

    nrsh_loop();

    return EXIT_SUCCESS;
}
