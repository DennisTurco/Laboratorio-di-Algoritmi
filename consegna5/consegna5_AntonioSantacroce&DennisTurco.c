/////////////////////////////////////////////////////////////////////////////
//// Algoritmo di Dijkstra con rimozione nodi controllati ///////////////////
//////////// Tramite Min_Heap - Antonio Santacroce & Dennis Turco ///////////
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

// compilazione: g++ -xc++ consegna5_AntonioSantacroce&DennisTurco.c
//
// Obiettivo:
// 1) grafo con archi pesati
// 2) implementazione shortest path

#define INFTY 1000000

int ct_swap = 0;
int ct_cmp = 0;
int ct_op = 0; /// operazioni per la ricerca

int max_dim = 0;
int ntests = 1;
int ndiv = 1;
int details = 0;
int graph = 0;

/// file di output per grafo
ofstream output_graph;
int n_operazione = 0; /// contatore di operazioni per visualizzare i vari step

int ct_visit = 0; // contatore durante visita

//////////////////////////////////////////////////
/// Definizione funzioni MIN HEAP
//////////////////////////////////////////////////

const int MAX_SIZE = 1024; 	/// allocazione statica
int heap[MAX_SIZE];	   		/// prima riga sarà la distanza minore del nodo
int heap_index[MAX_SIZE];
int heap_size = 0; 			/// dimensione attuale dell'heap

/// uso -1 per indicare un indice non esistente
int parent_idx(int n)
{
	if (n == 0)
		return -1;
	return (n - 1) / 2;
}

int child_L_idx(int n)
{
	if (2 * n + 1 >= heap_size)
		return -1;
	return 2 * n + 1;
}

int child_R_idx(int n)
{
	if (2 * n + 2 >= heap_size)
		return -1;
	return 2 * n + 2;
}

/// restituisce 0 se il nodo in posizione n e' un nodo interno (almeno un figlio)
/// restituisce 1 se il nodo non ha figli
int is_leaf(int n)
{
	return (child_L_idx(n) == -1); // non c'e' bisogno di controllare il figlio R
}

void sistema_relazione_con_genitori(int &i, int &p, int &val_i, int &val_p, int &idx_i, int &idx_p)
{ // passando i temp come & evito di eseguire n letture in piu'

        while (p >= 0)
        {
            if (val_i < val_p)
            { // constrollo se il valore appena aggiunto e' minore del padre e se si li swappo (siamo nel caso min heap)
                // swap
                swap(val_i, val_p);
                swap(idx_i, idx_p);

                // preparo per la prosima iterazione
                i = p;
                p = parent_idx(i);
                sistema_relazione_con_genitori(i, p, heap[i], heap[p], heap_index[i], heap_index[p]);
            }
            else
                break;
        }
    }

void heap_insert(int elem)
{
	/// inserisco il nuovo nodo con contenuto elem
	/// nell'ultima posizione dell'array
	/// ovvero continuo a completare il livello corrente

	if (details)
		printf("Inserisco elemento %d in posizione %d\n", elem, heap_size);

	if (heap_size < MAX_SIZE)
	{	
		
		int i = heap_size;
		heap_size++;
		//heap[i] = elem;
		int p = parent_idx(i);
		sistema_relazione_con_genitori(i, p, heap[i] = elem, heap[p], heap_index[i], heap_index[p]);
	}
	else if (details)
		printf("Heap pieno!\n");
}


void decrease_key(int indice_nodo, int key)
{ // da nn confondere con key, indice fa riferimento a heap_index mentre key è la chiave con cui ridurre il valore di un nodo
	// key = nuovo valore

	if (indice_nodo < 0 || indice_nodo >= heap_size)
	{
		printf("Nodo non esistente\n");
		return;
	}
	
	heap[indice_nodo] = key;

	//// sistema relazione con genitori! ////
	int i = indice_nodo;
	int p = parent_idx(i);
	sistema_relazione_con_genitori(i, p, heap[i], heap[p], heap_index[i], heap_index[p]);
}

