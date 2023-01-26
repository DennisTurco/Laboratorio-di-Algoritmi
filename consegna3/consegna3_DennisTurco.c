#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

// compilazione: g++ -xc++ consegna3_DennisTurco.c
// ./a.out -graph
// dot graph.dot -Tpdf -o graph.pdf

// Obiettivo:
// 1 - trasforma max heap in min heap 
//	   (con le operazioni heap_insert e remove_min, non serve decrease_key)
// 2 - crea l'algoritmo per il sorting di un array random in input
// 3 - misura le letture in memoria e confronta con una implementazione di 
//	   quicksort (ricicla il codice di quicksort con il lancio su 10 array di 
//	   dimensione da 100 a 1000, ripeti 1000 volte con array casuali)
// 4- produci un grafico del confronto letture quicksort - heapsort
// Consegna il codice c e il foglio xls con i grafici di confronto

int ct_swap = 0;
int ct_cmp = 0;
int ct_read = 0;
int ct_op = 0; /// operazioni per la ricerca

const int min_dim = 100;
const int max_dim = 1000;

int ntests = max_dim;
int ndiv = 1;
int details = 0;
int graph = 0;

int n = 0; /// dimensione dell'array

/// file di output per grafo
ofstream output_graph;
int n_operazione = 0; /// contatore di operazioni per visualizzare i vari step

const int MAX_SIZE = max_dim; /// allocazione statica
int heap[MAX_SIZE];
int heap_size = 0; /// dimensione attuale dell'heap

/// uso -1 per indicare un indice non esistente
int parent_idx(int n){
	if (n == 0)
		return -1;
	return (n - 1) / 2;
}

int child_L_idx(int n){
	if (2 * n + 1 >= heap_size)
		return -1;
	return 2 * n + 1;
}

int child_R_idx(int n){
	if (2 * n + 2 >= heap_size)
		return -1;
	return 2 * n + 2;
}

/// restituisce 0 se il nodo in posizione n e' un nodo interno (almeno un figlio)
/// restituisce 1 se il nodo non ha figli
int is_leaf(int n){
	return (child_L_idx(n) == -1); // non c'e' bisogno di controllare il figlio R
}

/// stampa il codice del nodo per dot
void print_node_code(int n){
	output_graph << "node_" << n << "_" << n_operazione;
}

void node_print_graph(int n){
	if (details)
		printf("Stampo su grafo nodo %d\n", n);
	print_node_code(n);
	output_graph << "\n[label=<\n<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" >\n<TR> <TD CELLPADDING=\"3\" BORDER=\"0\"  ALIGN=\"LEFT\" bgcolor=\"#f0f0f0\" PORT=\"id\">";
	output_graph << n << "</TD> </TR><TR>\n<TD PORT=\"val\" bgcolor=\"#a0FFa0\">";
	output_graph << heap[n] << "</TD>\n</TR></TABLE>>];\n";

	/// visualizzazione figli sullo stesso piano
	if (child_L_idx(n) != -1 && child_R_idx(n) != -1){
		output_graph << "rank = same; ";
		print_node_code(child_L_idx(n));
		output_graph << ";";
		print_node_code(child_R_idx(n));
		output_graph << ";\n";
	}

	// mostro archi uscenti

	if (child_L_idx(n) != -1){ /// disegno arco left
		print_node_code(n);
		output_graph << ":s -> ";
		print_node_code(child_L_idx(n));
		output_graph << ":id ;\n";
	}

	if (child_R_idx(n) != -1){ /// disegno arco R
		print_node_code(n);
		output_graph << ":s -> ";
		print_node_code(child_R_idx(n));
		output_graph << ":id ;\n";
	}
}

void tree_print_rec_graph(int n){
	if (n != -1){
		node_print_graph(n);
		tree_print_rec_graph(child_L_idx(n));
		tree_print_rec_graph(child_R_idx(n));
	}
}

void tree_print_graph(int n){
	/// stampa ricorsiva del nodo
	tree_print_rec_graph(n);
	n_operazione++;
}

void node_print(int n){
	if (n == -1)
		printf("nodo vuoto\n");
	else
		printf("allocato in %d [Val: %d, L: %d, R: %d]\n",
			   n,
			   heap[n],
			   child_L_idx(n),
			   child_R_idx(n));
}

void sistema_relazione_genitori(int &indice_i, int &indice_p, int &val_i, int &val_p){ // passando indice_i temp come & evito di eseguire n letture in piu'	
	
	while (indice_p>=0){
		if(details)
			printf("idx %d (val %d), parent %d (val %d)\n", indice_i, val_i, indice_p, val_p);

		if (val_i < val_p){ //constrollo se il valore appena aggiunto e' minore del padre e se si li swappo (siamo nel caso min heap)
			// swap
			swap(val_i, val_p);

			//preparo per la prosima iterazione
			indice_i = indice_p;
			indice_p = parent_idx(indice_i);
			
			++ct_read;
			++ct_read;
			sistema_relazione_genitori(indice_i, indice_p, heap[indice_i], heap[indice_p]); // devo mettere heap[indice_i] e heap[p] perche' mettendo val_i e val_p non si aggiornano
		} 
		else break;
	}
}

