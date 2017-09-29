#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

int ash_cd(char** args);
int ash_help(char** args);
int ash_exit(char** args);

char* builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char**) = {
    &ash_cd,
    &ash_help,
    &ash_exit
};

int ash_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

int ash_cd(char** args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "ash: expected argument to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("ash");
        }
    }
    return 1;
}

int ash_help(char** args)
{
    int i;
    printf("Ankit Goel's ASH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are builtin:\n");
    for(i = 0; i < ash_num_builtins(); ++i) {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

int ash_exit(char** args)
{
    return 0;
}

int ash_launch(char** args)
{
    pid_t pid;
    int status;

    pid = fork();

    if(pid == 0) {
        //Child process
        if (execvp(args[0], args) == -1) {
            perror("ash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error in forking
        perror("ash");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    }

    return 1;
}

int ash_execute(char** args)
{
    int i;
    if (args[0] == NULL) return 1;

    for(i = 0; i < ash_num_builtins(); ++i) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return ash_launch(args);
}

char* ash_read_line(void)
{
    char* line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}


#define ASH_TOK_BUFSIZE 64
#define ASH_TOK_DELIM " \t\r\n\a"
char** ash_split_line(char* line)
{
    const char* const delim = ASH_TOK_DELIM;
    size_t bufsize = ASH_TOK_BUFSIZE;
    int position = 0;
    char* token;
    char** tokens = malloc(sizeof(char*) * bufsize);

    if (!tokens) {
        fprintf(stderr, "ash: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, delim);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ASH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize);

            if (!tokens) {
                fprintf(stderr, "ash: allocation error.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, delim);
    }
    tokens[position] = NULL;

    return tokens;
}

void ash_loop(void)
{
    char* line;
    char** args;
    int status=0;

    do {
        printf("> ");
        line = ash_read_line();
        args = ash_split_line(line);
        status = ash_execute(args);
        free(line);
        free(args);
    } while (status);
}

int main()
{
    signal(SIGINT, SIG_IGN);
    ash_loop();

    return EXIT_SUCCESS;
}
