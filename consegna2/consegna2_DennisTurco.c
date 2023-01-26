#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

// compilazione: g++ -xc++ consegna2_DennisTurco.c
// ./a.out -graph
// dot graph.dot -Tpdf -o graph.pdf

// Alberi binari

// Obiettivo:
// 6) euler tour: stampa e ricostruzione albero da lista nodi
// 7) flip albero
// 8) height - depth

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

ofstream output_visit;
ifstream input_visit;

//////////////////////////////////////////////////
/// Definizione della struttura dati tree
//////////////////////////////////////////////////

/// struct per il nodo dell'albero
typedef struct node{
	int val;
	struct node *L;
	struct node *R;
} node_t;

//////////////////////////////////////////////////
/// Fine Definizione della struttura dati tree
//////////////////////////////////////////////////

/// Questo e' un modo per stampare l'indirizzo node relativamente ad un altro di riferimento.
/// Permette di ottenere offset di piccola dimensione per essere facilmente visualizzati
/// Nota: il metodo non e' robusto e potrebbe avere comportamenti indesiderati su architetture diverse
/// L'alternativa corretta' e' utilizzare %p di printf: es. printf("%p\n",(void*) node);
/// con lo svantaggio di avere interi a 64 bit poco leggibili

node_t *global_ptr_ref = NULL; /// usato per memorizzare il puntatore alla prima lista allocata


// ottengo indirizzo di memoria del nodo corrente
int get_address(void *node){
	if (node == NULL)
		return 0;
	return (int)((long)node - (long)global_ptr_ref);
}

/// stampa il codice del nodo per dot
void print_node_code(node_t *n){
	output_graph << "node_" << get_address(n) << "_" << n_operazione;
}

void node_print_graph(node_t *n){

	print_node_code(n);
	output_graph << "\n[label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" >\n<TR> <TD CELLPADDING=\"3\" BORDER=\"0\"  ALIGN=\"LEFT\" bgcolor=\"#f0f0f0\" PORT=\"id\">";
	output_graph << get_address(n) << "</TD> </TR><TR>\n<TD PORT=\"val\" bgcolor=\"#a0FFa0\">";
	output_graph << n->val << "</TD>\n <TD PORT=\"L\" ";
	if (n->L == NULL)
		output_graph << "bgcolor=\"#808080\"> NULL";
	else
		output_graph << "> " << get_address(n->L);
	output_graph << "</TD>\n <TD PORT=\"R\" ";
	if (n->R == NULL)
		output_graph << "bgcolor=\"#808080\"> NULL";
	else
		output_graph << "> " << get_address(n->R);
	output_graph << "</TD>\n</TR></TABLE>>];\n";

	/// visualizzazione figli sullo stesso piano
	if (n->L != NULL && n->R != NULL){
		output_graph << "rank = same; ";
		print_node_code(n);
		output_graph << ";";
		print_node_code(n->L);
		output_graph << ";\n";
	}

	// mostro archi uscenti

	if (n->L != NULL){ /// disegno arco left
		print_node_code(n);
		output_graph << ":L:c -> ";
		print_node_code(n->L);
		output_graph << ":id ;\n";
	}

	if (n->R != NULL){ /// disegno arco R
		print_node_code(n);
		output_graph << ":R:c -> ";
		print_node_code(n->R);
		output_graph << ":id ;\n";
	}
}

void tree_print_rec_graph(node_t *n){
	if (n != NULL){
		node_print_graph(n);
		tree_print_rec_graph(n->L);
		tree_print_rec_graph(n->R);
	}
}

void tree_print_graph(node_t *n){
	/// stampa ricorsiva del noto
	tree_print_rec_graph(n);
	n_operazione++;
}

void node_print(node_t *n){
	if (n == NULL)
		printf("Puntatore vuoto\n");
	else
		printf("allocato in %d [Val: %d, L: %d, R: %d]\n",
			   get_address(n),
			   n->val,
			   get_address(n->R),
			   get_address(n->L));
}

