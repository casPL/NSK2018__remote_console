all:
	gcc main.c -pthread -o server
	gcc client.c -pthread -o client
