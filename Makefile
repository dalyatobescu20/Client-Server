build: server.c subscriber.c
	gcc -g -Wall server.c -o server -lm
	gcc -g -Wall subscriber.c -o subscriber -lm
clean:
	gcc -g -Wall server.c -o server
	gcc -g -Wall subscriber.c -o subscriber
	rm server subscriber
