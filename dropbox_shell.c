#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/history.h>
#include<readline/readline.h>
#include<dirent.h>
#include<errno.h>
#include<fcntl.h>

int mkdir(const char * pathname, mode_t mode);

#define MAXCMD 1000
#define MAXLISTCMD 100

// practic functia de clear la shell, prima parte pana la \ muta cursorul stanga sus, iar a 2 sterge continutu
#define clear() printf("\033[H\033[J")

void start_shell() {
    clear();
    printf("\n\n```````````Welcome to our Shell``````````\n\n");
    char * user_name = getenv("USER");
    printf("\n\n```````````User : %s``````````\n\n", user_name);

    sleep(2);
    clear();
}

int get_input(char * str) {
    char * buf;

    buf = readline("\n>>> ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// Printeaza directorul in care te afli 
void get_dir() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

// Aici se executa functiile sistem built-in 
void exec_cmd(char ** parsed_cmd) {

    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed_cmd[0], parsed_cmd) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {

        wait(NULL);
        return;
    }
}

// Comanda help
void get_help() {
    puts("\n```WELCOME TO OUR SHELL```"
        "\n Available commands : "
        "\n->our_cd"
        "\n->our_mkdir"
        "\n->our_dir"
        "\n->our_rm"
        "\n->our_ls"
        "\n->our_exit"
        "\n->our_cp"
        "\n->our_mv"
        "\n->our_touch"
        "\n->our_rm_dir"
        "\n->all other general commands available in UNIX shell, but of course, without our. So if you execute ls, it will still work.");

    return;
}

void get_mkdir(char ** parsed_cmd) {
    if (mkdir(parsed_cmd[1], 0777) == -1) {
        perror("Unable to create directory\n");
    } else {
        printf("Directory %s  created with succes!\n", parsed_cmd[1]);
    }
}

void get_cd(char ** parsed_cmd) {
    if (parsed_cmd[1] == NULL) {
        printf("You did not specify where to go. Repeat the cd command by using cd <path> ");
    }
    if (chdir(parsed_cmd[1]) != 0) {
        perror("Change failed.");
    }

}

// propria comanda de copiere de fisiere
void get_cp(char * src_file, char * dest_file) {
    int fd_in = open(src_file, O_RDONLY, S_IRUSR);
    int fd_out = open(dest_file, O_WRONLY, S_IWUSR);
    const int BUFF_SIZE = 1024;
    char * buffer = malloc(BUFF_SIZE * sizeof(char));

    int bytes_read = read(fd_in, buffer, BUFF_SIZE);
    while (bytes_read > 0) {
        write(fd_out, buffer, bytes_read);
        bytes_read = read(fd_in, buffer, BUFF_SIZE);
    }

    free(buffer);
    close(fd_in);
    close(fd_out);
}

// propria comanda de mutare de fisiere
void get_mv(char * src, char * dst) {
    get_cp(src, dst);
    remove(src);
}

// propria comanda de creare de fisiere
void get_touch(char * file_name) {
    open(file_name, O_CREAT | S_IRUSR | S_IWUSR);
}

/*
practic in structul definit dirent avem d_reclen care e length , Si d_name care este numele fisierului
*/

void get_ls() {
    int x;

    struct dirent ** d;

    x = scandir(".", & d, NULL, alphasort);

    if (x < 0) {
        perror("Cannot scan the directory");
    } else {
        while (x--) {
            printf("%s      ", d[x] -> d_name);
            free(d[x]);

        }

        free(d);
    }
}

void get_rmdir(char ** parsed_cmd) {
    if (rmdir(parsed_cmd[1]) == -1) {
        perror("Can't delete this directory");

    } else {
        printf("Directory with name : %s deleted succesfully", parsed_cmd[1]);
    }
}

void get_rm(char ** parsed_cmd) {
    if (remove(parsed_cmd[1]) == -1) {
        perror("Unable to delete this file");
    } else {
        printf("Deleted file with name : %s succesfully", parsed_cmd[1]);
    }
}
// Aici executam comenzi custom specifice shell ului nostru 
int own_cmd(char ** parsed_cmd) {
    int nr_cmd = 10, i, switch_cmd = 0;
    char * Lista_cmd[nr_cmd];
    char * user_name;

    Lista_cmd[0] = "our_exit";
    Lista_cmd[1] = "our_cd";
    Lista_cmd[2] = "our_mkdir";
    Lista_cmd[3] = "our_help";
    Lista_cmd[4] = "our_rm_dir";
    Lista_cmd[5] = "our_rm";
    Lista_cmd[6] = "our_ls";
    Lista_cmd[7] = "our_cp";
    Lista_cmd[8] = "our_mv";
    Lista_cmd[9] = "our_touch";

    for (i = 0; i < nr_cmd; i++) {
        if (strcmp(parsed_cmd[0], Lista_cmd[i]) == 0) {
            switch_cmd = i + 1;
            break;
        }
    }

    switch (switch_cmd) {
    case 1:
        printf("\n Have a nice day ! \n");
        exit(0);
    case 2:
        get_cd(parsed_cmd);
        return 1;
    case 3:
        get_mkdir(parsed_cmd);
        return 1;
    case 4:
        get_help();
        return 1;
    case 5:
        get_rmdir(parsed_cmd);
        return 1;
    case 6:
        get_rm(parsed_cmd);
        return 1;
    case 7:
        get_ls();
        return 1;
    case 8:
        get_cp(parsed_cmd[1], parsed_cmd[2]);
        return 1;
    case 9:
        get_mv(parsed_cmd[1], parsed_cmd[2]);
        return 1;
    case 10:
        get_touch(parsed_cmd[1]);
        return 1;
    default:
        break;
    }

    return 0;
}

// functie pentru a separata cuvintele din comanda
void sep_cmd(char * cmd, char ** parsed_cmd) {
    int i;

    for (i = 0; i < MAXLISTCMD; i++) {
        parsed_cmd[i] = strsep( & cmd, " ");

        if (parsed_cmd[i] == NULL)
            break;
        if (strlen(parsed_cmd[i]) == 0)
            i--;
    }
}

int process_cmd(char * cmd, char ** parsed_cmd, char ** parsed_cmd_pipe) {

    char * cmd_piped[2];
    int pipe = 0;

    sep_cmd(cmd, parsed_cmd);

    if (own_cmd(parsed_cmd))
        return 0;
    else
        return 1;
}

int main() {
    char input_cmd[MAXCMD], * parsed_cmd[MAXLISTCMD];
    char * parsed_cmd_piped[MAXLISTCMD];
    int execFlag = 0;
    start_shell();

    while (1) {
        get_dir();
        if (get_input(input_cmd))
            continue;

        execFlag = process_cmd(input_cmd,
            parsed_cmd, parsed_cmd_piped);

        if (execFlag == 1)
            exec_cmd(parsed_cmd);

    }
    return 0;
}