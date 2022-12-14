#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

// compilazione: g++ -xc++ consegna6_DennisTurco.c
// run: ./a.out
// grafo: dot graph.dot -Tpdf -o graph.pdf
/*
Obbiettivo:
	- Implementare l'algoritmo per il calcolo delle componenti fortemente connesse (versione di Tarjan) come discusso a lezione;
	- Utilizzare il codice lezione15 per partire con DFS ed adattarlo di conseguenza;
	- Opzionale: mostrare con dot l'evoluzione della ricerca e lo stato degli indici associati ai nodi.
*/

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

typedef struct list my_stack;

my_stack *s;

int idx = 0; //indice [0, n_nodi-1]

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
	// node* tail; // per lista doubly linked
} list_t;


//////////////////////////////////////////////////
// Fine Definizione della struttura dati lista
//////////////////////////////////////////////////

//////////////////////////////////////////////////
/// Definizione della struttura dati grafo
//////////////////////////////////////////////////

int *V;			 	// elenco dei nodi del grafo
int *V_visitato; 	// nodo visitato? -> struttura di supporto che servira' per altri algoritmi
int *V_lowlink;		// struttura di supporto per tenere traccia delle componenti fortemente connesse
int *V_onStack;		// struttura di supporto per memorizzare quali sono gli elemnti all'interno dello stack

// list_t* E;  	// array con le liste di adiacenza per ogni nodo
list_t **E; 	/// array di puntatori a le liste di adiacenza per ogni nodo
int n_nodi;

//////////////////////////////////////////////////
// Fine Definizione della struttura dati grafo
//////////////////////////////////////////////////

/// Questo e' un modo per stampare l'indirizzo node relativamente ad un altro di riferimento.
/// Permette di ottenere offset di piccola dimensione per essere facilmente visualizzati
/// Nota: il metodo non e' robusto e potrebbe avere comportamenti indesiderati su architetture diverse
/// L'alternativa corretta' e' utilizzare %p di printf: es. printf("%p\n",(void*) node);
/// con lo svantaggio di avere interi a 64 bit poco leggibili
/// usato per memorizzare il puntatore alla prima lista allocata
list_t *global_ptr_ref = NULL; 

int get_address(void *node){
	return (int)((long)node - (long)global_ptr_ref);
}

void node_print(int n){

	output_graph << "node_" << n << "_" << n_operazione << endl;
	output_graph << "[ shape = oval; ";
	//if (V_visitato[n])
	//	output_graph << "fillcolor = red; style=filled; ";
	output_graph << "label = "<< "\"Idx: " << n << ", val: " << V[n] << " index: "<< V_visitato[n] << " lowlink " << V_lowlink[n] << "\" ];\n";

	node_t *elem = E[n]->head;
	while (elem != NULL){ /// disegno arco
		output_graph << "node_" << n << "_" << n_operazione << " -> ";
		output_graph << "node_" << elem->val << "_" << n_operazione << " [  color=gray ]\n";
		elem = elem->next;
	}
}

void graph_print(){
	for (int i = 0; i < n_nodi; i++)
		node_print(i);
	n_operazione++;
}

void list_print(list_t *l){
	printf("Stampa lista\n");

	if (l->head == NULL){
		printf("Lista vuota\n");
	}
	else{
		node_t *current = l->head;

		while (current != NULL){
			if (!details)
				printf("%d, ", current->val);
			else{ /// stampa completa
				if (current->next == NULL)
					printf("allocato in %d [Val: %d, Next: NULL]\n",
						   get_address(current),
						   current->val);
				else
					printf("allocato in %d [Val: %d, Next: %d]\n",
						   get_address(current),
						   current->val,
						   get_address(current->next));
			}
			current = current->next;
		}
		printf("\n");
	}
}

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

void list_delete(list_t *l){
	//// implementare rimozione dal fondo della lista
	//// deallocazione struct list
}

void list_insert_front(list_t *l, int elem){
	/// inserisce un elemento all'inizio della lista
	node_t *new_node = new node_t;
	new_node->next = NULL;
	new_node->val = elem;
	new_node->next = l->head;
	l->head = new_node;
}

void list_delete_front(list_t *l){
    /// elimina il primo elemento della lista
    node_t *node = l->head; // il nodo da eliminare

    if (node == NULL) return; // lista vuota

    l->head = node->next;
    node->next = NULL;
    delete node;
}

// ---------------- operazioni stack ----------------
my_stack *stack_new(){
    return list_new();
}

int stack_top(my_stack *s){
    if (s->head != NULL)
        return s->head->val;
    printf("ERRORE: stack vuoto!\n");
    return -1;
}

int stack_pop(my_stack *s){
    if (s->head != NULL){
        int v = s->head->val;
        list_delete_front((list_t *)s);
        return v;
    }
    printf("ERRORE: stack vuoto!\n");
    return -1;
}

void stack_push(my_stack *s, int v){
    list_insert_front((list_t *)s, v);
}

void stack_print(my_stack *s){
    list_print((list_t *)s);
}
// ---------------------------------------------------

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

