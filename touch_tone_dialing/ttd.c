#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memory {
  char *data;
  size_t size;
} memory_t;

// Function to store http response
size_t write_to_buffer(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t rsize = size * nmemb;
  memory_t *mem = userp;

  char *ptr = realloc(mem->data, mem->size + rsize + 1);
  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), contents, rsize);
  mem->size += rsize;
  mem->data[mem->size] = '\0';

  return rsize;
}

// Function to write http reponse into file
size_t write_to_file(void *contents, size_t size, size_t nmemb, void *stream) {
  return fwrite(contents, size, nmemb, (FILE *)stream);
}

int main() {
  CURL *curl = NULL;
  cJSON *root = NULL;
  FILE *fp = NULL;
  char *wav_url = NULL;

  const char *base = "https://hackattic.com/challenges/touch_tone_dialing";
  const char *token = getenv("HACKATTIC_TOKEN");

  if (!token) {
    printf("Token not found.\n");
    return 1;
  }

  // Set all global options
  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    return 1;
  }

  curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    return 1;
  }

  char url[512];
  snprintf(url, sizeof(url), "%s/problem?access_token=%s", base, token);

  /* Get the wav file url */
  memory_t response = {0};

  // Set curl request options
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buffer);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  // Perform the curl request
  if (curl_easy_perform(curl) != CURLE_OK) {
    printf("Curl request failed.\n");
    return 1;
  }

  // Extract the .wav file url from response json
  root = cJSON_Parse(response.data);
  cJSON *wav = cJSON_GetObjectItemCaseSensitive(root, "wav_url");
  wav_url = strdup(wav->valuestring);

  printf("wav_url: %s\n", wav_url);

  cJSON_Delete(root);
  root = NULL;
  free(response.data);
  response.data = NULL;

  // Write the .wav file
  fp = fopen("tone.wav", "wb");

  curl_easy_setopt(curl, CURLOPT_URL, wav_url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

  if (curl_easy_perform(curl) != CURLE_OK) {
    printf("Curl request failed.\n");
    return 1;
  }

  fclose(fp);
  fp = NULL;

  // Getting the dialed keys
  fp = popen("dtmf2num -o tone.wav", "r");
  if (!fp) {
    perror("popen");
    return 1;
  }

  // store the keys
  char line[512];
  char sequence[128];

  while (fgets(line, sizeof(line), fp)) {
    fputs(line, stdout);
    if (sscanf(line, "- DTMF numbers:  %s", sequence) == 1) {
      break;
    }
  }

  pclose(fp);

  // Submit solution
  memory_t post_response = {0};
  
  char solution_url[512];
  snprintf(solution_url, sizeof(solution_url), "%s/solve?access_token=%s", base, token);

  root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "sequence", sequence);
  char *json_data = cJSON_Print(root);

  printf("%s\n", json_data);

  curl_easy_setopt(curl, CURLOPT_URL, solution_url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json_data));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buffer);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &post_response);

  if(curl_easy_perform(curl) != CURLE_OK) {
    printf("Post request failed\n");
    return 1;
  }

  printf("Hackattic response: %s", post_response.data);
  free(post_response.data);
  post_response.data = NULL;

  return 0;
}
