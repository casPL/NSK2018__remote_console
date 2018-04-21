#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 1024
#define NUM_THREADS     5


//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
	int desc_new;
};

//wskaźnik na funkcję opisującą zachowanie wątku
void *ThreadBehavior(void *t_data)
{
	struct thread_data_t *th_data = (struct thread_data_t*)t_data;
	//dostęp do pól struktury: (*th_data).pole
	printf("Socket descriptor to: %i\n", (*th_data).desc_new);
	char text[BUF_SIZE] = {0};
	printf("> ");
	while (fgets(text, BUF_SIZE, stdin) == text) {
		if(strncmp(text, "bye\n", strlen("bye\n")) == 0) break;
		if(send((*th_data).desc_new, text, strlen(text)+1, 0) < 0) printf("Error while sending");
	}
	char buf[BUF_SIZE] = {0};
	int filled = 0;	
	while(filled = recv((*th_data).desc_new, buf, BUF_SIZE-1, 0)) {
		buf[filled] = '\0';
		printf("%s", buf);
		fflush(stdout);								}	
	printf("Terminating connection.\n");
    	pthread_exit(NULL);
}


//funkcja obsługująca połączenie z serwerem
void handleConnection(int connection_socket_descriptor) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;
	struct thread_data_t * t_data;
    
	t_data = malloc(sizeof(struct thread_data_t));
	t_data->desc_new = connection_socket_descriptor;

	create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);


	char text2[BUF_SIZE];
	int len = 0;
//	while (1) {
//		fgets(text2, BUF_SIZE, stdin);
//		len = strlen(text2);
//		write (connection_socket_descriptor, text2, len);
//	}
	pthread_exit(NULL);

}


int main (int argc, char *argv[])
{
   int connection_socket_descriptor;
   int connect_result;
   struct sockaddr_in server_address;
   struct hostent* server_host_entity;

   if (argc != 3)
   {
     fprintf(stderr, "Syntax error: %s server_name port_number\n", argv[0]);
     exit(1);
   }

   server_host_entity = gethostbyname(argv[1]);
   if (! server_host_entity)
   {
      fprintf(stderr, "%s: Could not resolve domain name\n", argv[0]);
      exit(1);
   }

   connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
   if (connection_socket_descriptor < 0)
   {
      fprintf(stderr, "%s: Socket creation error\n", argv[0]);
      exit(1);
   }

   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
   server_address.sin_port = htons(atoi(argv[2]));

   connect_result = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (connect_result < 0)
   {
      fprintf(stderr, "%s: Błąd przy próbie połączenia z serwerem (%s:%i).\n", argv[0], argv[1], atoi(argv[2]));
      exit(1);
   }

   handleConnection(connection_socket_descriptor);

   close(connection_socket_descriptor);
   return 0;

}
