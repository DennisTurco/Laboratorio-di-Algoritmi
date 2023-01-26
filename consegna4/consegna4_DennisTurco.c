#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

// compilazione: g++ -xc++ consegna4_DennisTurco.c
// run: ./a.out -graph
// Obiettivo:
// 	- Implementare la rappresentazione del grafo tramite matrice di adiacenza.
// 	- Modificare il codice lezione15-grafi e sostituire le liste di adiacenza con la matrice.
// 	- Adattare la creazione del grafo e la visita Dfs per lavorare con le matrici.
// 	- Mantenere il codice originale, non sono accettate implementazioni completamente riscritte.

int ct_swap = 0;
int ct_cmp = 0;
int ct_op = 0; /// operazioni per la ricerca

int max_dim = 0;
int ntests = 1;
int ndiv = 1;
int details = 0;
int graph = 0;

int n = 0; /// dimensione dell'array

/// file di output per grafo
ofstream output_graph;
int n_operazione = 0; /// contatore di operazioni per visualizzare i vari step

int ct_visit = 0; // contatore durante visita

//////////////////////////////////////////////////
/// Definizione della struttura dati lista
//////////////////////////////////////////////////

/// struct per il nodo della lista
typedef struct node{
	int val;
	struct node *next;
} node_t;

/// struct per la lista
typedef struct list{
	node *head;
	// node* tail; /// per lista doubly linked
} list_t;

//////////////////////////////////////////////////
/// Fine Definizione della struttura dati lista
//////////////////////////////////////////////////

//////////////////////////////////////////////////
/// Definizione della struttura dati grafo
//////////////////////////////////////////////////

int *V;			 	// elenco dei nodi del grafo
int *V_visitato; 	// nodo visitato? -> struttura di supporto che servira' per altri algoritmi

// list_t* E;  	/// array con le liste di adiacenza per ogni nodo
node_t **E; 	/// crea matrice di adiacenza (rispetto alla versione originale ora non e' piu' un tipo list_t ma node_t poiche' ora lavoro con le matrici di adiacenza)
int n_nodi;

//////////////////////////////////////////////////
/// Fine Definizione della struttura dati grafo
//////////////////////////////////////////////////

/// Questo e' un modo per stampare l'indirizzo node relativamente ad un altro di riferimento.
/// Permette di ottenere offset di piccola dimensione per essere facilmente visualizzati
/// Nota: il metodo non e' robusto e potrebbe avere comportamenti indesiderati su architetture diverse
/// L'alternativa corretta' e' utilizzare %p di printf: es. printf("%p\n",(void*) node);
/// con lo svantaggio di avere interi a 64 bit poco leggibili.
/// usato per memorizzare il puntatore alla prima lista allocata
node_t *global_ptr_ref = NULL; 

int get_address(void *node){
	return (int)((long)node - (long)global_ptr_ref);
}

void node_print(int n){
	output_graph << "node_" << n << "_" << n_operazione << endl;
	output_graph << "[ shape = oval; ";
	if (V_visitato[n])
		output_graph << "fillcolor = red; style=filled; ";
	output_graph << "label = "
				 << "\"Idx: " << n << ", val: " << V[n] << "\" ];\n";

	node_t *elem = E[n]; // passiamo il pointer di E a elem
	while (elem != NULL){ /// disegno arco
		output_graph << "node_" << n << "_" << n_operazione << " -> ";
		output_graph << "node_" << elem->val << "_" << n_operazione << " [  color=gray ]\n";
		elem = elem->next;
	}
}

// (rispetto alla versione originale ora lavora con i node_t e non piu' con le list_t)
// va in overloading con la funzione sopra definita
void node_print(node_t *l){
	printf("Stampa lista\n");

	if (l == NULL){
		printf("Lista vuota\n");
	}
	else{
		node_t *current = l;

		while (current != NULL){
			if (!details)
				printf("%d, ", current->val);
			else{ /// stampa completa
				if (current->next == NULL)
					printf("allocato in %d [Val: %d, Next: NULL]\n", get_address(current), current->val);
				else
					printf("allocato in %d [Val: %d, Next: %d]\n", get_address(current), current->val, get_address(current->next));
			}
			current = current->next;
		}
		printf("\n");
	}
}

void graph_print(){
	for (int i = 0; i < n_nodi; i++)
		node_print(i);
	n_operazione++;
}


