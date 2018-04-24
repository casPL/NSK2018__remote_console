/* 
	Simple network console server.
	inf138575
	PUT :: NSK 2018
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define QUEUE_SIZE 5
#define BUF_SIZE 1024
#define PROMPT "> "
#define GOODBYE_MSG "Bye... \n"
#define STDERR_POSTFIX "2>&1"


#define FLAG		0x10
#define ESC		0x11
#define	ESC_FLAG	0x01
#define ESC_ESC		0x02

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
	int new_socket_descriptor;
};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data) {
	pthread_detach(pthread_self());
	struct thread_data_t *th_data = (struct thread_data_t*)t_data;
	// access to  structure's fields: (*th_data).field
    	char command[BUF_SIZE +1];
    	char reply[BUF_SIZE +1];
   	FILE *output_fd;
   	FILE *input_fd;
    	char *read_result;

	//The fdopen() function  associates  a  stream  with  the  existing  file descriptor,  fd.
        input_fd = fdopen((*th_data).new_socket_descriptor,"r"); 
        	
	while (1) {
		write((*th_data).new_socket_descriptor, PROMPT, sizeof(PROMPT));
            	read_result = fgets(command, BUF_SIZE, input_fd); //fgets() reads until new line or EOF
		if (read_result == NULL) {
			printf("Error while reading command\n");
                	break;
		}
		read_result = strchr(command,'\n'); 
		if (read_result != NULL) 
			*read_result = 0;
            	
		printf("Recognized command '%s'\n", command);

            	if (strcmp(command,"exit") == 0 || strcmp(command, "quit") == 0) {
                	printf("The client is closing connection\n");
			write((*th_data).new_socket_descriptor, GOODBYE_MSG, sizeof(GOODBYE_MSG));
			break;
		}
            	output_fd = popen(command, "r");
            	while (1) {
                		read_result = fgets(reply, BUF_SIZE, output_fd);
                		if (read_result == NULL) 
                    			break;
                		write((*th_data).new_socket_descriptor, reply, strlen(reply));
            	}
            	pclose(output_fd);
        }
        fclose(input_fd);
    	close((*th_data).new_socket_descriptor);
   	free(t_data);	
	printf("Connection terminated\n");
    	pthread_exit(NULL);	
}

//handling connection with new client
void handleConnection(int connection_socket_descriptor) {

    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    //TODO wypełnienie pól struktury
    
    struct thread_data_t * t_data;
    t_data = malloc(sizeof(struct thread_data_t));
    t_data->new_socket_descriptor = connection_socket_descriptor;
    
    
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Pthread error, code : %d\n", create_result);
       exit(-1);
    }
}

int main(int argc, char* argv[])
{
	//safe start section
	
	if ( argc <= 1) {
		printf("Error. Bad syntax. Misiing argument\nProper usage: ./server [port_number]\n");
		exit(1);
	}
	printf("Starting program: %s with arguments: %s\n", argv[0], argv[1]);
	unsigned int server_port = atoi(argv[1]);
	if(server_port <1025 || server_port >65535) {
		printf("Error specified port must be between 1025 and 65535 inclusive\n");
		exit(2);
	}
	printf("Setting port to: %d\n", server_port);
	
	//section end
	
   int server_socket_descriptor;
   int connection_socket_descriptor;
   int bind_result;
   int listen_result;
   char reuse_addr_val = 1;
   struct sockaddr_in server_address;

   //inicjalizacja gniazda serwera
   
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(server_port);

   server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket_descriptor < 0)
   {
       fprintf(stderr, "%s: Cannot open a socket.\n", argv[0]);
       exit(1);
   }
   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (bind_result < 0)
   {
       fprintf(stderr, "%s: Binding error.\n", argv[0]);
       exit(1);
   }
	printf("Server started.\n");
   listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
   if (listen_result < 0) {
       fprintf(stderr, "%s: Setting queue size error\n", argv[0]);
       exit(1);
   }

	
   	while(1) {
       		connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
       		if (connection_socket_descriptor < 0) {
           		fprintf(stderr, "%s: Cannot set socket to new connection\n", argv[0]);
           		exit(1);
       		}

       	handleConnection(connection_socket_descriptor);
	}
	printf("Shutting server down...\n");
   	close(server_socket_descriptor);
   	return(0);
}
