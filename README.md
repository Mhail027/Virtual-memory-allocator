Name: Necula Mihail\
Group: 313CAa

## Readme Homework 1 - romanian version

### 1. Descrierea programului ###

Fișierul principal care conține rezolvarea temei este cel denumit "sfl.c".
În cadrul său am folosit 2 biblioteci proprii: "list.h" și "block.h". În prima
dintre ele sunt definite 3 structuri: una pentru un nod cu acces în ambele
direcții, alta pentru o lista dublu înlănțuită și ultima pentru un vector de
liste de tipul celui menționat anterior. În următoarea bibliotecă am definit
doar o structura care are scopul de a salva informațiile unui bloc de memorie.
De asemenea, este de menționat faptul că "block.h" face uz de "list.h". Pe
lângă structurile menționate anterior, în "sfl.c" este definită o structură
care contorizează operațiile de alocare virtuale (malloc, free, fragmentare).

În cadrul fișierului principal se poate observa existența a trei variabile
principale:

- adll_free - array of free doubly linking lists - unde se salvează blocurile
de memorie libere
- adll_all - array of allocate doubly linking list - unde se salvează blocurile
de memorie alocate
- all_cnt - counter for the operations of allocations - unde se contorizează
de cate ori s-au realizat virtual operațiile de malloc, free si fragmentare 
(această variabilă conține 3 contoare, unul pentru fiecare operație menționată
anterior)

Pe parcursului rezolvării am respectat mai multe reguli, cu scopul de a îmi
face viața mai ușoară:
1. Un nod conține informația unui bloc de memorie.
2. Blocurile de memorie dintr-o listă trebuie ordonate crescător după adresa
de start.
3. O listă poate conține dor blocuri de memorie de aceeași dimensiune.
4. Listele dintr-un vector trebuie ordonate crescător după lungimea blocurilor
pe care le salvează.

***A. INIT_HEAP***

Vectorul de liste adll_all este creat înainte de această comandă și îi este
pus la dispoziție spațiu pentru 16 liste (numărul 16 a fost ales aleatoriu).
Deocamdată, acest vector nu conține nicio listă și astfel niciun bloc. De
asemenea, înainte de această comandă, și all_cnt este creat și inițializat cu
zero. În momentul in care programul primește comanda "INIT_HEAP", vectorul de
liste adll_free este creat și inițializat cu datele corespunzătoare
informațiilor pe care le primește în continuare. Deci, la final, în interiorul
său vor fi salvate toate blocurile libere / toate blocurile (deoarece nicio
zonă nu este alocată) din heap-ul virtual de care dispunem.

***B. MALLOC***

Îmi caută, în vectorul cu blocuri libere, zona care are cea mai mică adresă și
care are lungimea egală cu cea cerută sau o depășește cu cel mai puțin. În
cazul în care nu este găsit un astfel de bloc, operația de MALLOC nu poate avea
loc. În caz contrar, blocul respectiv este scos din vector. În continuare
există două cazuri:
    
    I. Zona scoasă are aceeași lungime cu blocul de memorie cerut.

		Atunci, zona scoasă o punem în vectorul care salvează blocurile
		alocate,crespectând regulile menționate anterior.

    II. Zona scoasă depășește lungimea blocului de memorie cerut.

    	Atunci, zona este sparta în două astfel încât prima bucată să aibă
		lungimea cerută. Prima parte este băgată în vectorul cu blocuri
		alocate, pe când cea de-a doua este pusă în vectorul cu zone libre.

***C. FREE***

