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
    return 1;
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
    return 1;
}

int nrsh_exit(char **args) {
    return 0;
}
int nrsh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("nrsh: fork failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("nrsh: fork failed");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
int nrsh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
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

void nrsh_loop(void) {

    char *line;
    char **args;
    int status;

    do {
        printf("$ ");
        line = nrsh_read_line();
        args = nrsh_split_line(line);
        status = nrsh_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int agrc, char **argv) {

    nrsh_loop();

    return EXIT_SUCCESS;
}
