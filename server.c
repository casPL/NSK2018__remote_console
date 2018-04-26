/* 
 *	Network console server
 *	inf138575
 *	PUT NSK2018
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
#define HELLO_MSG "Connection established\nEnter password: "
#define GOODBYE_MSG "Bye... \n"
#define ACCESS_GRANTED_MSG  "Welcome!\n" 
#define ACCESS_DENIED_MSG "ACCESS DENIED\n"
#define STDERR_POSTFIX " 2>&1"
#define MAX_LOGIN_NAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

int GLOBAL_ID = 0;

int auth (char * password, char * login_name, int session_id) {
	printf("[ID%d] Authentincation process started with password: %s\n",session_id, password);
	char buf1[MAX_LOGIN_NAME_LENGTH];
	char buf2[MAX_LOGIN_NAME_LENGTH];
	FILE * cred_file;
	cred_file = fopen("credentials.txt", "r");
	if (cred_file == NULL) {
		printf("Error. Could not open credentials file.\n");
		exit(2);
	}
	printf("[ID%d] Scanning credentials file ",session_id);
	while(fscanf(cred_file, "%s :: %s", buf1, buf2) != EOF) {
		// cool animation section
		printf(".");
		fflush(stdout);
		usleep(600000);
		// sction end
		
		//simply compare password passed by user and password stored in a file
		if (strcmp(password, buf2) == 0) {
			login_name = buf1;
			printf("\n[ID%d] Authentincation passed. User name: %s\n",session_id, buf1);
			return 1;
		}
	}
	printf("\n[ID%d] Authentincation process failed. Access denied\n",session_id);
	return 0;

}


//send STDERR with STDOUR
char * add_stderr (char * command, char * execute) {
	strcpy(execute, command); //copy command string to 'execute' string
	strcat(execute, STDERR_POSTFIX); //append stderr postfix
	return execute;
}

struct thread_data_t {
	int new_socket_descriptor;
	int session_id;
	char * login_name;
	char password[MAX_PASSWORD_LENGTH];
};

void *ThreadBehavior(void *t_data) {
	pthread_detach(pthread_self());
	
	//ignore SIGPIPE signal, like CTRL-C
	signal(SIGPIPE, SIG_IGN);

	struct thread_data_t *th_data = (struct thread_data_t*)t_data;
	// access to  structure's fields: (*th_data).field

	GLOBAL_ID++;

	printf("[ID%d] Connection established\n", (*th_data).session_id);
    	char command[BUF_SIZE +1];
    	char reply[BUF_SIZE +1];
   	FILE *output_fd;
   	FILE *input_fd;
    	char *read_result;

	//The fdopen() function  associates  a  stream  with  the  existing  file descriptor,  fd.
       	input_fd = fdopen((*th_data).new_socket_descriptor,"r"); 

	//Auth section
	write((*th_data).new_socket_descriptor, HELLO_MSG, sizeof(HELLO_MSG));
	
	fscanf(input_fd, "%s", (*th_data).password);
	if (auth((*th_data).password, (*th_data).login_name, (*th_data).session_id) ==0) {
		write((*th_data).new_socket_descriptor, ACCESS_DENIED_MSG, sizeof(ACCESS_DENIED_MSG));
		(*th_data).password[0] = 0; //data won't be read by another process
       		fclose(input_fd);
    		close((*th_data).new_socket_descriptor);
		printf("[ID%d] Connection terminated due to failed authentication\n",(*th_data).session_id);
   		free(t_data);	
    		pthread_exit(NULL);
	}
	(*th_data).password[0]= 0; //data won't be read by another process
	write((*th_data).new_socket_descriptor, ACCESS_GRANTED_MSG, sizeof(ACCESS_GRANTED_MSG));  //say hello
	fgets(command, BUF_SIZE, input_fd); //read rest of input_fd
	//section end

	while (1) {

		write((*th_data).new_socket_descriptor, PROMPT, sizeof(PROMPT));
            	read_result = fgets(command, BUF_SIZE, input_fd); //fgets() reads until new line or EOF
		if (read_result == NULL) {
                	printf("[ID%d] Error while reading command\n",(*th_data).session_id);
                	break;
		}
		read_result = strchr(command,'\n'); //look for new line character, if not found break the while loop
		if (read_result == NULL) {
                	printf("[ID%d] Could not find new line charcter\n",(*th_data).session_id);
              	 	break;
		}
		else
			*read_result = 0;

		printf("[ID%d] Received command '%s'\n",(*th_data).session_id,  command);

            	if (strcmp(command,"exit") == 0 || strcmp(command, "quit") == 0) { 
                	printf("[ID%d] The client is closing connection\n",(*th_data).session_id);
			write((*th_data).new_socket_descriptor, GOODBYE_MSG, sizeof(GOODBYE_MSG));
			break;
		}
		

		char * execute = malloc(strlen(command)+strlen(STDERR_POSTFIX)-1);  //works better with -1
            	output_fd = popen(add_stderr(command, execute), "r"); //execute command and direct output to output_fd
            	free(execute); //free memory after execution


		while (1) {
                		read_result = fgets(reply, BUF_SIZE, output_fd);  //read from file descriptor 
                		if (read_result == NULL) 
                    			break;
                		write((*th_data).new_socket_descriptor, reply, strlen(reply)); //send it via socket
            	}
            	pclose(output_fd);  //close file descriptor
        }
        fclose(input_fd); //close input file descriptor
    	close((*th_data).new_socket_descriptor); //close socket
	printf("[ID%d] Connection terminated\n",(*th_data).session_id);
   	free(t_data);	//free memory
    	pthread_exit(NULL);	 //close thread
}

//handling connection with new client
void handleConnection(int connection_socket_descriptor) {

    //result of creating new therad
    int create_result = 0;

    //thread descriptor
    pthread_t thread1;

    //structure that will be passed to new thread
    struct thread_data_t * t_data;
    t_data = malloc(sizeof(struct thread_data_t));
    t_data->new_socket_descriptor = connection_socket_descriptor;
    t_data->session_id =GLOBAL_ID;
    
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);  //create pthread
    if (create_result){
       printf("Pthread error, code : %d\n", create_result);
       exit(-1);
    }
}

int main(int argc, char* argv[])
{
	//safe start section
	if ( argc <= 1) {
		printf("Bad syntax. Proper usage: ./server [port_number]\n");
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

   	//initializing server socket
   
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
   	if (bind_result < 0) {
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