Îmi caută, în vectorul de blocuri alocate, zona care începe cu adresa dată de
utilizator. În cazul in care nu este găsită zona, înseamnă că nu există un
astfel de bloc si operația nu poate avea loc. În caz contrar, trebuie sa
scoatem blocul găsit din vectorul respectiv și să îl adăugăm in acela cu
blocuri libere. Facem asta indiferent de tipul de reconstrucție al heap-ului.
În continuare, există două cazuri:

	I. Tipul de reconstrucție este 0.

		Atunci nu mai trebuie să facem nimic.
   	
	II. Tipul de reconstrucție este 1.
		
		Atunci, folosindu-se de cei doi vectori și de numărul inițial de byți
		per listă cu care s-a creat heap-ul, se determină zona din care a fost
		rupt blocul. Toate blocurile libere din zona determinată se scot și se
		stochează într-un vector temporar de liste. Dacă un bloc in urma unirii
		cu blocul căruia i-am dat free ar forma o zona continuă, se contopește
		cu el. Apoi se verifică daca blocul rezultat nu poate fi unit cu altă
		zona de memorie din vectorul temporar. Acest proces se continua până
		când nu mai poate avea loc nicio fuziune. La sfârșit, toate blocurile
		sunt aduse înapoi în vectorul in care sunt stocate zonele de
		memorie libere.

***D. WRITE***

Se verifică dacă, în zona in care se dorește să se scrie, toți byții sunt
alocați. În cazul in care operația poate avea loc, se parcurge vectorul cu
blocuri alocate. În momentul in care găsim un bloc care face parte, parțial sau
complet, din zonă, se determină:
- un offset, ca să știm de le ce poziție din string-ul primit incepem să luăm
informație să punem în bloc, 
- și numărul de byți / caractere care se vor copia din string în blocul de
memorie virtual.

***E. READ***

Se verifică, dacă în zona in care se dorește să se citească, toți byții sunt
alocați. În cazul in care operația poate avea loc, se parcurge vectorul cu
blocuri alocate. În momentul în care s-a găsit blocul de la începutul zonei,
se citește din el informația corespunzătoare și se reactualizează zona care
trebuie citită. Acest proces continuă pănă cănd zona ajunge să aibă lungimea 0.

***F. DUMP_MEMORY***

Se afișează pe ecran informații despre blocurile libere, zonele alocate și
operațiile virtuale de alocare.

***G. DESTROY_HEAP***

Se eliberează memoria alocată pentru cele trei variabile principale.

### 2. Comentarii asupra temei ###

***Cred că aș fi putut realiza o implementare mai bună?***

Da. Cred că ar fi fost mai eficient să stochez toate blocurile alocate într-o
singură listă decât să le salvez într-un vector de liste. Cel mai mare
dezavantaj al abordării mele este la operația de READ. Acolo, sunt constrâns
să parcurg blocurile alocate de câteva ori. În contrast, dacă aș fi avut o
singură listă în care aveam zonele  alocate ordonate crescător după adresa de
start, aș fi avut nevoie de o singură parcurgere. 

***Ce am învățat din realizarea acestei teme?***

Pe parcursul acestei teme am descoperit o imagine mai amplă a alocării
dinamice. În plus, am devenit mai comfortabil cu programarea generică.


## Readme Homework 1 - english version

### 1. The description of the program ###

The main file, which contains the solution to the homework, is that one named
"sfl.c". In it, I used 2 own libraries: "list h" and "block.h". In the first of
them are defined 3 structures: one for a node with access in both directions,
other for a doubly linking list and the last one for a vector of lists of the
type mentioned previously. In the next library I defined just a structure which
have the purpose to save the informations about a block of memory. Also, it's
worth mentioning that "block.h" uses "list.h". Besides the structures mentioned
previously, in "sfl.c" is defined a structure which count the virtual
operations of allocations(malloc, free, fragmentation).

In the main file can be observed the existance of three principal variables:

- adll_free - array of free doubly linking lists - where are saved the free
blocks of memory
- adll_all - array of allocate doubly linking list - where are saved the
allocated bllocks of memory
- all_cnt - counter for the operations of allocations - where are counted
of how many times the virtual operations of malloc, free and fragmentation
were realised (this variable contains 3 counters, one for every operation
mentionated previously)