void scc(int v){

    V_visitato[v] = idx;
    V_lowlink[v] = idx;
    idx++; //incremento indice

    printf("\npush %d\n", v);
    stack_push(s, v); //push nello stack s il valore v
    V_onStack[v] = 1; //imposto V_onStack a 1 in corrispondenza del valore appena aggiunto

	/// esploro la lista di adiacenza
	node_t *elem = E[v]->head;
	while (elem != NULL){ /// elenco tutti i nodi nella lista
        
        printf("esploro %d -> %d\n", v, elem->val);

        if(V_visitato[elem->val] == -1) { // caso in cui non e' mai stato visitato
            printf("chiamo scc su %d\n", elem->val);
            scc(elem->val);
            if(V_lowlink[v] > V_lowlink[elem->val]){ // calcolo v.lowlink := min(v.lowlink, w.lowlink)
				printf("V_lowlink[v] = %d . V_lowlink[elem->val] = %d\n", V_lowlink[v], V_lowlink[elem->val]);
                V_lowlink[v] = V_lowlink[elem->val];
            }
        }
        else {
            if(V_onStack[elem->val] == 1){ // caso in cui il valore e' presente nello stack
                if(V_lowlink[v] > V_visitato[elem->val]){ // calcolo v.lowlink := min(v.lowlink, w.index)
					printf("V_lowlink[v] = %d . V_visitato[elem->val] = %d\n", V_lowlink[v], V_visitato[elem->val]);
                    V_lowlink[v] = V_visitato[elem->val];
                }
            }
        }

		elem = elem->next; // incremento elem
	}

    printf("finito il nodo %d\n", v);
    if (V_lowlink[v] == V_visitato[v]){
        //start a new strongly connected component
        printf("\nNuova componente connessa %d: ", v);
	
		//pulisco lo stack
		int val;
        do {
            val = stack_pop(s); // 'poppo' i valori della componente fortemente connessa appena trovata
            V_onStack[val] = 0; // inizializzo a 0 per indicare l'assenza dallo stack 
            printf("%d ", val); // stampo i valori della componente fortemente connessa
        } while(val != v);

		printf("\n");
    }

}

void swap(int &a, int &b){
	int tmp = a;
	a = b;
	b = tmp;
	ct_swap++; // aggiorno contatore globale di swap
}

/// controllo argomenti
int parse_cmd(int argc, char **argv){
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
		// preparo header
		output_graph << "digraph g" << endl;
		output_graph << "{ " << endl;
		output_graph << "node [shape=none]" << endl;
		output_graph << "rankdir=\"LR\"" << endl;
		;
		//    output_graph << "edge[tailclip=false,arrowtail=dot];"<<endl;
	}

    s = stack_new(); // non devo creare un nuovo oggetto stack ma voglio lavorare su quello definito globalmente

	n_nodi = 8; // se imposto un valore di nodi < 8 si entra in segmentation fault
	V = new int[n_nodi];		  //(int*)malloc(n_nodi*sizeof(int));
	V_visitato = new int[n_nodi]; //(int*)malloc(n_nodi*sizeof(int));
	V_lowlink = new int[n_nodi];  //(int*)malloc(n_nodi*sizeof(int));
	V_onStack = new int[n_nodi];  //(int*)malloc(n_nodi*sizeof(int));
	
    // creazione delle liste di adiacenza
	E = new list_t *[n_nodi]; //(list_t**)malloc(n_nodi*sizeof(list_t*));

	// inizializzazione
	for (int i = 0; i < n_nodi; i++){
		V[i] = 2 * i; // ogni nodo ha un valore che e'il doppio dell'indice
		V_visitato[i] = -1; // flag = non visitato // inizializzo V_visitato a -1 -> nessun nodo e' stato visitato

		E[i] = list_new();

		if (i == 0)
			global_ptr_ref = E[i];
	}

	// inserimenti
	// ho ottenuto la struttura del grafo da: https://it.wikipedia.org/wiki/Algoritmo_di_Tarjan_per_le_componenti_fortemente_connesse
	list_insert_front(E[0], 1); 
	list_insert_front(E[1], 2);
	list_insert_front(E[2], 0);
	list_insert_front(E[3], 1);
	list_insert_front(E[3], 2);
	list_insert_front(E[3], 4);
	list_insert_front(E[4], 3);
	list_insert_front(E[4], 5);
	list_insert_front(E[5], 2);
	list_insert_front(E[5], 6);
	list_insert_front(E[6], 5);
	list_insert_front(E[7], 7);
	list_insert_front(E[7], 4);
	list_insert_front(E[7], 6);


	for (int i=0; i<n_nodi; i++){ // stampa la lista di adiacenza per ciascun nodo
		printf("\nSono il nodo di indice %d nell'array\n", i);
		printf("Il valore del nodo e' %d\n", V[i]);
		printf("La lista di adiacenza e'\n");
		list_print(E[i]);
	}

    for (int i=0; i<n_nodi; i++){
		if (V_visitato[i] == -1){ // chiamata a funzione solo per i nodi non ancora visitati
			scc(i);
		}
    }
	graph_print();

	// oppure per cercare solo la componente di indice 'c':
	// int c = 0;
	// scc(c);


	if (graph){
		/// preparo footer e chiudo file
		output_graph << "}" << endl;
		output_graph.close();
		cout << "\n\nFile graph.dot scritto" << endl
			 << "Creare il grafo con: dot graph.dot -Tpdf -o graph.pdf" << endl;
	}

	return 0;
}