// FUNZIONE NON PIU' UTILIZZATA
list_t *list_new(void){
	list_t *l = new list;
	if (details){
		printf("Lista creata\n");
	}

	l->head = NULL; //// perche' non e' l.head ?
	if (details){
		printf("Imposto a NULL head\n");
	}

	return l;
}

// elimina tutti i nodi creati
void delete_node(node_t *node){
	if(node == NULL) return; // uscita dalla funzione

	delete_node(node->next); //chiamata ricorsiva sul prossimo nodo
	delete node; //eliminazione nodo
}

//stampa matrice di adiacenza
void print_matrice_adiacenza(int** matrice_adiacenza){
	for	(int i=0; i<n_nodi; i++){
		for (int j=0; j<n_nodi; j++){
			printf("%d ", matrice_adiacenza[i][j]);
		}
		printf("\n");
	}
}

// costruzione matrice di adiacenza
int** new_matrice_adiacenza(){
	
	//creazione matrice
	int** matrice_adiacenza = new int *[n_nodi]; //creo la colonna della matrice
	for (int i=0; i<n_nodi; i++){
		matrice_adiacenza[i] = new int [n_nodi]; //creo le righe della matrice
	}

	//pre-inizializzo la matrice a 0 per evitare possibili problematiche dovute a valori non inizializzati
	for (int i=0; i<n_nodi; i++){
		for (int j=0; j<n_nodi; j++){
			matrice_adiacenza[i][j] = 0;
		}
	}
	
	//cerco gli archi dei nodi assegnando 1 nella matrice di adiacenza quando li trovo 
	for (int i=0; i<n_nodi; i++){
		node_t* elem = E[i];

		while (elem != NULL){
			// assegno 1 le posizioni della matrice in cui vi e' la presenza di archi 
			// elem->val se != NULL indica per ciascuna riga della matrice, dove il nodo i ha un arco verso il nodo elem->val 
			matrice_adiacenza[i][elem->val] = 1; 
			elem = elem->next; 
		}
	}
	return matrice_adiacenza;
}

// la trasformo da list a node perchè ora lavoriamo con le matrici e non più con le liste 
/// inserisce un elemento all'inizio della lista
node_t* node_insert_front(node_t *l, int elem){
	node_t *new_node = new node_t;
	
	new_node->val = elem; // inserisco il valore passato come parametro al nuovo nodo
	new_node->next = l; //il nuovo nodo non punta a nulla
	
	l = new_node; // l punta a new_node

	return l;
}

void print_array(int *A, int dim){
	for (int j = 0; j < dim; j++){
		printf("%d ", A[j]);
	}
	printf("\n");
}

void print_array_graph(int *A, int n, string c, int a, int l, int m, int r){
	/// prepara il disegno dell'array A ed il suo contenuto (n celle)
	/// a e' il codice del nodo e c la stringa
	/// l,m,r i tre indici della bisezione

	// return ;

	output_graph << c << a << " [label=<" << endl;

	/// tabella con contenuto array
	output_graph << "<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" > " << endl;
	/// indici
	output_graph << "<TR  >";
	for (int j = 0; j < n; j++){
		output_graph << "<TD ";
		output_graph << ">" << j << "</TD>" << endl;
	}
	output_graph << "</TR>" << endl;
	output_graph << "<TR>";
	// contenuto
	for (int j = 0; j < n; j++){
		output_graph << "<TD BORDER=\"1\"";
		if (j == m)
			output_graph << " bgcolor=\"#00a000\""; /// valore testato
		else if (j >= l && j <= r)
			output_graph << " bgcolor=\"#80ff80\""; /// range di competenza
		output_graph << ">" << A[j] << "</TD>" << endl;
	}
	output_graph << "</TR>" << endl;
	output_graph << "</TABLE> " << endl;

	output_graph << ">];" << endl;
}

void DFS(int n){

	graph_print();

	if (details)
		printf("DFS: lavoro sul nodo %d (visitato %d)\n", n, V_visitato[n]);

	if (V_visitato[n]) return; // nodo gia' visitato

	V_visitato[n] = 1; // prima volta che incontro questo nodo

	if (details)
		printf("Visito il nodo %d (val %d)\n", n, V[n]);

	/// esploro la matrice di adiacenza
	node_t *elem = E[n];
	while (elem != NULL){ /// elenco tutti i nodi nella matrice
		/// espando arco  n --> elem->val
		/// quindi DFS(elem->val)
		output_graph << "dfs_" << n << " -> dfs_" << elem->val;
		if (V_visitato[elem->val])
			output_graph << "[color=gray, label = \"" << ct_visit++ << "\"]";
		else
			output_graph << "[color=red, label = \"" << ct_visit++ << "\"]";
		output_graph << endl;

		DFS(elem->val); // eseguo ricorsivamente sul valore successivo

		elem = elem->next; // mi sposto nei nodi
	}
}