During the program I respected more rules, with the purpose to make myself
the life easier:
1. A node contains the information of a block of memory.
2. The blocks of memory from a list are ordered increasingly after the address
of start.
3. A list can contain just blocks of memory of the same size.
4. The lists from a vector must be ordered increasingly after the size of the
blocks which they save.

***A. INIT_HEAP***

The vector of lists adll_all is created before of this command and has space
for 16 lists (the number 16 was chosen randomly). At this time, the vector
contains no list and thus no block. Also, before this command, and all_cnt is
created and initialized with 0. In the moment when the program receives the
command "INIT_HEAP", the vector of lists adll_free is created and initialized
with the corresponding datas of the informations received in continuation. So,
at final, inside of it will be saved all free blocks / all blocks (because
no zone is allocated) from the virtual heap which we have it.

***B. MALLOC***

It searches, in the vector of free blocks, the zone which has the smallest
address and which has the same length with that asked or overcame it by the
least. In the case in whihc is not found such a block, the operation of
MALLOC can not take place. In the contrary case, the respective block is
taken out froom the vector. In continuation, exists 2 case:

	I. The taken out zone has the same size with the asked block of memory.

		Then, that zone is put in the vector which saves the allocated blocks,
		respecting the rules mentioned previously.

	II. The taken out zone overcomes the length of the wanted block.

		Then, the zone is separated in 2 such that the first piece to have
		the requested size. The first part is put in the vector with the
		allocated blocks, while the second one is added in the vector with the
		free chunks.

***C. FREE***

Search, in the vector with free block, the zone that starts with the given
address of user. In the case that it's not found the zone, means that does
not exist such a block and the operation can not take place. In the contrary
case, must take out the found chunck from vector and to add it in that one
with free zones. We do this indeferently of the type of reconstruction which
we have. In continuation, exists two cases:

	I. The type of reconstruction is 0.

	Then, we must do nothing anymore.

	II. The type of reconstruction is 1.

	Then, using the 2 vectors and the initial number of bytes per list with
	which was initialized the heap, we determinate the zone from where the
	block was broken. All the free chunks from that zone are taken out and
	are put in a temporary array of lists. If a block would form a continue
	zone after an union with the chunk which was freed, then is contopated
	with it. After is verified if the resulted block can not be united with
	other zone of memory from the temporary vector. This process continue until
	can not take place any fusion anymore. At end, all the blocks are taken
	and put back from the temporary vector in the vector which saves the free
	zones of memory.

***D. WRITE***

Verify if, in the zone which we want to write, the all bytes are allocated.
In the case that the operation can take place, we go through the vector with
the allocated blocks. In the moment when we find a chunk which is part,
partially or completely, from zone, we determinate:
- an offset, to know from what position of the received string we start
to take informations and put in the block.
- and number of bytes / characters which will be copied from the string
in the virtual block of memory.

***E. READ***

Verify, if in the zone which want to be read, all bytes are allocated. In the
case that the operation can take take place, we go through the vector with
the allocated blocks. In the moment when is found the block from the start of
the zone, we take from it the needed information and actualize the zone from
where want to read. This proces continue to take place until the zone gets to
the length zero.

***F. DUMP_MEMORY***

It's shown on the screen informations about the free blocks, allocated
zones and the virtual operations of allocation.

***G. DESTROY_HEAP***

It's freed the memory allocated for the 3 principal variables. 

### 2. Commnents about the homework ###

***Do I think that I could have done a better implementation?***

Yes, I believe that would have been more efficient to save all the allocated
blocks in just a list and not in a vector of lists. The biggest disadvantage
of my approach is at the operation of READ. There, I am constrained to go
through the allocated block by few times. In contrast, if I would have had
only a list in which I was saving the alloacated blocks increasingly after
the address of start, I could have need to go through those blocks just one
time.

***What I learned during this homework?***

During this homework I discovered a bigger image of the dynamic
allocation. In plus, I became more comfortable with the generic
programming.

