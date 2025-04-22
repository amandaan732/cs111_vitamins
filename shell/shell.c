#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function
 * parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
//new
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    //me
    {cmd_pwd, "pwd", "print the current working directory"},
    {cmd_cd, "cd", "change the current working directory"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    }
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
    exit(0);
}

//actually printing the current working directory
int cmd_pwd(unused struct tokens *tokens) {
    char cwd[4096]; //buffer to store path of cwd
    if (getcwd(cwd, sizeof(cwd)) != NULL) { 
        //char *language = "korean";
        //printf("I speak %s!\n", language);
        ////Output: 
        ////I speak korean!
        printf("%s\n", cwd);
        return 1;
    } else { //if you cant get the cwd, print error
        perror("pwd");
        return -1;
    }
}

//actually changing the current working directory
int cmd_cd(struct tokens *tokens) {
    //get the first argument after the cd command and store in dir pointer
        //ex. cd ../Desktop --> "../Desktop" is stored in dir
    char *dir = tokens_get_token(tokens, 1);
    if (dir == NULL) { //no arguments is invalid
        fprintf(stderr, "cd: missing argument\n");
        return -1;
    }
    if (chdir(dir) != 0) { //if the dir is invalid, print error 
        perror("cd");
        return -1;
    }
    return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char *cmd) {
    if (cmd != NULL) {
        for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
            if (strcmp(cmd_table[i].cmd, cmd) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the
         * shell until it becomes a foreground process. We use SIGTTIN to pause
         * the shell. When the shell gets moved to the foreground, we'll receive
         * a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later.
         */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

int main(unused int argc, unused char *argv[]) {
    init_shell();

    static char line[4096];
    int line_num = 0;

    /* Only print shell prompts when standard input is not a tty */
    if (shell_is_interactive) {
        fprintf(stdout, "%d: ", line_num);
    }

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);

        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            /* REPLACE this to run commands as programs. */
            //plan:
                //" fork a child process, which calls one of the 
                //exec functions to run the new program. 
                //The parent process should wait until the child process completes and 
                //then continue listening for more commands."
            
                //so take the tokens of the input as arguments --> arguments[0] is the path, [1] is argument 1, etc
                //use execv and run the program (path, arguments[])
                //if its the parent, wait for child to be done 
                //print errors when necessary
            pid_t pid = fork(); //0 = child, - = error, + = parent
            if (pid == 0) { //child
                size_t length = tokens_get_length(tokens); //the number of tokens (directory + arguments)
                //char** is an array of strings (char* is a string with null as lest element but u can also just do a normal array)
                char **arguments = malloc((length + 1) * sizeof(char *)); //the last one in the array should be NULL (so length+1)
                for (size_t i = 0; i < length; i++) { //put the tokens into arguments array
                    arguments[i] = tokens_get_token(tokens, i);
                    // printf("%s\n", arguments[i]);
                }
                arguments[length] = NULL; //last element is NULL

                //pt 5 doing IO redirection
                int inpfd = -1; //these are the in/output file descriptors
                int outpfd = -1; //-1 is default/file not opened yet

                for (size_t i = 0; i < length; i++) { //go through all the tokens
                    if (arguments[i] == NULL) continue; //skip if the arg entry is null

                    if (strcmp(arguments[i], "<") == 0 && i + 1 < length) { //"if the arg is < and there is an argument after it too"
                        inpfd = open(arguments[i + 1], O_RDONLY); //reads it and puts file descriptor in in_fd
                        if (inpfd < 0) { //if u couldnt open it, print error
                            perror("opening input file");
                            exit(1);
                        }
                        //int dup2(int oldFileDescriptor, int newFileDescriptor);
                        dup2(inpfd, STDIN_FILENO); //replace child’s standard input (fd = 0) with newly opened file's fd
                        close(inpfd); //close the file!!! (its fd is copied to stdin now)
                        //just make the < and the file null so we skip it ; skipping iterations too finicky
                        arguments[i] = NULL;
                        arguments[i + 1] = NULL;
                    } else if (strcmp(arguments[i], ">") == 0 && i + 1 < length) { //same but > 
                        outpfd = open(arguments[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666); //write, create, erase, permissions
                        if (outpfd < 0) { //cant open file
                            perror("opening output file");
                            exit(1);
                        }
                        dup2(outpfd, STDOUT_FILENO); //replace stdout (fd = 1) with outpfd
                        close(outpfd); 
                        arguments[i] = NULL;
                        arguments[i + 1] = NULL;
                    }
                }

                // execv(arguments[0], arguments); //try to run program
                //****PT 4 modifies ^pt3 to ALSO allow use of "PATH variable from the environment to resolve program names"
                    //plan:
                        //if args[0] starts with '/' then keep it the same as pt 3 (the path is given)
                        //otherwise:
                            //need to look up PATH and get all the directories in a string (separated by ':')
                            //append '/program_name' to each dir in the PATH's long string
                            //put that path inside execv and try running program until it works or all directories were checked
                
                if (strchr(arguments[0], '/') != NULL) { //strchr gets first instance of / in args[0]
                    execv(arguments[0], arguments); //same as pt 3
                    perror("execv");
                    exit(1);
                }

                //for PATH:
                char *path = getenv("PATH"); //get the actual string
                if (path == NULL) { //if it couldn't get the string then exit 
                    fprintf(stderr, "couldn't get path string\n");
                    exit(1);
                }

                char *pathCopy = strdup(path); //need to make copies bc strtok will directly modify the string
                char *dir = strtok(pathCopy, ":"); //tokenize at each : (separate directories)
                while (dir != NULL) {
                    char fullPath[4096]; //decided to allocate space w/o using malloc; recommended on stack overflow to use 4096
                    snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, arguments[0]); //safer print for no buffer overflow

                    execv(fullPath, arguments); //try running program; if it fails, then go to next directory
                    dir = strtok(NULL, ":"); //will only go here if execv failed (going to next :/directory)
                }

                perror("execv"); //execv returning == an error (invalid path), so exit child process
                exit(1); //exit with an error
                //*NOTE:
                    //so execv replaces the child process with /usr/bin/wc, so the waitpid
                    //status that is stored in parent process should be 0 if everything went well
                    //and 1 if the execv failed
                    //either way, waitpid sees that the child finished, so we can move on
            } else if (pid > 0) { //parent
                int childCompleted; //somewhere to store child’s exit status
                waitpid(pid, &childCompleted, 0); //0 flag == wait for child to exit normally
                //wiat for THIS specific child to finish
            } else { //pid is negative == error
                perror("fork");
            }
        }

        if (shell_is_interactive) {
            /* Only print shell prompts when standard input is not a tty. */
            fprintf(stdout, "%d: ", ++line_num);
        }

        /* Clean up memory. */
        tokens_destroy(tokens);
    }

    return 0;
}