node_t *node_new(int elem){ /// crea nuovo nodo
	node_t *t = new node_t;
	if (details){
		printf("nodo creato\n");
	}

	t->val = elem;
	t->L = NULL;
	t->R = NULL;
	if (details){
		printf("Imposto a NULL children\n");
	}

	return t;
}

void tree_insert_child_L(node_t *n, int elem){
	/// inserisco il nuovo nodo con contenuto elem
	/// come figlio Left del nodo n

	/// creo nodo
	n->L = node_new(elem);
}

void tree_insert_child_R(node_t *n, int elem){
	/// inserisco il nuovo nodo con contenuto elem
	/// come figlio Right del nodo n
	n->R = node_new(elem);
}

int max_nodes = 10;
int n_nodes = 0;

void insert_random_rec(node_t *n){
	//// inserisce in modo random un nodo L e R e prosegue ricorsivamente
	/// limito i nodi interni totali, in modo da evitare alberi troppo grandi

	printf("inserisco %d\n", n_nodes);

	if (n_nodes++ >= max_nodes) /// limito il numero di nodi
		return;
	printf("inserisco %d\n", n_nodes);

	float probabilita = 0.5; /// tra 0 e 1

	if (rand() % 100 < probabilita * 100){ // se numero random e' minore della probabilita' -> aggiungo nodo R con valore a caso
		tree_insert_child_R(n, rand() % 100);
	}
	if (rand() % 100 < probabilita * 100){ // se numero random e' minore della probabilita' -> aggiungo nodo L con valore a caso
		tree_insert_child_L(n, rand() % 100);
	}
	if (n->L != NULL)
		insert_random_rec(n->L);
	if (n->R != NULL)
		insert_random_rec(n->R);
}

int ct_visit = 0;


void inOrder(node_t *n){

	if (n->L != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":id:w -> ";
			print_node_code(n->L);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}

		inOrder(n->L);

		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->L);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":val:s [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	if (details)
		printf("%d ", n->val);

	if (graph){
		node_print_graph(n);
	}

	if (n->R != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":val:s -> ";
			print_node_code(n->R);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
		inOrder(n->R);
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->R);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":id:e [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}
}

void preOrder(node_t *n){

	if (details)
		printf("%d ", n->val);

	if (graph){
		node_print_graph(n);
	}

	if (n->L != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":id:w -> ";
			print_node_code(n->L);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}

		preOrder(n->L);

		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->L);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":val:s [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	if (n->R != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":val:s -> ";
			print_node_code(n->R);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
		preOrder(n->R);
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->R);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":id:e [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}
}

void postOrder(node_t *n){

	if (n->L != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":id:w -> ";
			print_node_code(n->L);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}

		postOrder(n->L);

		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->L);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":val:s [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	if (n->R != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":val:s -> ";
			print_node_code(n->R);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
		postOrder(n->R);
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->R);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":id:e [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	if (details)
		printf("%d ", n->val);

	if (graph){
		node_print_graph(n);
	}
}

void EulerOrder(node_t *n){

	printf("%d ", n->val);

	output_visit << n->val << "\n";

	node_print_graph(n);

	if (n->L != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":id:w -> ";
			print_node_code(n->L);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}

		EulerOrder(n->L);

		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->L);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":val:s [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	printf("%d ", n->val);
	output_visit << n->val << "\n";

	if (n->R != NULL){
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n);
			output_graph << ":val:s -> ";
			print_node_code(n->R);
			output_graph << ":id:w [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
		EulerOrder(n->R);
		if (graph){
			// scrivo arco con numero operazione di visita
			print_node_code(n->R);
			output_graph << ":id:e -> ";
			print_node_code(n);
			output_graph << ":id:e [ color=blue, fontcolor=blue, penwidth=3, label = \"" << ct_visit++ << "\"]\n";
		}
	}

	printf("%d ", n->val);
	output_visit << n->val << "\n";
}


/// l'estrazione del nodo "pre" deve essere fatta esternamente alla chiamata ricorsiva
/// e passata come argomento
///  input_visit >> preordine;