// ora siamo nel caso del min heap
void heap_insert(int elem){
	/// inserisco il nuovo nodo con contenuto elem
	/// nell'ultima posizione dell'array
	/// ovvero continuo a completare il livello corrente

	if (details)
		printf("Inserisco elemento %d in posizione %d\n", elem, heap_size);

	if (heap_size < MAX_SIZE){ // caso in cui l'heap ha ancora posto per contenere altri valori
		int i = heap_size; //i ha valore di indice maggiore
		heap_size++;
		//heap[i] = elem;
		int p = parent_idx(i); //assegno a p, l'indice del padre di i
		sistema_relazione_genitori(i, p, heap[i]=elem, heap[p]);
		ct_read++;		
		ct_read++;		

	} else
		if(details)
			printf("Heap pieno!\n");
}

// ora siamo nel caso del min heap
void decrese_key(int indice_nodo, int key){
	// key = nuovo valore

	ct_read++;
	int &temp_heap = heap[indice_nodo]; //serve per fare meno letture

	if (indice_nodo < 0 || indice_nodo >= heap_size){
		printf("Nodo non esistente\n");
		return;
	}

	temp_heap = key;

	tree_print_graph(0); // radice
	n_operazione++;

	int i = indice_nodo;
	int p = parent_idx(i);
	sistema_relazione_genitori(i, p, heap[i], heap[p]);
	ct_read++;		
	ct_read++;		

}

// ora siamo nel caso del min heap
int heap_remove_min(){

	if (heap_size <= 0){ /// heap vuoto!
		printf("Errore: heap vuoto\n");
		return -1;
	}

	ct_read++;
	ct_read++;
	int &temp_heapMinimo = heap[0];
	int &temp_heapSize = heap[heap_size-1];

	int minimo = temp_heapMinimo; //il minimo e' il primo elemento dell'array

	if (details)
		printf("Minimo identificato %d\n", minimo);

	/// scambio la radice con l'ultima foglia a destra
	/// il minimo e' stato spostato in fondo --> pronto per l'eliminazione
	swap(temp_heapMinimo, temp_heapSize);

	tree_print_graph(0);  // radice
	n_operazione++;

	// elimino il minimo (ora in fondo all'array)
	heap_size--;

	/// nella radice c'e' un valore grande (massimo?) -> si se disposto in ordine decrescente
	int i = 0; // indice di lavoro (parto dalla root)

	while (i>=0 && !is_leaf(i)){ /// garantisco di fermarmi alla foglia
		
		ct_read++;
		int &temp_heap = heap[i]; // temp

		if (details)
			printf("Lavoro con il nodo in posizione i = %d, valore %d\n", i, temp_heap);

		int con_chi_mi_scambio = -1; /// -1 === non mi scambio con nessuno

		int child_L = child_L_idx(i);
		int child_R = child_R_idx(i);

		ct_read++;
		int &temp_childL = heap[child_L]; // temp

		/// max L R --> indice
		if (temp_childL < temp_heap) { 		/// confronto i con L
			con_chi_mi_scambio = child_L;
		}

		///considero figlio destro
		if(child_R >= 0){	// esiste R

			ct_read++;
			int &temp_childR = heap[child_R];
			
			/// controllare L <-> R e i <-> R
			if (temp_childR < temp_childL && temp_childR < temp_heap){
				con_chi_mi_scambio = child_R;
			}
		
		}

		if (con_chi_mi_scambio>=0){
			
			ct_read++;
			int &temp_heapScambio = heap[con_chi_mi_scambio]; // questo passaggio in realta' non mi fa risparmiare letture perche' usato una sola volta e quando passato a funzione viene gia' trattato come &

			/// swap tra i e con_chi_mi_scambio
			swap(temp_heap, temp_heapScambio);
		}

		i = con_chi_mi_scambio;
	}

	return minimo;
}

// quick sort
int partition(int *A, const int &p, const int &r){
    
    // copia valori delle due meta p..q e q+1..r
	ct_read++;
    int &x = A[r];
    int i = p-1;

    for (int j=p; j<r; j++){

		ct_read++;
		int &temp = A[j];
        if (temp <= x){
            i++;
			ct_read++;
            swap(A[i], temp);
        }
    }
	ct_read++;
    swap(A[i+1], x);

    return i+1;
}

