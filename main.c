// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCMD 1000 // max number of letters to be supported
#define MAXLISTCMD 100 // max number of commands to be supported

// practic functia de clear la shell, prima parte pana la \ muta cursorul stanga sus, iar a 2 sterge continutu
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
void start_shell()
{
	clear();
    printf("\n\n```````````Welcome to our Shell``````````\n\n");
    char *user_name = getenv("USER");
    printf("\n\n```````````User : %s``````````\n\n", user_name);

    sleep(1);
	clear();
}

// Function to take input
int get_input(char* str)
{
	char* buf;

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
void get_dir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nDir: %s", cwd);
}

// Aici se executa functiile sistem built-in ( gen ls saumai st iu eu ce, alea de poti sa le faci cu execve direct)
void exec_cmd(char** parsed_cmd)
{

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
void get_help()
{
	puts("\n```WELCOME TO OUR SHELL```"
		"\n Available commands : "
		"\n->cd"
		"\n->ls"
		"\n->exit"
		"\n->all other general commands available in UNIX shell");

	return;
}

// Aici executam comenzi custom specifice shell ului nostru 
int own_cmd(char** parsed_cmd)
{
	int nr_cmd = 3, i, switch_cmd = 0;
	char* Lista_cmd[nr_cmd];
	char* user_name;

	Lista_cmd[0] = "exit";
	Lista_cmd[1] = "cd";
	Lista_cmd[2] = "help";


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
		chdir(parsed_cmd[1]);
		return 1;
	case 3:
		get_help();
		return 1;
	default:
		break;
	}

	return 0;
}



// functie pentru a separata cuvintele din comanda
void sep_cmd(char* cmd, char** parsed_cmd)
{
	int i;

	for (i = 0; i < MAXLISTCMD; i++) {
		parsed_cmd[i] = strsep(&cmd, " ");

		if (parsed_cmd[i] == NULL)
			break;
		if (strlen(parsed_cmd[i]) == 0)
			i--;
	}
}

int process_cmd(char* cmd, char** parsed_cmd, char** parsed_cmd_pipe)
{

	char* cmd_piped[2];
	int pipe = 0;

	sep_cmd(cmd, parsed_cmd);


	if (own_cmd(parsed_cmd))
		return 0;
	else
		return 1 ;
}

int main()
{
	char input_cmd[MAXCMD], *parsed_cmd[MAXLISTCMD];
	char* parsed_cmd_piped[MAXLISTCMD];
	int execFlag = 0;
	start_shell();

	while (1) {
		// print shell line
		get_dir();
		// take input
		if (get_input(input_cmd))
			continue;
		// process
		execFlag = process_cmd(input_cmd,
		parsed_cmd, parsed_cmd_piped);
        /*
        0 daca nu e nici o comanda
        1 daca e o comanda simpla
        2 daca e comanda piped ( adica cu | )
        */
		// execute
        
		if (execFlag == 1)
			exec_cmd(parsed_cmd);

        /*
		if (execFlag == 2)
			exec_cmd_piped(parsed_cmd, parsed_cmd_piped);
            */
	}
	return 0;
}
