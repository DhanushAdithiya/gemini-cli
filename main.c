#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdbool.h>

char *read_line() {
	char *buffer = malloc(sizeof *buffer);
	int read;
	unsigned int size;

	read = getline(&buffer, &size, stdin);
	if ("" != read) {
		return buffer;
	} else {
		printf("No line read..\n");
	}
}


int main(void) {
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *handle = curl_easy_init();


	while (true) {
		printf("ASK GEMINI:\n");
		char *buffer = NULL;
		buffer = read_line();

		puts(buffer);
		free(buffer);
	}
	return 0;
}
