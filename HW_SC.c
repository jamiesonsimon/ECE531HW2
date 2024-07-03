// Included libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Define flags
#define POST_flag "--post"
#define POST_flag_short "-o"
#define PUT_flag "--put"
#define PUT_flag_short "-p"
#define DELETE_flag "--delete"
#define DELETE_flag_short "-d"
#define GET_flag "--get"
#define GET_flag_short "-g"
#define URL_flag "--url"
#define URL_flag_short "-u"
#define HELP_flag "--help"
#define HELP_flag_short "-h"

// Return flags
#define OK 0
#define INIT_ERR 1
#define REQ_ERR 2

// Callback function to handle the response
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *data = (char *)contents;
    data[realsize] = '\0'; // Null-terminate the response
    printf("Response: %s\n", data);
    return realsize;
}

// Function to print CLI information about the program
void cli_info(const char *hw2) {
    printf("Usage: %s <method> --url <URL> [<message>]\n", hw2);
    printf("Methods:\n");
    printf("  %s, %s    HTTP POST\n", POST_flag, POST_flag_short);
    printf("  %s, %s    HTTP PUT\n", PUT_flag, PUT_flag_short);
    printf("  %s, %s    HTTP DELETE\n", DELETE_flag, DELETE_flag_short);
    printf("  %s, %s    HTTP GET\n", GET_flag, GET_flag_short);
    printf("Flags:\n");
    printf("  %s, %s   URL to send request to\n", URL_flag, URL_flag_short);
    printf("  %s, %s   Print this help message\n", HELP_flag, HELP_flag_short);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cli_info(argv[0]);
        return INIT_ERR;
    }

    const char *url = NULL;
    const char *data = NULL;
    const char *method = NULL;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], HELP_flag) == 0 || strcmp(argv[i], HELP_flag_short) == 0) {
            cli_info(argv[0]);
            return OK;
        } else if ((strcmp(argv[i], URL_flag) == 0 || strcmp(argv[i], URL_flag_short) == 0) && i + 1 < argc) {
            url = argv[i + 1];
            if (i + 2 < argc) {
                data = argv[i + 2]; // Capture the message if provided
            }
            i += 2; // Skip processed arguments
        } else if (strcmp(argv[i], POST_flag) == 0 || strcmp(argv[i], POST_flag_short) == 0) {
            method = "POST";
        } else if (strcmp(argv[i], PUT_flag) == 0 || strcmp(argv[i], PUT_flag_short) == 0) {
            method = "PUT";
        } else if (strcmp(argv[i], DELETE_flag) == 0 || strcmp(argv[i], DELETE_flag_short) == 0) {
            method = "DELETE";
        } else if (strcmp(argv[i], GET_flag) == 0 || strcmp(argv[i], GET_flag_short) == 0) {
            method = "GET";
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            cli_info(argv[0]);
            return INIT_ERR;
        }
    }

    if (!url || !method) {
        fprintf(stderr, "Error: URL and method are required.\n");
        cli_info(argv[0]);
        return INIT_ERR;
    }

    CURL *curl;
    CURLcode res;

    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the HTTP method
        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (data) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            }
        } else if (strcmp(method, "PUT") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            if (data) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            }
        } else if (strcmp(method, "DELETE") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            if (data) {
                char *full_url = malloc(strlen(url) + strlen(data) + 2);
                sprintf(full_url, "%s/%s", url, data);
                curl_easy_setopt(curl, CURLOPT_URL, full_url);
                free(full_url);
            }
        } else if (strcmp(method, "GET") == 0) {
            // GET is the default method; no special options needed
        }

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return REQ_ERR;
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Global cleanup
    curl_global_cleanup();
    return OK;
}

