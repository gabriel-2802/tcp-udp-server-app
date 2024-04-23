Pentru a vă testa mai ușor tema 2, v-am pus la dispoziție printre resursele temei un script de testare [1], pe care îl vom folosi și noi la evaluarea temei. Script-ul necesită Python 3 pe Linux și se rulează cu sudo astfel:

$ sudo python3 test.py

Înainte de a rula scriptul, vă recomandăm să citiți cu atenție enunțul temei și să vă asigurați că respectați toate cerințele legate de formatarea afișărilor, buffering de output, Makefile, sau comportament general al temei voastre. De asemenea, este necesar să aveți clientul UDP de test, pe care îl găsiți tot printre resursele pentru temă.

Scriptul rulează următoarele teste:

1. compile --> se compilează cele două executabile
2. server_start --> se pornește serverul (portul implicit este 12345, dar se poate schimba prin modificarea variabilei port de la linia 15 din script)
3. c1_start --> se pornește un subscriber C1; dacă testul pică, se trece direct la pasul 19
4. data_unsubscribed --> se generează câte un mesaj pe fiecare topic, folosind clientul UDP de test; subscriber-ul TCP trebuie să nu primească nimic; clientul UDP de test trebuie să se afle implicit într-un director numit pcom_hw2_udp_client, dar acest lucru se poate schimba prin modificarea variabilei udp_client_path de la linia 21 din script
5. c1_subscribe_all --> se abonează C1 la toate topic-urile
6. data_subscribed --> se generează câte un mesaj pe fiecare topic, pe care C1 trebuie să le primească și afișeze corect
7. c1_stop --> se deconectează C1 de la server; dacă testul pică, se trece direct la pasul 18
8. c1_restart --> se generează un mesaj și apoi se reconectează C1; dacă testul pică, se trece direct la pasul 18
9. data_no_clients --> se verifică faptul că C1 nu primește mesajul generat cât era deconectat
10. same_id --> se încearcă pornirea și conectarea unui al doilea subscriber cu același ID
11. c2_start --> se pornește un subscriber C2; dacă testul pică, se trece direct la pasul 18
12. c2_subscribe --> se abonează C2 la un topic
13. c2_stop --> se deconectează C2 de la server; dacă testul pică, se trece direct la pasul 18
14. c2_subscribe_plus_wildcard --> se abonează C2 la topicuri folosind wildcard-ul "+"
15. c2_subscribe_star_wildcard --> se abonează C2 la topicuri folosind wildcard-ul "*"
16. c2_subscribe_compound_wildcard --> se abonează C2 la topicuri folosind ambele tipuri de wildcards
17. c2_subscribe_wildcard_set_inclusion --> se abonează C2 la topicuri care se includ, folosind wildcards (C2 abonat la A, C2 abonat la B, B inclus în A); C2 nu trebuie să primească mesaje duplicate (și de la A și de la B)
18. quick_flow --> se generează toate tipurile de mesaje de 30 ori și se trimit în succesiune rapidă, verificându-se că toate ajung în mod corect la C1
19. server_stop --> se oprește serverul.

Atenție! Script-ul nu vă va da nota finală. Temele vor fi testate și prin mijloace externe, pentru a se verifica respectarea tuturor cerințelor temei și eventual depunctări conform baremului din enunțul temei.