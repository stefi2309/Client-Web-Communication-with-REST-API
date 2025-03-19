#include "json_helpers.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"
#include "parson.h"
#include "requests.h"

char *json_payload_register_login(char *username, char *password) {
  // initializarea unui obiect JSON
  JSON_Value *json_value = json_value_init_object();
  // obtinerea referintei la obiectul JSON
  JSON_Object *json_object = json_value_get_object(json_value);

  // setarea username ului si a parolei in obiectul JSON
  json_object_set_string(json_object, "username", username);
  json_object_set_string(json_object, "password", password);

  // serializarea obiectului JSON intr-un sir de caractere formatat frumos
  char *json_payload = json_serialize_to_string_pretty(json_value);

  // eliberarea memoriei pentru obiectul JSON Value, dar păstrarea sirului
  // serializat
  json_value_free(json_value);

  return json_payload;
}

// concepte asemanatoare cu cele din functia anterioara
char *json_payload_book(const char *title, const char *author,
                        const char *publisher, const char *genre,
                        const char *page_count) {
  JSON_Value *json_value = json_value_init_object();
  JSON_Object *json_object = json_value_get_object(json_value);

  // setarea atributelor cartii in obiectul JSON
  json_object_set_string(json_object, "title", title);
  json_object_set_string(json_object, "author", author);
  json_object_set_string(json_object, "publisher", publisher);
  json_object_set_string(json_object, "genre", genre);
  json_object_set_number(json_object, "page_count",
                         atoi(page_count));  // convertim page_count la int

  char *json_payload = json_serialize_to_string_pretty(json_value);
  json_value_free(json_value);

  return json_payload;
}

void json_display_book_details(char *json_response) {
  JSON_Value *json_value = json_parse_string(strchr(json_response, '{'));
  // Parseaza JSON-ul incepand de la prima acolada
  JSON_Object *book = json_value_get_object(json_value);

  // Afisarea detaliilor despre carte
  printf("\nid=%d\n", (int)json_object_get_number(book, "id"));
  printf("title=%s\n", json_object_get_string(book, "title"));
  printf("author=%s\n", json_object_get_string(book, "author"));
  printf("publisher=%s\n", json_object_get_string(book, "publisher"));
  printf("genre=%s\n", json_object_get_string(book, "genre"));
  printf("page_count=%d\n\n", (int)json_object_get_number(book, "page_count"));

  json_value_free(json_value);
}

void json_display_books_from_response(const char *json_response) {
  JSON_Value *json_value = json_parse_string(strchr(json_response, '['));
  // Parseaza JSON-ul incepand de la prima paranteza dreapat
  JSON_Array *books = json_value_get_array(json_value);

  // Parcurgerea array-ului de carti si afisarea detaliilor fiecarei carti
  for (int i = 0; i < json_array_get_count(books); i++) {
    JSON_Object *book = json_array_get_object(books, i);
    printf("\nid=%d\n", (int)json_object_get_number(book, "id"));
    printf("title=%s\n", json_object_get_string(book, "title"));
  }
  printf("\n");

  json_value_free(json_value);
}

void json_token(const char *json_response, char **token) {
  // Parseaza JSON-ul de la prima acolada
  JSON_Value *json_value = json_parse_string(strchr(json_response, '{'));
  JSON_Object *json_object = json_value_get_object(json_value);

  // copiaza token-ul
  int token_size = json_object_get_string_len(json_object, "token");
  *token = calloc(token_size + 1, sizeof(char));
  if (*token) {
    memcpy(*token, json_object_get_string(json_object, "token"), token_size);
    (*token)[token_size] = '\0';
  } else {
    printf("Failed to allocate memory for token.\n");
  }

  json_value_free(json_value);
}