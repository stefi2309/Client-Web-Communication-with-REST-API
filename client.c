#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"
#include "json_helpers.h"
#include "parson.h"
#include "requests.h"

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080

// functie pt realizarea operatiilor de retea
void perform_network_operation(char *request, char **response,
                               char **response_code, int *sockfd) {
  // deschide conexiunea la server
  *sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
  // trimite request-ul la server
  send_to_server(*sockfd, request);
  // primeste raspunsul de la server
  *response = receive_from_server(*sockfd);
  // extrage codul de raspuns HTTP din raspunsul primit
  *response_code = calloc(3, sizeof(char *));
  memccpy(*response_code, strchr(*response, ' ') + 1, ' ', 3);
}

// functie pt inregistrarea unui nou utilizator
void register_(char **message, char **response, char **response_code,
               char ***cookies, int *cookies_count, int *sockfd) {
  if (!*cookies_count) {
    char *username = calloc(64, sizeof(char *));
    printf("username=");
    fgets(username, 64, stdin);
    // elimina newline-ul de la sfarsit
    if (strchr(username, '\n')) *strchr(username, '\n') = '\0';

    char *password = calloc(64, sizeof(char *));
    printf("password=");
    fgets(password, 64, stdin);
    if (strchr(password, '\n')) *strchr(password, '\n') = '\0';

    // verifica daca username-ul sau parola contin spatii
    if (strchr(username, ' ') || strchr(password, ' ')) {
      free(username);
      free(password);
      printf("ERROR - Username or password contains spaces!\n\n");
      return;
    }
    // creeaza payload-ul JSON pt inregistrare
    char *json_payload = json_payload_register_login(username, password);
    // creeaza si trimite request-ul de inregistrare
    *message =
        compute_post_request(SERVER_IP, "/api/v1/tema/auth/register",
                             "application/json", json_payload, NULL, NULL, 0);
    // executa operatia de retea si primeste raspunsul
    perform_network_operation(*message, response, response_code, sockfd);

    // verifica raspunsul primit de la server
    if (strcmp(*response_code, "400") == 0) {
      printf("ERROR %s - Username %s already taken!\n\n", *response_code,
             username);
    } else {
      printf("Username %s successfully registered!\n\n", username);
    }
    free(username);
    free(password);
    json_free_serialized_string(json_payload);
  } else {
    printf("ERROR - Already logged in!\n\n");
    return;
  }
}

// functie pt logarea unui utilizator
// asemanatoare cu register
void login(char **message, char **response, char **response_code,
           char ***cookies, int *cookies_count, int *sockfd) {
  // verifica daca utilizatorul este deja logat
  if (!*cookies_count) {
    char *username = calloc(64, sizeof(char *));
    printf("username=");
    fgets(username, 64, stdin);
    if (strchr(username, '\n')) *strchr(username, '\n') = '\0';

    char *password = calloc(64, sizeof(char *));
    printf("password=");
    fgets(password, 64, stdin);
    if (strchr(password, '\n')) *strchr(password, '\n') = '\0';

    if (strchr(username, ' ') || strchr(password, ' ')) {
      printf("ERROR - Username or password contains spaces!\n\n");
      free(username);
      free(password);
      return;
    }

    char *json_payload = json_payload_register_login(username, password);
    *message =
        compute_post_request(SERVER_IP, "/api/v1/tema/auth/login",
                             "application/json", json_payload, NULL, NULL, 0);

    perform_network_operation(*message, response, response_code, sockfd);

    // verifica raspunsul primit de la server
    if (strcmp(*response_code, "400")) {
      *cookies = calloc(1, sizeof(char *));
      (*cookies)[0] = calloc(LINELEN, sizeof(char));
      memccpy((*cookies)[0],
              strstr(*response, "Set-Cookie: ") + strlen("Set-Cookie: "), ';',
              256);
      (*cookies)[0][strcspn((*cookies)[0], ";")] = 0;
      (*cookies_count)++;
      printf("SUCCESS - Login successful for username %s!\n\n", username);
    } else {
      printf("ERROR %s - Wrong credentials for username %s!\n\n",
             *response_code, username);
    }
    json_free_serialized_string(json_payload);
    free(username);
    free(password);
  } else {
    printf("ERROR - Already logged in!\n\n");
    return;
  }
}

// functie pt accesarea bibliotecii
void enter_library(char **message, char **response, char **response_code,
                   char **token, char **cookies, int cookies_count,
                   int *sockfd) {
  // verifica daca utilizatorul are deja un token de acces
  if (!*token) {
    *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/access",
                                   NULL, NULL, cookies, cookies_count);
    perform_network_operation(*message, response, response_code, sockfd);

    // daca nu este autorizat extrage tokenul din raspunsul JSON
    if (strcmp(*response_code, "401")) {
      json_token(*response, token);
      if (*token) {
        printf("SUCCESS - Library access granted!\n\n");
      } else {
        printf("ERROR - Failed to extract token.\n\n");
      }
    } else {
      printf("ERROR %s - You are not logged in!\n\n", *response_code);
    }
  } else {
    printf("ERROR - You already have access to the library!\n\n");
  }
}

