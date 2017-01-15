#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mt.h"

void send_cmd(int sock, int pid);
void receive(int sock);
void keyword_init(void);
void command_init(void);
int find_substr(char *str, int str_size, char *substr, int substr_size);

enum command_enum{UP, DOWN, LEFT, RIGHT, ENTER, GUEST, COMMAND_ENUM_END};
typedef struct _command
{
	char *str;
	unsigned int size;
} command;
command command_set[COMMAND_ENUM_END];

enum keyword_enum{ACCOUNT, KEYWORD_ENUM_END};
typedef struct _keyword
{
	char *str;
	unsigned int size;
}keyword;
keyword keyword_set[KEYWORD_ENUM_END];

int pipe_fd[2];

int main(int argc, char **argv)
{
	if(argc != 2) perro("args");

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) perro("socket");

	struct in_addr server_addr;
	if(!inet_aton(argv[1], &server_addr)) perro("inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(PORT);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("connect");

	if(pipe(pipe_fd) == -1)
	{
		printf("create pipe error\n");
		perror("pipe");
		return 1;
	}

	int pid;
	if(pid = fork())
	{
		close(pipe_fd[1]);
		send_cmd(sock, pid);
	}
	else
	{
		close(pipe_fd[0]);
		receive(sock);
	}

	return 0;
}

void send_cmd(int sock, int pid)
{
	int command;
	char pipe_buf[3];
	fd_set pipe_fd_set;
	fd_set tmp;

	FD_ZERO(&pipe_fd_set);
	FD_SET(pipe_fd[0], &pipe_fd_set);
	FD_SET(STDIN_FILENO, &pipe_fd_set);

	command_init();

	while (1) {
		tmp = pipe_fd_set;
		select(pipe_fd[0]+1, &tmp, NULL, NULL, NULL);
		if(FD_ISSET(pipe_fd[0], &tmp))
		{
			read(pipe_fd[0], pipe_buf, 1);
			pipe_buf[1] = '\0';
			command = atoi(pipe_buf);
			send(sock, command_set[command].str, command_set[command].size, 0);
		}
		if(FD_ISSET(STDIN_FILENO, &tmp))
			break;
	}
	kill(pid, SIGKILL);
	printf("Goodbye.\n");
}

void receive(int sock)
{
	char buf[MAX_MSG_LENGTH] = {0};
	int filled = 0;
	keyword_init();
	while(filled = recv(sock, buf, MAX_MSG_LENGTH-1, 0))
	{
		buf[filled] = '\0';
		printf("%s", buf);
		fflush(stdout);
		if(find_substr(buf, strlen(buf), keyword_set[ACCOUNT].str, keyword_set[ACCOUNT].size) == 1)
		{
			write(pipe_fd[1], "5", 1);
		}
	}
	printf("Server disconnected.\n");
}

void command_init(void)
{
	command_set[GUEST].str = (char*)malloc(sizeof(char)*6);
	strncpy(command_set[GUEST].str, "guest", 5);
	command_set[GUEST].str[5] = '\n';
	command_set[GUEST].size = 6;
}

void keyword_init(void)
{
	// {0xB1, 0x7A, 0xAA, 0xBA, 0xB1, 0x62, 0xB8, 0xB9, '\0'} == 您的帳號
	keyword_set[ACCOUNT].str = (char*)malloc(sizeof(char)*8);
	keyword_set[ACCOUNT].str[0] = 0xB1;
	keyword_set[ACCOUNT].str[1] = 0x7A;
	keyword_set[ACCOUNT].str[2] = 0xAA;
	keyword_set[ACCOUNT].str[3] = 0xBA;
	keyword_set[ACCOUNT].str[4] = 0xB1;
	keyword_set[ACCOUNT].str[5] = 0x62;
	keyword_set[ACCOUNT].str[6] = 0xB8;
	keyword_set[ACCOUNT].str[7] = 0xB9;
	keyword_set[ACCOUNT].size = 8;
}

int find_substr(char *str, int str_size, char *substr, int substr_size)
{
	int i, j;
	int result = 0;

	for(i = 0; i < str_size-substr_size+1; i++)
	{
		for(j = 0; j < substr_size; j++)
		{
			if(str[i+j] != substr[j])
				break;
		}
		if(j == substr_size)
			result = 1;
	}

	return result;
}