void swap(int &a, int &b){
	int tmp = a;
	a = b;
	b = tmp;
	/// aggiorno contatore globale di swap
	ct_swap++;
}

int parse_cmd(int argc, char **argv){
	/// controllo argomenti
	int ok_parse = 0;
	for (int i = 1; i < argc; i++){
		if (argv[i][1] == 'v'){
			details = 1;
			ok_parse = 1;
		}
		if (argv[i][1] == 'g'){
			graph = 1;
			ok_parse = 1;
		}
	}

	if (argc > 1 && !ok_parse){
		printf("Usage: %s [Options]\n", argv[0]);
		printf("Options:\n");
		printf("  -verbose: Abilita stampe durante l'esecuzione dell'algoritmo\n");
		printf("  -graph: creazione file di dot con il grafo dell'esecuzione (forza d=1 t=1)\n");
		return 1;
	}

	return 0;
}

int main(int argc, char **argv){
	int i, test;

	// init random
	srand((unsigned) time(NULL));

	if (parse_cmd(argc, argv))
		return 1;

	if (graph){
		output_graph.open("graph.dot");
		/// preparo header
		output_graph << "digraph g" << endl;
		output_graph << "{ " << endl;
		output_graph << "node [shape=none]" << endl;
		output_graph << "rankdir=\"LR\"" << endl;
		;node_insert_front;
		//    output_graph << "edge[tailclip=false,arrowtail=dot];"<<endl;
	}

	// int* V; // elenco dei nodi del grafo
	// list_t* E;  /// array con le liste di adiacenza per ogni nodo

	n_nodi = 5;
	V = new int[n_nodi];		  //(int*)malloc(n_nodi*sizeof(int));
	V_visitato = new int[n_nodi]; //(int*)malloc(n_nodi*sizeof(int));

	// creazione matrice di adiacenza
	// crea matrice di tot n_nodi di tipo node_t*
	E = new node_t *[n_nodi]; //(list_t**)malloc(n_nodi*sizeof(list_t*));

	// inizializzazione
	for (int i = 0; i < n_nodi; i++){
		V[i] = 2 * i; // ogni nodo ha un valore che e'il doppio dell'indice
		V_visitato[i] = 0; // flag = non visitato

		if (i == 0)
			global_ptr_ref = E[i];

		// inserimento dei nodi all'interno della lista
		// alla fine di questo ciclo otteniamo un grafo completo
		for (int j = 0; j < n_nodi; j++){
			// if (rand()%2==0)
			if(i <= j) //metto un arco solo se c'e'una relazione di minori
				E[i] = node_insert_front(E[i], j);
		}
	}

	printf(" ------------ MATRICE ADIACENZA ------------ \n");
	int **matrice_adiacenza = new_matrice_adiacenza();
	print_matrice_adiacenza(matrice_adiacenza);

	graph_print();

	for (int i = 0; i < n_nodi; i++){
		printf("\nSono il nodo di indice %d nell'array\n", i);
		printf("Il valore del nodo e' %d\n", V[i]);
		printf("La lista di adiacenza e'\n");
		node_print(E[i]);
	}

	//pulisco E
	for(int i=0; i<n_nodi; i++){
		delete_node(E[i]);
	}
	
	// Costruzione grafo partendo dalla matrice di adiacenza appena ottenuta
	for (int i=0; i<n_nodi; i++){
		E[i] = new node_t [n_nodi];
		for (int j = 0; j<n_nodi; j++){
			if(matrice_adiacenza[i][j] == 1) E[i] = node_insert_front(E[i], j);
		}
	}

	DFS(1); //visita DFS // 1 mi rappresenta l'indice di partenza della visita DFS
	

	if (graph){
		/// preparo footer e chiudo file
		output_graph << "}" << endl;
		output_graph.close();
		cout << " File graph.dot scritto" << endl
			 << "Creare il grafo con: dot graph.dot -Tpdf -o graph.pdf" << endl;
	}

	return 0;
}