// costruzione albero in base ai valori stampati dalla visita di eulero
// la funzione restituisce il puntatore alla radice dell'albero
// Es. input: 32 74 74 64 76 76 44 44 44 76 64 64 74 32 85 85 2 36 36 36 2 85 85 85 2 85 32

node_t *build_euler (int preordine){
	node_t *root = node_new(preordine);	// il primo valore e' la radice dell'albero
	int n1, n2, n3; //i tre valori di n rappresentano rispettivamente il valore della chiave del nodo corrente e il valore del figlio di sinistra e di quello di destra
	n1 = preordine;

	input_visit >> n2;

	if (n1 != n2){ // se il valore del figlio di sinistra e' differente dal valore del padre, significa che il nodo di valore n1 ha un figlio sinistro di valore n2
		root->L = build_euler(n2); // nuovo nodo a sinistra
		input_visit >> n2;
	}

	input_visit >> n3;

	if (n1 != n3){ // se il valore del figlio di destra e' differente dal valore del padre, significa che il nodo di valore n1 ha un figlio destro di valore n3
		root->R = build_euler(n3); // nuovo nodo a destra
		input_visit >> n3;
	} 

	return root;
}








/// crea una copia dell'albero in input, scambiando i sottoalberi L e R
node_t *flip(node_t *n){
	node_t *n1 = n;

	//caso in cui l'albero e' vuoto
	if (n1 == NULL) return NULL;

	/// chiamate ricorsive
	flip(n1->L); 
	flip(n1->R);

	//creo un temporaneo per gli scambi
	node_t *temp; 

	//swap -> inverto i nodi di sinistra con i nodi di destra
	temp = n1->L;
	n1->L = n1->R;
	n1->R = temp;
	
	return n1;
}

int height_tree(node_t *n){
	//caso albero vuoto
	if (n == NULL) return 0;

	// contatori che mi tengono traccia dell'altezza dei due sottoalberi (sinistra e destra)
	int left_side;
	int right_side;
	
	left_side = height_tree(n->L); //chiamata ricorsiva sul sottoalbero di sinistra
	right_side = height_tree(n->R); //chiamata ricorsiva sul sottoalbero di destra
	
	if(left_side > right_side) return left_side + 1; //incremento il contatore di sinistra
	else return right_side + 1; //incremento il contatore di destra

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

	if (parse_cmd(argc, argv))
		return 1;

	// init random
	srand((unsigned)time(NULL));

	if (graph){
		output_graph.open("graph.dot");
		/// preparo header
		output_graph << "digraph g" << endl;
		output_graph << "{ " << endl;
		output_graph << "node [shape=none]" << endl;
		output_graph << "rankdir=\"TB\"" << endl;
		;
		output_graph << "edge[tailclip=false,arrowtail=dot];" << endl;
	}

	// PUNTO 6 -> euler tour: stampa e ricostruzione albero da lista nodi
	//  stampa visita di eulero
	printf("6) -------- Euler Tour --------\n");
	int val;
	input_visit.open("visit.txt");
	input_visit >> val;
	node_t *root = build_euler(val);
	input_visit.close();
	global_ptr_ref = root;
	EulerOrder(root); // la visita di eulero deve coincidere con il file da cui si e' creato l'albero

	// PUNTO 7 -> flip albero
	printf("\n\n7) -------- Flip Albero --------\n");
	node_t *root_flipped = flip(root); // root_flipped e' il nuovo albero con figli destri e sinistri scambiati fra loro
	printf("Visita Eulero dell'albero invertito: \n");
	EulerOrder(root_flipped);
	
	// PUNTO 8 -> height - depth
	printf("\n\n8) -------- Height-Deep --------\n");
	int height = height_tree(root);
	printf("altezza albero = %d\n\n\n", height);

	tree_print_graph(root);

	if (graph){
		/// preparo footer e chiudo file
		output_graph << "}" << endl;
		output_graph.close();
		cout << " File graph.dot scritto" << endl
			 << "Creare il grafo con: dot graph.dot -Tpdf -o graph.pdf" << endl;
	}

	return 0;
}
