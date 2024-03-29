#include <curl/curl.h>
#include <json-c/json.h>
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

  printf("%s\n", json_object_get_string(text));

  return 0;
}

char *inp_to_json(char *input) {

  if (input[0] == '\0') {
    return NULL;
  }

  input[strlen(input) - 1] = '\0';

  json_object *outer, *contents, *parts, *text;
  json_object *text_arr, *parts_arr;

  text = json_object_new_object();
  json_object_object_add(text, "text", json_object_new_string(input));

  text_arr = json_object_new_array();
  json_object_array_add(text_arr, text);

  parts = json_object_new_object();
  json_object_object_add(parts, "parts", text_arr);

  parts_arr = json_object_new_array();
  json_object_array_add(parts_arr, parts);

  contents = json_object_new_object();
  json_object_object_add(contents, "contents", parts_arr);

  const char *contents_str = json_object_get_string(contents);

  char *response = NULL;
  size_t len = strlen(contents_str + 1);
  response = malloc(len * sizeof(char));

  if (response == NULL) {
    return NULL;
  }

  strcpy(response, contents_str);

  return response;
}

int main(void) {
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *handle = curl_easy_init();
  CURLcode res;

  const char *s = getenv("URL");

  if (handle) {
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(handle, CURLOPT_URL, s);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hs);
  } 

  while (true) {
    printf("ASK GEMINI:\n");
    char *prompt = NULL;
    prompt = read_line();
    char *json = NULL;
    json = inp_to_json(prompt);

    struct memory response = {0};

    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, json);
    res = curl_easy_perform(handle);

    if (res != CURLE_OK)
      printf("ERROR");

    if (response.response) {
      process_resp(&response);
    }
  }

  curl_easy_cleanup(handle);
  return 0;
}