// functie pt obtinerea listei de carti
void get_books(char **message, char **response, char **response_code,
               char *token, int *sockfd) {
  *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/books", NULL,
                                 token, NULL, 0);
  perform_network_operation(*message, response, response_code, sockfd);

  // verifica daca accesul este permis
  if (strcmp(*response_code, "403") == 0) {
    printf("ERROR %s - You don't have access to the library!\n\n",
           *response_code);
  } else {
    // verifica daca lista de carti este goala
    if (strcmp(strchr(*response, '['), "[]") == 0) {
      printf("Your book list is empty!\n\n");
    } else {
      // daca lista nu este goala, afiseaza cartile
      json_display_books_from_response(*response);
    }
  }
}

// functie pt obtinerea detaliilor unei carti
void get_book(char **message, char **response, char **response_code,
              char *token, int *sockfd) {
  char *id = calloc(16, sizeof(char *));
  printf("id=");
  fgets(id, 16, stdin);
  if (strchr(id, '\n')) *strchr(id, '\n') = '\0';

  // verifica daca ID-ul introdus este numeric
  int ok = 1;
  for (int i = 0; i < strlen(id); i++) {
    if (!isdigit(id[i])) {
      ok = 0;
    }
  }
  if (ok == 0) {
    printf("ERROR - Please enter a valid number!\n\n");
    free(id);
    return;
  }

  char *url = calloc(LINELEN, sizeof(char *));
  // creeaza URL-ul pt request
  sprintf(url, "/api/v1/tema/library/books/%s", id);

  *message = compute_get_request(SERVER_IP, url, NULL, token, NULL, 0);

  perform_network_operation(*message, response, response_code, sockfd);

  // verifica daca accesul este permis
  if (strcmp(*response_code, "403") == 0) {
    printf("ERROR %s - You don't have access to the library!\n\n",
           *response_code);
  }
  // verifica daca exista cartea
  else if (strcmp(*response_code, "404") == 0) {
    printf("ERROR %s - The book with id %s was not found!\n\n", *response_code,
           id);
  } else {
    // daca exista cartea, afiseaza detaliile acesteia
    json_display_book_details(*response);
  }

  free(url);
  free(id);
}

// functie pt stergerea unei carti
// asemanatoare cu get_book
void delete_book(char **message, char **response, char **response_code,
                 char *token, int *sockfd) {
  char *id = calloc(16, sizeof(char *));
  printf("id=");
  fgets(id, 16, stdin);
  if (strchr(id, '\n')) *strchr(id, '\n') = '\0';

  // verifica daca ID-ul introdus este numeric
  int ok = 1;
  for (int i = 0; i < strlen(id); i++) {
    if (!isdigit(id[i])) {
      ok = 0;
    }
  }
  if (ok == 0) {
    printf("ERROR - Please enter a valid number!\n\n");
    free(id);
    return;
  }

  char *url = calloc(LINELEN, sizeof(char *));
  sprintf(url, "/api/v1/tema/library/books/%s", id);

  *message = compute_delete_request(SERVER_IP, url, NULL, token, NULL, 0);

  perform_network_operation(*message, response, response_code, sockfd);

  if (strcmp(*response_code, "403") == 0) {
    printf("ERROR %s - You don't have access to the library!\n\n",
           *response_code);
  } else if (strcmp(*response_code, "404") == 0) {
    printf("ERROR %s - The book with id %s was not found!\n\n", *response_code,
           id);
  } else {
    printf("SUCCESS - The book with id %s was deleted successfully!\n\n", id);
  }

  free(url);
  free(id);
}

