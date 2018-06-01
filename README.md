# NSK2018__remote_console

Simple academic project.
Remote console: server + client 

Compilation:
make

Usage:

./server [port number]

./client [host name] [port number]



Known bugs:
1. memory leaks in add_stderr() 
2. improper SIGPIPE handling in specific circumstances
3. not resistant to message fragmentation and other network malfunctions
