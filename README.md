Ivana Stefania 321CD

Tema 4. Client Web. Comunicatie cu REST API.

Am folosit 2 sleep days pentru aceasta tema.

Am pornit cu implementarea temei de la scheletul laboratorului 9, modificandu-l in functie de cerinta.
Am adaugat in request.c si request.h functia compute_delete_request pt comanda delete_books.

Pentru parsarea JSON am folosit parson (https://github.com/nlohmann/json) care permite cu usurinta sa creez payload-uri JSON cat si sa parsez un JSON primit de la server. Pentru toate blocurile de cod care folosesc JSON am implementat functii separate adaugate in json_helpers.c si json_helpers.h.

Structura:
- conexiunea de retea: gestioneaza comunicare intre client si server
- autentificare si inregistrare: permite utilizatorilor sa se inregistreze si sa se autentifice in sistem
- gestionarea cartilor: permite utilizatorilor sa adauge, sa stearga si sa vizualizeze cartile din biblioteca
- sesiune si cookies: gestioneaza sesiunile utilizatorilor prin cookies si tokenuri de acces

Pentru a avea o structura usor de urmarit in client.c am implementat functii pt fiecare comanda si in plus am implememtat si functia perform_network_operation pt trimiterea cererilor la server si primirea raspunsurilor, extragand si codul de status HTTP.

Comenzi implementate (pt fiecare functie am tratat cazurile in care se returneaza o eroare in functie de cerinta):
- register_ : permite unui nou utilizator sa se inregistreze si verifica daca username-ul si parola contin spatii, trimite datele la server(ERORI: username sau parola contin spatii, username deja luat, erori de conexiune sau retea)
- login : autentifica un utilizator, are concepte asemanatoare cu register_, in plus verifica si seteaza cookie-urile de sesiune(ERORI: username sau parola contin spatii, credentiale incorecte, utilizator deja logat, erori de conexiune sau retea)
- enter_library : verifica daca utilizatorul are acces la biblioteca si extrage un token de acces din raspunsul serverului(ERORI: lipsa accesului, esec la extragerea token-ului, utilizatorul are deja acces la biblioteca)
- get_books: afiseaza lista de carti disponibile(ERORI: acces neautorizat, lista de carti este goala)
- get_book: afiseaza detaliile unei carti specificare(ERORI: id invalid, acces neautorizat, cartea nu exista)
- delete_book: sterge o carte existenta(ERORI: id invalid, acces neautorizat, cartea nu exista)
- add_book: adauga o carte noua in biblioteca(ERORI: campuri obligatorii lipsa, nr de pagini invalid, acces neautorizat)
- logout: deconecteaza utilizatorul si curata sesiunea si cookie-urile(ERORI: esec la deconectare, utilizatorul nu este logat)
- exit: inchide aplicatia