// functie pt adaugarea unei carti noi
void add_book(char **message, char **response, char **response_code,
              char *token, int *sockfd) {
  char *title = calloc(128, sizeof(char *));
  printf("title=");
  fgets(title, 128, stdin);
  if (strchr(title, '\n')) *strchr(title, '\n') = '\0';

  char *author = calloc(128, sizeof(char *));
  printf("author=");
  fgets(author, 128, stdin);
  if (strchr(author, '\n')) *strchr(author, '\n') = '\0';

  char *publisher = calloc(128, sizeof(char *));
  printf("publisher=");
  fgets(publisher, 128, stdin);
  if (strchr(publisher, '\n')) *strchr(publisher, '\n') = '\0';

  char *genre = calloc(128, sizeof(char *));
  printf("genre=");
  fgets(genre, 128, stdin);
  if (strchr(genre, '\n')) *strchr(genre, '\n') = '\0';

  char *page_count = calloc(16, sizeof(char *));
  printf("page_count=");
  fgets(page_count, 16, stdin);
  if (strchr(page_count, '\n')) *strchr(page_count, '\n') = '\0';

  // verifica daca nr de pagini introdus este numeric
  int ok = 1;
  for (int i = 0; i < strlen(page_count); i++) {
    if (!isdigit(page_count[i])) {
      ok = 0;
    }
  }

  // verifica daca toate campurile necesare au fost completate corect
  if (!strlen(title) || !strlen(author) || !strlen(publisher) ||
      !strlen(genre) || !strlen(page_count) || ok == 0) {
    if (ok == 0) {
      printf("ERROR - Please enter a valid number!\n\n");
    } else {
      printf("ERROR - All book fields are mandatory!\n\n");
    }
    free(title);
    free(author);
    free(publisher);
    free(genre);
    free(page_count);
    return;
  }

  // creeaza payload-ul JSON pt adaugarea cartii
  char *json_payload =
      json_payload_book(title, author, publisher, genre, page_count);

  *message =
      compute_post_request(SERVER_IP, "/api/v1/tema/library/books",
                           "application/json", json_payload, token, NULL, 0);
  perform_network_operation(*message, response, response_code, sockfd);

  json_free_serialized_string(json_payload);

  if (strcmp(*response_code, "403") == 0) {
    printf("ERROR %s - You don't have access to the library!\n\n",
           *response_code);
  } else {
    printf("SUCCESS - Book successfully added!\n\n");
  }

  free(title);
  free(author);
  free(publisher);
  free(genre);
  free(page_count);
}

// functie pt deconectarea utilizatorului
void logout(char **message, char **response, char **response_code, char **token,
            char ***cookies, int *cookies_count, int *sockfd) {
  *message = compute_get_request(SERVER_IP, "/api/v1/tema/auth/logout", NULL,
                                 NULL, *cookies, *cookies_count);
  perform_network_operation(*message, response, response_code, sockfd);

  // verifica daca deconectarea a fost efectuata cu succes
  if (strcmp(*response_code, "400")) {
    // sterge token-ul de acces
    free(*token);
    *token = NULL;
    // sterge cookie-urile
    free((*cookies)[0]);
    free(*cookies);
    *cookies = NULL;
    // reseteaza nr de cookie-uri
    *cookies_count = 0;
    printf("SUCCESS - Logout successful!\n\n");
  } else {
    printf("ERROR %s - You are not logged in!\n\n", *response_code);
  }
}

int main(int argc, char *argv[]) {
  char **cookies = NULL;
  int cookies_count = 0;
  char *token = NULL;

  char *command = calloc(32, sizeof(char *));
  while (strcmp(fgets(command, 32, stdin), "exit\n")) {
    int command_executed = 0;
    int sockfd = 0;
    char *message = NULL;
    char *response = NULL;
    char *response_code = NULL;
    // proceseaza diferite comenzi de la utilizator
    if (strcmp(command, "register\n") == 0) {
      register_(&message, &response, &response_code, &cookies, &cookies_count,
                &sockfd);
      command_executed++;
    }

    if (strcmp(command, "login\n") == 0) {
      login(&message, &response, &response_code, &cookies, &cookies_count,
            &sockfd);
      command_executed++;
    }

    if (strcmp(command, "enter_library\n") == 0) {
      enter_library(&message, &response, &response_code, &token, cookies,
                    cookies_count, &sockfd);
      command_executed++;
    }

    if (strcmp(command, "get_books\n") == 0) {
      get_books(&message, &response, &response_code, token, &sockfd);
      command_executed++;
    }

    if (strcmp(command, "get_book\n") == 0) {
      get_book(&message, &response, &response_code, token, &sockfd);
      command_executed++;
    }

    if (strcmp(command, "add_book\n") == 0) {
      add_book(&message, &response, &response_code, token, &sockfd);
      command_executed++;
    }

    if (strcmp(command, "delete_book\n") == 0) {
      delete_book(&message, &response, &response_code, token, &sockfd);
      command_executed++;
    }

    if (strcmp(command, "logout\n") == 0) {
      logout(&message, &response, &response_code, &token, &cookies,
             &cookies_count, &sockfd);
      command_executed++;
    }

    if (!command_executed) {
      printf("ERROR - Invalid command! List of available commands:\n");
      printf(
          "- register\n- login\n- enter_library\n- get_books\n- get_book\n- "
          "add_book\n- delete_book\n- logout\n- exit\n\n");
    } else if (sockfd) {
      free(message);
      free(response);
      free(response_code);
      // inchide socket-ul
      close(sockfd);
    }
  }

  free(command);
  return 0;
}