void quickSort(int *A, const int &p, const int &r){
    if (p < r){
        int q = partition(A, p, r);
        quickSort(A, p, q-1);
        quickSort(A, q+1, r);
    }
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

void print_array(int *A, const int &dim){
	for (int i=0; i<dim; i++){
		printf("%d ", A[i]);
	}
	printf("\n");
}

void sort(int *A, const int &dim){
	for(int i=0; i<dim; i++){
		ct_read++;
		int &temp1 = A[i];
		for (int j=i+1; j<dim; j++){
			ct_read++;
			int &temp2 = A[j];
			if(temp1 > temp2){
				swap(temp1, temp2);
			}
		}	
	}
}

int main(int argc, char **argv){
	int i, test; 
	int array_dim; // mantiene traccia della dimensione dell'array

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

	// PUNTO 2
	int read_min = -1;
    int read_max = -1;
    long read_avg = 0;

	int heap_read_min = -1;
    int heap_read_max = -1;
    long heap_read_avg = 0;
	
	int heap_ct_swap = 0;
    int heap_ct_cmp = 0;
    int heap_ct_read = 0;

	array_dim = 1000;
	int *A_punto2 = new int[array_dim];
	for(int i=0; i<array_dim; i++){
		A_punto2[i] = i + rand() % 10;
		heap_insert(A_punto2[i]);
	}
	sort(A_punto2, array_dim); //attraverso questo algoritmo di sorting, non mi interessa essere efficiente (faccio molte letture in memoria), ma voglio semplicemente ottenere un algoritmo semplice da scrivere
	printf("\n------------ Punto 2 ------------\n");
	print_array(A_punto2, array_dim);
	delete []A_punto2;	


	// PUNTO 3
	printf("\n------------ Punto 3 ------------\n");
	
	printf("Sort Statistiche: \n");
	printf("Letture effettuate: %d\n\n", ct_read);

	
	//// lancio su 10 array (10 test)
	printf("QuickSort Statistiche: \n");
	for (test = 0; test < 10; test++){

		// azzero le statistiche per il QuickSort
		ct_swap = 0;
        ct_cmp = 0;
        ct_read = 0;
		read_min = -1;
		read_max = -1;
		read_avg = 0;

		// azzero le statistiche per l'heap
		heap_ct_swap = 0;
        heap_ct_cmp = 0;
        heap_ct_read = 0;
		heap_read_min = -1;
		heap_read_max = -1;
		heap_read_avg = 0;

		array_dim = rand() % max_dim-min_dim + min_dim; // valori da 100 a 1000
		int *A = new int[array_dim];
		printf("Grandezza array: %d\n", array_dim);

		// lancio ntests volte per coprire diversi casi di input random
		for (int i = 0;  i < ntests; i++){

			heap_size = 0;

			if(details) printf("generato array di dimensione: %d\n", array_dim);

			//inizializzazzione heap ed array
			for (int j=0; j<array_dim; j++){
				//    heap_insert(rand()%100);
				A[j] = rand() % 100; // inizializzazzione random da 0 a 100;
				heap_insert(A[j]);
			}

			/// statistiche heap
			heap_read_avg += ct_read;
			if (heap_read_min < 0 || heap_read_min > ct_read)
				heap_read_min = ct_read;
			if (heap_read_max < 0 || heap_read_max < ct_read)
				heap_read_max = ct_read;
			if(details) printf("Test Heap - %d %d\n", test, ct_read);

			ct_read = 0;

			// algoritmo di sorting
			quickSort(A, 0, array_dim);

			/// statistiche QuickSort
			read_avg += ct_read;
			if (read_min < 0 || read_min > ct_read)
				read_min = ct_read;
			if (read_max < 0 || read_max < ct_read)
				read_max = ct_read;
			if(details) printf("Test QuickSort - %d %d\n", test, ct_read);

			ct_read = 0;
			
		}
		printf("  Heap    -> Numero test: %d, Letture minime: %d, Letture media: %.1f, Letture massime: %d\n", test, heap_read_min, (0.0 + heap_read_avg)/ntests, heap_read_max);
		printf("QuickSort -> Numero test: %d, Letture minime: %d, Letture media: %.1f, Letture massime: %d\n\n", test, read_min, (0.0 + read_avg)/ntests, read_max);

		delete []A;
	}

	tree_print_graph(0); // radice
	n_operazione++;

	//decrese_key(3, 2); //diminuisce il valore di una chiave in posizione 3(in questo caso)
	
	// rimozione nodo
	/*for (int i=0; i<2; i++){
		int valore = heap_remove_min();
		printf("Il valore minimo estratto e' %d\n",valore);

		tree_print_graph(0);  // radice
		n_operazione++;
	}*/

	// visualizzazione dell'array --> heapsort!
	if (details){
		for (int i = 0; i < array_dim; i++){
			printf("%d ", heap[i]);
		}
		printf("\n");
	}
		
	if (graph){
		/// preparo footer e chiudo file
		output_graph << "}" << endl;
		output_graph.close();
		cout << " File graph.dot scritto" << endl
			<< "Creare il grafo con: dot graph.dot -Tpdf -o graph.pdf" << endl;
	}	

	return EXIT_SUCCESS;
}
