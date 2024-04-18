###Ana Stanciulescu, 312CA, 07.04.2024

#####Segregated Free List#####

###Descriere###

  Tema consta intr-un vector de liste dublu inlantuite, care simuleaza heap-ul cu zonele de memorie libere
si o alta lista dublu inlantuita, care gestioneaza zonele alocate de memorie.

  Exista 7 functii principale, care ajuta in crearea heap-ului, gestionarea zonelor de memorie prin malloc() si free(),
stocarea de informatie si afisarea ei in zone alocate prin read() si write(), afisarea infromatiilor privind memoria prin dump_memory()
si eliberarea heap-ului prin destroy_heap().

  Pentru a crea aceste functii, au fost necesare si niste subprograme generice, care lucreaza cu liste si manipuleaza
blocurile in functie de tipul listei (sfl = segregated free list si aml = allocated-memory list). In principal, m-au 
preocupat adaugarea blocurilor, eliminarea lor si lucrul cu adresele asociate fiecaruia.

  Alocatorul de memorie este unul virtual, deoarece doar simuleaza folosirea unor adrese din heap. In realitate, functiile mele de free(), malloc(), read() si write() folosesc la randul lor functiile oficiale care au exact acelasi scop.
  Scopul principal al acestui proiect a fost aprofundarea cunostintelor legate de structuri de date si felul in care poate fi structurata memoria heap.

