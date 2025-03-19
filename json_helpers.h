
// functia genereaza un payload JSON pentru operatiile de inregistrare si
// autentificare
char *json_payload_register_login(char *username, char *password);

// functia genereaza un payload JSON pentru adaugarea unei carti in biblioteca
char *json_payload_book(const char *title, const char *author,
                        const char *publisher, const char *genre,
                        const char *page_count);

// functia afiseaza detaliile unei carti dintr-un raspuns JSON
void json_display_book_details(char *json_response);

// functia afiseaza detaliile unei carti dintr-un raspuns JSON
void json_display_books_from_response(const char *json_response);

// functia extrage si stocheaza token-ul de acces dintr-un raspuns JSON
void json_token(const char *json_response, char **token);