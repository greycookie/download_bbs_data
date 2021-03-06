#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mt.h"

void send_cmd(int sock, int pid);
void receive(int sock);
void keyword_init(void);
void command_init(void);
int find_substr(char *str, int str_size, char *substr, int substr_size);
void dump_article(int sock);

enum command_enum{UP, DOWN, LEFT, RIGHT, HOME, END, ENTER, GUEST, COMMAND_ENUM_END};
typedef struct _command
{
	char *str;
	unsigned int size;
} command;
command command_set[COMMAND_ENUM_END];

enum keyword_enum{ACCOUNT, ANYKEY, MAIN_PAGE, ARTICLE_LIST, BROWSE, ARTICLE_END, KEYWORD_ENUM_END};
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
		// printf("%s", buf);
		// fflush(stdout);
		if(find_substr(buf, filled, keyword_set[ACCOUNT].str, keyword_set[ACCOUNT].size) != -1)
		{
			write(pipe_fd[1], "7", 1);
		}
		else if(find_substr(buf, filled, keyword_set[ANYKEY].str, keyword_set[ANYKEY].size) != -1)
		{
			write(pipe_fd[1], "6", 1);
		}
		else if(find_substr(buf, filled, keyword_set[MAIN_PAGE].str, keyword_set[MAIN_PAGE].size) != -1)
		{
			// 下右下下下右(下＊35)
			write(pipe_fd[1], "13111311111111111111111111111111111111111324", 44);
		}
		else if(find_substr(buf, filled, keyword_set[ARTICLE_LIST].str, keyword_set[ARTICLE_LIST].size) != -1)
		{
			write(pipe_fd[1], "3", 1);
			dump_article(sock);
			//write(pipe_fd[1], "21", 3);
			printf("dump one article\n");
			pause();
		}
	}
	printf("Server disconnected.\n");
}

void command_init(void)
{
	command_set[UP].str = (char*)malloc(sizeof(char)*3);
	command_set[UP].str[0] = 0x1B;
	command_set[UP].str[1] = 0x5B;
	command_set[UP].str[2] = 0x41;
	command_set[UP].size = 3;

	command_set[DOWN].str = (char*)malloc(sizeof(char)*3);
	command_set[DOWN].str[0] = 0x1B;
	command_set[DOWN].str[1] = 0x5B;
	command_set[DOWN].str[2] = 0x42;
	command_set[DOWN].size = 3;

	command_set[LEFT].str = (char*)malloc(sizeof(char)*3);
	command_set[LEFT].str[0] = 0x1B;
	command_set[LEFT].str[1] = 0x5B;
	command_set[LEFT].str[2] = 0x44;
	command_set[LEFT].size = 3;

	command_set[RIGHT].str = (char*)malloc(sizeof(char)*3);
	command_set[RIGHT].str[0] = 0x1B;
	command_set[RIGHT].str[1] = 0x5B;
	command_set[RIGHT].str[2] = 0x43;
	command_set[RIGHT].size = 3;

	command_set[HOME].str = (char*)malloc(sizeof(char)*1);
	command_set[HOME].str[0] = '0';
	command_set[HOME].size = 1;

	command_set[END].str = (char*)malloc(sizeof(char)*1);
	command_set[END].str[0] = '$';
	command_set[END].size = 1;

	command_set[ENTER].str = (char*)malloc(sizeof(char)*1);
	command_set[ENTER].str[0] = '\n';
	command_set[ENTER].size = 1;

	command_set[GUEST].str = (char*)malloc(sizeof(char)*6);
	strncpy(command_set[GUEST].str, "guest", 5);
	command_set[GUEST].str[5] = '\n';
	command_set[GUEST].size = 6;
}

