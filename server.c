/**********************************************
 * Jan Kuliga, Krzysztof Bera, Konrad Sikora
 * ********************************************/
/*
 *INCLUDES
 */
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <pthread.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "strmap.h"

/*
 * MACROS
 */
#define PORT 22222
#define QUEUE_LEN 4
#define MAX_RCV_LEN 2048
#define STR_LEN 2000

enum {
	SERV,
	CLI
 } addr;

 enum {
	 LISTEN,
	 CONN
 } sock;

/*
 * FUNCTION PROTOTYPES
 */
/*Thread function to handle communication with clients*/
void *server_thread(void *par);

/*Call back function for hashmap enumerator*/
void clients_list(const void *key, const void *val, void *obj);

void sig_pipe(void);

void sig_child(void);
/*
 * STATIC VARIABLES
 */
static int queue = 0;

/*hashmap used to hold players' names' as keys and their adresses*/
/*<Nickname, IP address>*/
static StrMap *clients;

/*mutex variable, used to lock resources*/
static pthread_mutex_t sync_proc;

/* made it a global variable so that we could close it when sigpipe is generated*/
int fd[2];

int main(int argc, char *argv[])
{
	struct sockaddr_in addr[2];
	int port;
	socklen_t addr_len;
	pthread_t thr_id;
	
	/*mutex_init*/
	pthread_mutex_init(&sync_proc, NULL);
	
	/*initialize hashmap*/
	clients = sm_new(QUEUE_LEN);
	
	/*turn off stdout buffering*/
	setvbuf(stdout, NULL, _IONBF, 0);
	
	/*reset sockaddr_in structures*/
	memset((void*) &addr[SERV], 0, sizeof(addr[SERV]));
	memset((void*) &addr[CLI], 0, sizeof(addr[CLI]));
	
	/*set local IP address*/
	addr[SERV].sin_family = AF_INET;
	addr[SERV].sin_addr.s_addr = INADDR_ANY;
	
	/*Check whether port was specified during server start-up*/
	if (argc > 1) 
		port = atoi(argv[1]);
	else 
		port = PORT;
			
	/*Check whether specified port is valid*/
	if (port > 0) {
		addr[SERV].sin_port = htons(port);
	} else {
		fprintf(stderr, "Invalid port number. Shutting down server...\n");
		goto exit;
	}
	
	/*Create a socket*/
	if ((fd[LISTEN] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Critical failure when creating socket. Shutting down server...\n");
		goto exit;
	}
	
	/*Bind a adress to the socket*/
	if (bind(fd[LISTEN], (struct sockaddr*) &addr[SERV], sizeof(addr[SERV])) < 0) {
		fprintf(stderr, "Critical failure when binding address. Shutting down server...\n");
		goto exit;
	}
	/*Specify maximum number of players*/
	if (listen(fd[LISTEN], QUEUE_LEN) < 0) {
		fprintf(stderr, "Critical failure when specifying number of players. Shutting down server...\n");
		goto exit;
	}
	
	printf("Server's setup correct. Waiting for players to join...\n");
	addr_len = sizeof(addr[CLI]); 
	
	/* Register handler of sigpipe signal */
	signal(SIGPIPE, (__sighandler_t) sig_pipe);
	
	/*Server's main loop*/
	while (1) {
		if ((fd[CONN] = accept(fd[LISTEN], (struct sockaddr*) &addr[CLI], &addr_len)) < 0) {
			fprintf(stderr, "Critical failure during accepting request.\n");
			goto exit;
		}
		/*Create seperate thread with default attributes for every accepted client*/
		pthread_create(&thr_id, NULL, &server_thread, (void*) &fd[CONN]);
	}
exit:		
	close(fd[LISTEN]);
	close(fd[CONN]);
	sm_delete(clients);
}		

/*
* FUNCTIONS` DEFINITIONS
*/
void *server_thread(void *par)
{
	int thread_fd;
	int len;
	char addr_buf[INET_ADDRSTRLEN];
	struct sockaddr_in peer_addr;
	socklen_t addr_len;
	char data_buf[MAX_RCV_LEN + 1];
	char player_name[STR_LEN + 1];
	
	/*assign passed file desciptor to new thread*/
	thread_fd = (*(int*) par);

	/*reset sockaddr_in structure*/
	memset((void*) &peer_addr, 0, sizeof(peer_addr));
	
	addr_len = sizeof(peer_addr);
	
	/*retrieve IP address of the peer connected to this particular socket*/
	getpeername(thread_fd, (struct sockaddr*) &peer_addr, &addr_len);
	
	inet_ntop(AF_INET, &(peer_addr.sin_addr), addr_buf, INET_ADDRSTRLEN); 
	
	/*thread's main loop*/
	char arg1[STR_LEN], arg2[STR_LEN];

	while ((len = recv(thread_fd, data_buf, MAX_RCV_LEN, 0))) {
		data_buf[len] = '\0';
		
		/*split received string into two substrings*/
		sscanf(data_buf, "%s %s", arg1, arg2);
		
		/*join*/
		if ((strcmp(arg1, "join") == 0) && (strcmp(arg2, "") != 0)) {
			/*block access to variables by other threads; only this particular thread
			can now access a set of variables*/
			/*works only on threads created within one process*/
			pthread_mutex_lock(&sync_proc);
			
			/*create new entry in hashmap*/
			if (sm_exists(clients, arg2) == 0) {
				sm_put(clients, arg2, addr_buf);
				queue++;
				strcpy(player_name, arg2);
				printf("Player %s added to server's queue\n", player_name);
				sprintf(data_buf, "Your nickname %s has been added to server's queue\n", player_name);
			} else {
				sprintf(data_buf, "You're already on player's list!\n");
			}
			/*free used resources*/
			pthread_mutex_unlock(&sync_proc);
			send(thread_fd, data_buf, strlen(data_buf), 0);
		/*invite <playername>*/
		} else if ((strcmp(arg1, "invite") == 0) && (strcmp(arg2, "") != 0)) {
			pthread_mutex_lock(&sync_proc);

			if ((sm_exists(clients, arg2)) && (queue > 0)) 
				sm_get(clients, arg2, data_buf, sizeof(data_buf));
			else
				sprintf(data_buf, "There's no such player on server's list!\n");
			
			pthread_mutex_unlock(&sync_proc);
			send(thread_fd, data_buf, sizeof(data_buf), 0);
		/*list*/
		} else if (strcmp(arg1, "list") == 0) {
			pthread_mutex_lock(&sync_proc);
			
			if (queue > 0) {
				sprintf(data_buf, "List of players: \n");
				sm_enum(clients, (sm_enum_func) clients_list, (const void*) data_buf);
			} else {
				sprintf(data_buf, "No one is online\n");
			}
			pthread_mutex_unlock(&sync_proc);
			send(thread_fd, data_buf, strlen(data_buf), 0);
		}
	}
	/*When clients closes connection, remove entry from hashmap, close the socket's file descriptor and thread*/
	pthread_mutex_lock(&sync_proc);
	if (strlen(player_name) > 1) {
		sm_get(clients, player_name, data_buf, sizeof(data_buf));
		if (strcmp(data_buf, addr_buf) == 0) {
			sm_remove(clients, player_name);
			queue--;
			printf("Player %s removed from player's list\n", player_name);
			player_name[0] = '\0';
		}
	}
	pthread_mutex_unlock(&sync_proc);
	close(thread_fd);
	pthread_exit(0);
}

void clients_list(const void *key, const void *val, void *obj)
{
	if (val != NULL) {
		strcat(obj, key);
		strcat(obj, "\n");
	}
}

void sig_pipe(void)
{
	// printf("Server received SIGPIPE - Default action is exit \n");
	close(fd[CONN]);
	printf("\nConnection interrupted");
	exit(1);
}