int heap_remove_min()
{

	if (heap_size <= 0)
	{ /// heap vuoto!
		printf("Errore: heap vuoto\n");
		return -1;
	}

	int &temp_heapMinimo = heap[0];
	int &temp_heap_min_idx = heap_index[0];
	int &temp_heapSize = heap[heap_size - 1];
	int &temp_heap_2 = heap_index[heap_size - 1];

	int minimo = temp_heapMinimo; // il minimo e' il primo elemento dell'array

	if (details)
		printf("Minimo identificato %d\n", minimo);

	/// scambio la radice con l'ultima foglia a destra
	/// il minimo e' stato spostato in fondo --> pronto per l'eliminazione
	swap(temp_heapMinimo, temp_heapSize);
	swap(temp_heap_min_idx, temp_heap_2);

	n_operazione++;

	// elimino il minimo (ora in fondo all'array)
	heap_size--;

	/// nella radice c'e' un valore grande (massimo?) -> si se disposto in ordine decrescente
	int i = 0; // indice di lavoro (parto dalla root)

	while (i >= 0 && !is_leaf(i))
	{ /// garantisco di fermarmi alla foglia

		int &temp_heap = heap[i]; // temp
		int &temp_heap_i = heap_index[i];

		if (details)
			printf("Lavoro con il nodo in posizione i = %d, valore %d\n", i, temp_heap);

		int con_chi_mi_scambio = -1; /// -1 === non mi scambio con nessuno

		int child_L = child_L_idx(i);
		int child_R = child_R_idx(i);

		int &temp_childL = heap[child_L]; // temp

		/// max L R --> indice
		if (temp_childL < temp_heap)
		{ /// confronto i con L
			con_chi_mi_scambio = child_L;
		}

		/// considero figlio destro
		if (child_R >= 0)
		{ // esiste R

			int &temp_childR = heap[child_R];

			/// controllare L <-> R e i <-> R
			if (temp_childR < temp_childL && temp_childR < temp_heap)
			{
				con_chi_mi_scambio = child_R;
			}
		}

		if (con_chi_mi_scambio >= 0)
		{

			int &temp_heapScambio = heap[con_chi_mi_scambio]; // questo passaggio in realta' non mi fa risparmiare letture perche' usato una sola volta e quando passato a funzione viene gia' trattato come &
			int &temp_heap_si = heap_index[con_chi_mi_scambio];

			/// swap tra i e con_chi_mi_scambio
			swap(temp_heap, temp_heapScambio);
			swap(temp_heap_i, temp_heap_si);
		}

		i = con_chi_mi_scambio;
	}

	return minimo;
}

//////////////////////////////////////////////////
/// Fine Definizione funzioni MIN HEAP
//////////////////////////////////////////////////

//////////////////////////////////////////////////
/// Definizione della struttura dati lista
//////////////////////////////////////////////////

/// struct per il nodo della lista
typedef struct node
{
	int val; /// prossimo nodo
	float w; /// peso dell'arco
	struct node *next;
} node_t;

/// struct per la lista
typedef struct list
{
	node *head;
} list_t;

//////////////////////////////////////////////////
/// Fine Definizione della struttura dati lista
//////////////////////////////////////////////////

//////////////////////////////////////////////////
/// Definizione della struttura dati grafo
//////////////////////////////////////////////////

int *V;		 // elenco dei nodi del grafo
int *V_visitato; // nodo visitato?
int *V_prev;	 // nodo precedente dalla visita
float *V_dist;	 // distanza da sorgente

// list_t* E;  /// array con le liste di adiacenza per ogni nodo
list_t **E; /// array di puntatori a le liste di adiacenza per ogni nodo
int n_nodi;

//////////////////////////////////////////////////
/// Fine Definizione della struttura dati grafo
//////////////////////////////////////////////////

