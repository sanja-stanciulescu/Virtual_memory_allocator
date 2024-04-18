###Ana Stanciulescu, 312CA, 07.04.2024

#####Segregated Free List - Tema 1#####

###Descriere###

  Tema consta intr-un vector de liste dublu inlantuite, care simuleaza heap-ul cu zonele de memorie libere
si o alta lista dublu inlantuita, care gestioneaza zonele alocate de memorie.

  Exista 7 functii principale, care ajuta in crearea heap-ului, gestionarea zonelor de memorie prin malloc() si free(),
stocarea de informatie si afisarea ei in zone alocate prin read() si write(), afisarea infromatiilor privind memoria prin dump_memory()
si eliberarea heap-ului prin destroy_heap().

  Pentru a crea aceste functii, au fost necesare si niste subprograme generice, care lucreaza cu liste si manipuleaza
blocurile in functie de tipul listei (sfl = segregated free list si aml = allocated-memory list). In principal, m-au 
preocupat adaugarea blocurilor, eliminarea lor si lucrul cu adresele asociate fiecaruia.


###Comentarii asupra temei###

  #Crezi ca ai fi putut realiza o implementare mai buna?
Atunci cand realizez temele de programare, incerc sa implemetez in cod gandirea propria mea gandire.
Din acest motiv, codul nu este intotdeauna optim si sunt sigura ca si pentru tema asta este valabil acest lucru.
Am avut chiar aceasta problema cu prima versiune pe care am scris-o pentru  malloc(). Nu imi treceau ultimele teste
din cauza implementarii. Din fericre, nu a fost foarte mult de modificat in aceasta situatie.

 #Ce ai invatat din realizarea acestei teme?
Inainte de acesta tema nu auzisem de conceptul de Segregated free lists. Pentru a intelege cerinta a trebuit sa
imi aloc jumatate de zi sa studiez mai bine despre alocatoarele de memorie si am aflat cum se gestioneaza in realitate heap-ul.
In acelasi timp, scriind cod, mi-am creat o deprindere mai buna in lucrul cu listele (+cum sa lucrez cu ele fara memory leaks).

 #Impresii initiale asupra cerintei
Initial, tema m-a speriat foarte tare ca nivel de dificultate. Nu credeam ca o sa reusesc vreodata sa o termin si, mai ales,
nu in timp util.
Pana semestrul acesta, nu mai lucrasem cu liste si diferenta dintre tema si exercitiile din laboratoare este destul de mare.
Din fericire pentru mine, am inteles mai bine ce am de facut multumita laboratorului 4.
Acum ca am tema terminata, sunt mandra de mine ca toata munca mea din ultima saptamana a dat roade.