void keyword_init(void)
{
	// 您的帳號
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

	// 任意鍵
	keyword_set[ANYKEY].str = (char*)malloc(sizeof(char)*6);
	keyword_set[ANYKEY].str[0] = 0xA5;
	keyword_set[ANYKEY].str[1] = 0xF4;
	keyword_set[ANYKEY].str[2] = 0xB7;
	keyword_set[ANYKEY].str[3] = 0x4E;
	keyword_set[ANYKEY].str[4] = 0xC1;
	keyword_set[ANYKEY].str[5] = 0xE4;
	keyword_set[ANYKEY].size = 6;

	// 主功能表
	keyword_set[MAIN_PAGE].str = (char*)malloc(sizeof(char)*8);
	keyword_set[MAIN_PAGE].str[0] = 0xA5;
	keyword_set[MAIN_PAGE].str[1] = 0x44;
	keyword_set[MAIN_PAGE].str[2] = 0xA5;
	keyword_set[MAIN_PAGE].str[3] = 0x5C;
	keyword_set[MAIN_PAGE].str[4] = 0xAF;
	keyword_set[MAIN_PAGE].str[5] = 0xE0;
	keyword_set[MAIN_PAGE].str[6] = 0xAA;
	keyword_set[MAIN_PAGE].str[7] = 0xED;
	keyword_set[MAIN_PAGE].size = 8;

	// 文章列表
	keyword_set[ARTICLE_LIST].str = (char*)malloc(sizeof(char)*8);
	keyword_set[ARTICLE_LIST].str[0] = 0xA4;
	keyword_set[ARTICLE_LIST].str[1] = 0xE5;
	keyword_set[ARTICLE_LIST].str[2] = 0xB3;
	keyword_set[ARTICLE_LIST].str[3] = 0xB9;
	keyword_set[ARTICLE_LIST].str[4] = 0xA6;
	keyword_set[ARTICLE_LIST].str[5] = 0x43;
	keyword_set[ARTICLE_LIST].str[6] = 0xAA;
	keyword_set[ARTICLE_LIST].str[7] = 0xED;
	keyword_set[ARTICLE_LIST].size = 8;

	// [34;46m   瀏覽 P
	keyword_set[BROWSE].str = (char*)malloc(sizeof(char)*14);
	keyword_set[BROWSE].str[0] = '[';
	keyword_set[BROWSE].str[1] = '3';
	keyword_set[BROWSE].str[2] = '4';
	keyword_set[BROWSE].str[3] = ';';
	keyword_set[BROWSE].str[4] = '4';
	keyword_set[BROWSE].str[5] = '6';
	keyword_set[BROWSE].str[6] = 'm';
	keyword_set[BROWSE].str[7] = ' ';
	keyword_set[BROWSE].str[8] = 0xC2;
	keyword_set[BROWSE].str[9] = 0x73;
	keyword_set[BROWSE].str[10] = 0xC4;
	keyword_set[BROWSE].str[11] = 0xFD;
	keyword_set[BROWSE].str[12] = ' ';
	keyword_set[BROWSE].str[13] = 'P';
	keyword_set[BROWSE].size = 14;

	// [34;46m  文章選讀
	keyword_set[ARTICLE_END].str = (char*)malloc(sizeof(char)*16);
	keyword_set[ARTICLE_END].str[0] = '[';
	keyword_set[ARTICLE_END].str[1] = '3';
	keyword_set[ARTICLE_END].str[2] = '4';
	keyword_set[ARTICLE_END].str[3] = ';';
	keyword_set[ARTICLE_END].str[4] = '4';
	keyword_set[ARTICLE_END].str[5] = '6';
	keyword_set[ARTICLE_END].str[6] = 'm';
	keyword_set[ARTICLE_END].str[7] = ' ';
	keyword_set[ARTICLE_END].str[8] = 0xA4;
	keyword_set[ARTICLE_END].str[9] = 0xE5;
	keyword_set[ARTICLE_END].str[10] = 0xB3;
	keyword_set[ARTICLE_END].str[11] = 0xB9;
	keyword_set[ARTICLE_END].str[12] = 0xBF;
	keyword_set[ARTICLE_END].str[13] = 0xEF;
	keyword_set[ARTICLE_END].str[14] = 0xC5;
	keyword_set[ARTICLE_END].str[15] = 0xAA;
	keyword_set[ARTICLE_END].size = 16;
}

int find_substr(char *str, int str_size, char *substr, int substr_size)
{
	int i, j;
	int result = -1;

	for(i = 0; i < str_size-substr_size+1; i++)
	{
		for(j = 0; j < substr_size; j++)
		{
			if(str[i+j] != substr[j])
				break;
		}
		if(j == substr_size)
		{
			result = i;
			break;
		}
	}

	return result;
}

void dump_article(int sock)
{
	int first = 1;
	int rt_val;
	char buf[MAX_MSG_LENGTH] = {0};
	char *tmp;
	int filled = 0;
	int tmp_filled;
	FILE *fp;

	fp = fopen("test.txt", "w+");

	while(filled = recv(sock, buf, MAX_MSG_LENGTH-1, 0))
	{
		tmp_filled = filled;

		if(first == 1)
		{
			tmp = buf;
			first = !first;
		}
		else
		{
			tmp = buf + 21;
			filled -= 21;
		}

		if((rt_val = find_substr(tmp, filled, keyword_set[BROWSE].str, keyword_set[BROWSE].size)) != -1)
		{
			fwrite(tmp, sizeof(char), rt_val, fp);
			write(pipe_fd[1], "3", 1);
		}
		else if((rt_val = find_substr(tmp, filled, keyword_set[ARTICLE_END].str, keyword_set[ARTICLE_END].size)) != -1)
		{
			fwrite(tmp, sizeof(char), rt_val, fp);
			break;
		}
	}
	
	fclose(fp);
}