/// Questo e' un modo per stampare l'indirizzo node relativamente ad un altro di riferimento.
/// Permette di ottenere offset di piccola dimensione per essere facilmente visualizzati
/// Nota: il metodo non e' robusto e potrebbe avere comportamenti indesiderati su architetture diverse
/// L'alternativa corretta' e' utilizzare %p di printf: es. printf("%p\n",(void*) node);
/// con lo svantaggio di avere interi a 64 bit poco leggibili

list_t *global_ptr_ref = NULL; /// usato per memorizzare il puntatore alla prima lista allocata

int get_address(void *node)
{
	return (int)((long)node - (long)global_ptr_ref);
}

void node_print(int n)
{

	/// calcolo massima distanza (eccetto infinito)
	float max_d = 0;
	for (int i = 0; i < n_nodi; i++)
		if (max_d < V_dist[i] && V_dist[i] < INFTY)
			max_d = V_dist[i];

	output_graph << "node_" << n << "_" << n_operazione << endl;
	output_graph << "[ shape = oval; ";

	if (V_visitato[n] == 1)
		output_graph << "penwidth = 4; ";

	float col = V_dist[n] / max_d; /// distanza in scala 0..1
	output_graph << "fillcolor = \"0.0 0.0 " << col / 2 + 0.5 << "\"; style=filled; ";
	if (V_dist[n] < INFTY)
		output_graph << "label = "
			     << "\"Idx: " << n << ", dist: " << V_dist[n] << "\" ];\n";
	else
		output_graph << "label = "
			     << "\"Idx: " << n << ", dist: INF\" ];\n";

	node_t *elem = E[n]->head;
	while (elem != NULL){ /// disegno arco
		output_graph << "node_" << n << "_" << n_operazione << " -> ";
		output_graph << "node_" << elem->val << "_" << n_operazione << " [ label=\"" << elem->w << "\", len=" << elem->w / 100 * 10 << " ]\n";
		elem = elem->next;
	}

	if (V_prev[n] != -1){ // se c'e' un nodo precedente visitato -> disegno arco
		
		//cerco arco V_prev[n] --> V[n]
		float len = 1;
		
		node_t *elem = E[V_prev[n]]->head;
		while(elem != NULL)
		{
			if (elem->val == V[n])
				len = elem->w;
			elem = elem->next;
		}
		
		output_graph << "node_" << n << "_" << n_operazione << " -> ";
		output_graph << "node_" << V_prev[n] << "_" << n_operazione << " [ color=blue, penwidth=5, len="<< len/100*10 << " ]\n";
	}
}

void graph_print()
{
	for (int i = 0; i < n_nodi; i++)
		node_print(i);
	n_operazione++;
}

void list_print(list_t *l)
{
	printf("Stampa lista\n");

	if (l->head == NULL)
	{
		printf("Lista vuota\n");
	}
	else
	{
		node_t *current = l->head;

		while (current != NULL)
		{
			if (!details)
				printf("%d w:%f, ", current->val, current->w);
			else
			{ /// stampa completa
				if (current->next == NULL)
					printf("allocato in %d [Val: %d, W: %f, Next: NULL]\n",
					       get_address(current),
					       current->val,
					       current->w);
				else
					printf("allocato in %d [Val: %d, W: %f, Next: %d]\n",
					       get_address(current),
					       current->val,
					       current->w,
					       get_address(current->next));
			}
			current = current->next;
		}
		printf("\n");
	}
}

list_t *list_new(void)
{
	list_t *l = new list;
	if (details)
	{
		printf("Lista creata\n");
	}

	l->head = NULL; //// perche' non e' l.head ?
	if (details)
	{
		printf("Imposto a NULL head\n");
	}

	return l;
}

void list_insert_front(list_t *l, int elem, float w)
{
	/// inserisce un elemento all'inizio della lista
	node_t *new_node = new node_t;
	new_node->next = NULL;

	new_node->val = elem;
	new_node->w = w;

	new_node->next = l->head;
	l->head = new_node;
}

