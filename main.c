/* 
	Simple telnet server.
	Maciej Kasprzyk inf138575
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
#define BUF 1024

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
	//TODO
	int new_socket_descriptor;
	int status;
	int time;
};

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    //dostęp do pól struktury: (*th_data).pole
    //TODO (przy zadaniu 1) klawiatura -> wysyłanie albo odbieranie -> wyświetlanie
    char command[BUF];
    while (1) {	
	    read((*th_data).new_socket_descriptor, command, BUF);
	    printf("%s", command);
	    printf("Command executed\n");
	    system(command);
	    write ((*th_data).new_socket_descriptor, "Executed\n", 12);

    }
    close((*th_data).new_socket_descriptor);
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor) {

    printf("New connection established, socket id:%d\n", connection_socket_descriptor);

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
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }

    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
    //
    char server_response[BUF];
    int len = 0;
    while (1) {
	    memset(server_response, 0, BUF);
	    fgets(server_response, BUF, stdin);
	    len = strlen(server_response);
	    write (connection_socket_descriptor, server_response, len);
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
       fprintf(stderr, "%s: Cannot set socket.\n", argv[0]);
       exit(1);
   }
   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (bind_result < 0)
   {
       fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
       exit(1);
   }
	printf("Server started.\n");
   listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
   if (listen_result < 0) {
       fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
       exit(1);
   }

   while(1)
   {
       connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
       if (connection_socket_descriptor < 0)
       {
           fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
           exit(1);
       }

       handleConnection(connection_socket_descriptor);
   }

   close(server_socket_descriptor);
   return(0);
}
