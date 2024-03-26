#include <curl/curl.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_line() {
  char *buffer = malloc(sizeof *buffer);
  int read;
  unsigned long int size;

  read = getline(&buffer, &size, stdin);
  if (*"\n" != *buffer) {
    return buffer;
  } else {
    printf("No line read..\n");
    return "No Input Read";
  }
}

struct memory {
  char *response;
  size_t size;
};

static size_t callback(void *data, size_t size, size_t nmemb, void *clientp) {
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0; /* out of memory! */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

int process_resp(struct memory *response) {
  json_object *json_obj = NULL;
  json_object *candidates, *content, *part, *text;
  int exists;


  json_obj = json_tokener_parse(response->response);
  candidates = json_object_object_get(json_obj, "candidates");

	json_object *can_obj = json_object_array_get_idx(candidates, 0);

  exists = json_object_object_get_ex(can_obj, "content", &content);
  if (exists == false) {
    printf("key \"channel\" not found in JSON");
    return 1;
  }
  exists = json_object_object_get_ex(content, "parts", &part);
  if (exists == false) {
    printf("key \"item\" not found in JSON");
    return 1;
  }


	json_object *part_obj = json_object_array_get_idx(part, 0);

  exists = json_object_object_get_ex(part_obj, "text", &text);
  if (exists == false) {
    printf("key \"condition\" not found in JSON");
    return 1;
  }

	printf("%s",json_object_get_string(text));


  return 0;
}

int main(void) {
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *handle = curl_easy_init();
  CURLcode res;

  char *json = "{\
		\"contents\": [{\
				\"parts\":[{\
						\"text\": \"Write a story about a magic backpack.\"\
				}]\
		}]\
	}";

  const char *s = getenv("URL");
  printf("ASK GEMINI:\n");
  char *prompt = NULL;
  prompt = read_line();
  struct memory response = {0};

  if (handle) {
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(handle, CURLOPT_URL, s);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, json);

    res = curl_easy_perform(handle);

    if (res != CURLE_OK)
      printf("ERROR");

    if (response.response) {
      process_resp(&response);
    }

    curl_easy_cleanup(handle);
  }

  free(prompt);
  return 0;
}
