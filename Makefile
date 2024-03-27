main: main.c
	gcc -Wall -Wextra -pedantic -ggdb main.c -lcurl -ljson-c -o main
