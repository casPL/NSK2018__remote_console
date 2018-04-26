/*
 *	Network console server
 *	inf138575
 *	PUT NSK2018
 */ 

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

//data structure which will be passed to thead
struct thread_data_t
{
	int desc_new;
};

//pointer to funcion describing thread behavior
void *ThreadBehavior(void *t_data)
{
    	struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    	//access to stucture's filds: (*th_data).field
	
	//sending to server section
	char buffer_local[BUF_SIZE];
	while (fgets(buffer_local, BUF_SIZE, stdin) >0)   //fget returns -1 when problem occurs
		write ((*th_data).desc_new, buffer_local, strlen(buffer_local));

	
	//close thread	
    	pthread_exit(NULL);
}


//connection handler
void handleConnection(int connection_socket_descriptor) {	
	printf("Connecting...\n");
	
	//result of creating a new thread
    int create_result = 0;

    //thread descriptor
    pthread_t thread1;

    //data which will be passed to thread
    struct thread_data_t t_data;
    t_data.desc_new = connection_socket_descriptor;


    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)&t_data);
    if (create_result){
       printf("Error while creating thread, error code: %d\n", create_result);
       exit(-1);
    }
	//reading server replys section
	char buf[BUF_SIZE];
	while(read(connection_socket_descriptor, buf, BUF_SIZE) >0) {
		buf[strlen(buf)] = '\0';
		printf("%s", buf);
		fflush(stdout);
		memset(buf, 0, BUF_SIZE);
	}
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
      fprintf(stderr, "%s: Could not reslove hostname.\n", argv[0]);
      exit(1);
   }

   connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
   if (connection_socket_descriptor < 0)
   {
      fprintf(stderr, "%s: Could not create a new socket.\n", argv[0]);
      exit(1);
   }

   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
   server_address.sin_port = htons(atoi(argv[2]));

   connect_result = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (connect_result < 0)
   {
      fprintf(stderr, "%s: Could not make a connection with server: %s:%i.\n", argv[0], argv[1], atoi(argv[2]));
      exit(1);
   }

   handleConnection(connection_socket_descriptor);

   close(connection_socket_descriptor);
   printf("Connection closed\n");
   return 0;

}