void print_array(int *A, int dim)
{
	for (int j = 0; j < dim; j++)
	{
		printf("%d ", A[j]);
	}
	printf("\n");
}

void print_array_graph(int *A, int n, string c, int a, int l, int m, int r)
{
	/// prepara il disegno dell'array A ed il suo contenuto (n celle)
	/// a e' il codice del nodo e c la stringa
	/// l,m,r i tre indici della bisezione

	// return ;

	output_graph << c << a << " [label=<" << endl;

	/// tabella con contenuto array
	output_graph << "<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" > " << endl;
	/// indici
	output_graph << "<TR  >";
	for (int j = 0; j < n; j++)
	{
		output_graph << "<TD ";
		output_graph << ">" << j << "</TD>" << endl;
	}
	output_graph << "</TR>" << endl;
	output_graph << "<TR>";
	// contenuto
	for (int j = 0; j < n; j++)
	{
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////// SHORTEST PATH //////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void shortest_path(int n)
{
	V_dist[n] = 0;
	decrease_key(n, 0);
	int q_size = n_nodi; // contatore elementi presenti ancora sull'array V_visistato[]

	while (q_size > 0)
	{
		//  trova il minimo in coda			  // V_visitato[n] == 0 && V_dist[n] < best_dist){ // solo se in coda e migliore del nodo corrente
		float best_dist = heap[0];	  // per come è strutturato il min heap, la radice è sempre l'elemento più piccolo
		int best_idx = heap_index[0]; // di conseguenza heap index conterrà l'indice del minore
		
		// potenziale nodo migliore trovato probabilmente eccetto se tutti i nodi hanno distanza infinita se il grafo è disconnesso
		if (best_idx >= 0)
		{
			// estrai dalla coda
			int u = best_idx;
			heap_remove_min();
			V_visitato[u] = 1; // modifichiamo il flag, nodo trovato
			q_size--; 

			// esploro la lista di adiacenza
			node_t *elem = E[u]->head;
			while (elem != NULL)
			{
				int v = elem->val; /// best_idx ------> v

				if (V_visitato[v] == 0 || V_dist[v] == INFTY)
				{
					float alt = V_dist[u] + elem->w; // costo per arrivare al nuovo nodo passando per best_idx dove il percorso è il minore alt<-dist[best_idx]+graph.edges(u,v)
					// elem -> val; // valore del nodo che è raggiunto dall'arco considerato con la lista di adiacenza
					// elem -> w; // peso dell'arco considerato

					if (alt < V_dist[v])
					{ // trovata una scorciatoia verso v => informo V_dist
						V_dist[v] = alt; 
						V_prev[v] = u;
						decrease_key(heap_index[v], alt);
					}
				}

				elem = elem->next;
			}
		}
		else { // coda non vuota e nodi nn raggiungibili -----> finito
			q_size = 0;
		}
	}
	graph_print();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////// FINE SHORTEST PATH //////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int DFS(int n)
{

	graph_print();

	if (details)
		printf("DFS: lavoro sul nodo %d (visitato %d)\n", n, V_visitato[n]);

	if (V_visitato[n])
		return 0;

	V_visitato[n] = 1; // prima volta che incontro questo nodo

	if (details)
		printf("Visito il nodo %d (val %d)\n", n, V[n]);

	/// esploro la lista di adiacenza
	node_t *elem = E[n]->head;
	while (elem != NULL)
	{ /// elenco tutti i nodi nella lista

		/// espando arco  n --> elem->val
		/// quindi DFS(elem->val)
		output_graph << "dfs_" << n << " -> dfs_" << elem->val;
		if (V_visitato[elem->val])
			output_graph << "[color=gray, label = \"" << ct_visit++ << "\"]";
		else
			output_graph << "[color=red, label = \"" << ct_visit++ << "\"]";
		output_graph << endl;

		if (V_visitato[elem->val] == 1)
			return 1;

		int ret = DFS(elem->val);

		if (ret == 1)
			return 1;

		elem = elem->next;
	}

	V_visitato[n] = 2; // abbandono il nodo per sempre
	return 0;
}
 
void swap(int &a, int &b)
{
	int tmp = a;
	a = b;
	b = tmp;
	/// aggiorno contatore globale di swap
	ct_swap++;
}

int parse_cmd(int argc, char **argv)
{
	/// controllo argomenti
	int ok_parse = 0;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][1] == 'v')
		{
			details = 1;
			ok_parse = 1;
		}
		if (argv[i][1] == 'g')
		{
			graph = 1;
			ok_parse = 1;
		}
	}

	if (argc > 1 && !ok_parse)
	{
		printf("Usage: %s [Options]\n", argv[0]);
		printf("Options:\n");
		printf("  -verbose: Abilita stampe durante l'esecuzione dell'algoritmo\n");
		printf("  -graph: creazione file di dot con il grafo dell'esecuzione (forza d=1 t=1)\n");
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i, test;

	// init random
	srand((unsigned)time(NULL));

	if (parse_cmd(argc, argv))
		return 1;

	if (graph)
	{
		output_graph.open("graph.dot");
		/// preparo header
		output_graph << "digraph g" << endl;
		output_graph << "{ " << endl;
		output_graph << "node [shape=none]" << endl;
		output_graph << "rankdir=\"LR\"" << endl;
		;
		//    output_graph << "edge[tailclip=false,arrowtail=dot];"<<endl;
	}
	int N = 5;
	n_nodi = N * N;

	//// init nodi
	V = new int[n_nodi];
	V_visitato = new int[n_nodi];
	V_prev = new int[n_nodi];
	V_dist = new float[n_nodi];

	//// init archi
	E = new list_t *[n_nodi]; //(list_t**)malloc(n_nodi*sizeof(list_t*));

	// costruzione grafo
	for (int i = 0; i < n_nodi; i++)
	{
		V[i] = 2 * i;
		V_visitato[i] = 0; // flag = non visitato
		V_prev[i] = -1;	   // non c'e' precedente
		V_dist[i] = INFTY; // infinito
		heap[i] = INFTY;   // distanze nell'heap impostate a infinito
		++heap_size;
		heap_index[i] = i; // indice inserito in modo crescente
		printf("creato nodo %d, in posizione %d, con distanza %d\n", i, heap_index[i], heap[i]);

		E[i] = list_new();

		if (i == 0)
			global_ptr_ref = E[i];
		int x = i % N;
		int y = i / N;
		for (int dx = -1; dx <= 1; dx += 1)
		{
			for (int dy = -1; dy <= 1; dy += 1)
			{
				if (abs(dx) + abs(dy) == 1)
				{
					// limito gli archi ai vicini con una variazione assoluta sulle coordinate
					int nx = x + dx;
					int ny = y + dy;
					if (nx >= 0 && nx < N && ny >= 0 && ny < N)
					{			     // limito gli archi ai vicini con 1 variazione assoluta sulle coordinate
						int j = nx + N * ny; // indice del nuovo nodo
						list_insert_front(E[i], j, 50);
						// list_insert_front( E[i] , j,50*sqrt(abs(dx) + abs(dy)));
					}
				}
			}
		}
	}


	for (int i = 0; i < n_nodi; i++)
	{
		printf("Sono il nodo di indice %d nell'array\n", i);
		printf("Il valore del nodo e' %d\n", V[i]);
		printf("La lista di adiacenza e'\n");
		list_print(E[i]);
	}

	shortest_path(0);

	if (graph)
	{
		/// preparo footer e chiudo file
		output_graph << "}" << endl;
		output_graph.close();
		cout << " File graph.dot scritto" << endl
		     << "****** Creare il grafo con: neato graph.dot -Tpdf -o graph.pdf" << endl;
	}

	return 0;
}