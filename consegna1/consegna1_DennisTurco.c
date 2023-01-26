#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

// compilazione: g++ -xc++ consegna1_DennisTurco.c

// Il programma carica il file data.txt contenente 107 righe con i dati di contagi covid per provincia
// ./a.out
// In output viene mostrato il numero di accessi in read alla memoria per eseguire il sorting di ciascuna riga

// Obiettivo:
// Creare un algoritmo di sorting che minimizzi la somma del numero di accessi per ogni sorting di ciascuna riga del file

int ct_swap = 0;
int ct_cmp = 0;
int ct_read = 0;

int max_dim = 0;
int ntests = 107; //numero massimo di test
int ndiv = 1;
int details = 0;
int graph = 0;

int n = 0; /// dimensione dell'array

// stampa array in input
void print_array(int *A, const int &dim){
    for (int i = 0; i < dim; i++){
        printf("%d ", A[i]);
    }
    printf("\n");
}

//scambia la posizione di 2 elementi
void swap(const int *a, const int *b){
    const int *tmp = a;
    a = b;
    b = tmp;
    /// aggiorno contatore globale di swap
    ct_swap++;
}


void countingSort(int *A, const int &index, const int &pivot){

    int *B = new int[index];
    int *C = new int[pivot+1];

    // A: array in input 0..n-1
    // B: array in output 0..n-1
    // C: array per conteggi 0..max
    // index: elementi da ordinare
    // pivot: valore massimo contenuto in A

    for (int i = 0; i <= pivot; i++){ // pre-inizializzo l'array dei conteggi a 0
        C[i] = 0; 
    }

    for (int j = 0; j < index; j++){ // conteggio istogramma
        ct_read++;
        ct_read++;
        ++C[A[j]]; // salva nelle posizioni da 0 a pivot di C quanti elementi hanno valore uguale all'indice dell'array 
    }

    for (int i = 1; i <= pivot; i++){ // C[i] contiene il numero di elementi <= i  --> da conteggio ad accumolo
        ct_read++;
        ct_read++;
        C[i] += C[i-1];
    }

    //utilizzo temp e temp2 per diminuire il numero di letture
    //per ogni elemento originale in A -> mi chiedo nel conteggio C quanti sono gli elementi minori o uguali:  questo corrisponde alla posizione dell'elemento in B
    for (int j = index-1; j >= 0; j--){ 
        ct_read++;
        ct_read++;
        ct_read++;
        int &temp = A[j];
        int &temp2 = C[temp];
        B[temp2 - 1] = temp;
        temp2 = temp2 - 1;
    }

    //questo passaggio lo faccio solo per comodita' quindi non conto le letture
    for (int i = 0; i < index; i++){
        A[i] = B[i];
    }

    // dealloco gli array di supporto
    delete [] C;
    delete [] B;
}

int partition(int *A, const int &p, const int &r){

    // copia valori delle due meta p..q e q+1..r
    ct_read++;
    int &x = A[r];
    int i = p - 1;

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
    swap(A[i + 1], x);

    return i + 1;
}

void quickSort(int *A, const int &p, const int &r){
    if (p < r){
        int q = partition(A, p, r);
        quickSort(A, p, q - 1);
        quickSort(A, q + 1, r);
    }
}

void insertion_sort(int *A1, const int &index1){ // PARAMETRI: array,dimensione

    for (int i = 0; i < index1; i++){ // per evitare un passo in piu' si potrebbe inizializzare i = 1

        for (int j = i - 1; j >= 0; j--){ // si poteva evitare di mettere un for con gli if mettendo il while
            

            //uso il sistema di passaggio a temp per evitare letture in piu'
            int &temp1 = A1[j];
            int &temp2 = A1[j+1];
            ct_read++;
            ct_read++;
            if (temp1 > temp2){
                swap(temp1, temp2); // TRAMITE LA FUNZIONE NON UTILIZZO UNA VARIABILE TEMPORANEA MA FACCIO LO SCAMBIO VERSO SINISTRA SE C'è BISOGNO
            }
            else break;
        }
    }
}

int parse_cmd(int argc, char **argv){

    /// parsing argomento
    max_dim = 743;

    for (int i = 1; i < argc; i++){
        if (argv[i][1] == 'd')
            ndiv = atoi(argv[i] + 3);
        if (argv[i][1] == 't')
            ntests = atoi(argv[i] + 3);
        if (argv[i][1] == 'v')
            details = 1;
        if (argv[i][1] == 'g'){
            graph = 1;
            ndiv = 1;
            ntests = 1;
        }
    }

    return 0;
}


void split(const int *A, int *A1, int *A2, int *A3, const int &n, const int &pivot, int &index1, int &index2, int &index3){
    for (int i = 0; i < n; i++){
        ct_read++;
        int temp = A[i];
        
        if (temp >= 0 && temp <= pivot){
            ct_read++;
            A2[index2] = temp;
            index2++;
        }
        else if (temp > pivot){
            ct_read++;
            A3[index3] = temp;
            index3++;
        }
        else{
            ct_read++;
            A1[index1] = temp;
            index1++;
        }
    }
}

void merge(int *A, const int *A1, const int *A2, const int *A3, const int &index1, const int &index2, const int &index3){
    int index = 0;
    for (int i = 0; i < index1; i++){
        ct_read++;
        ct_read++;
        A[index] = A1[i];
        index++;
    }
    for (int i = 0; i < index2; i++){
        ct_read++;
        ct_read++;
        A[index] = A2[i];
        index++;
    }
    for (int i = 0; i < index3; i++){
        ct_read++;
        ct_read++;
        A[index] = A3[i];
        index++;
    }
}

int main(int argc, char **argv){
    int i, test;
    int *A;

    if (parse_cmd(argc, argv))
        return 1;

    /// allocazione array
    A = new int[max_dim];
    n = max_dim;

    ifstream input_data;
    input_data.open("data.txt");

    int read_min = -1;
    int read_max = -1;
    long read_avg = 0;

    //// lancio ntests volte per coprire diversi casi di input random
    for (test = 0; test < ntests; test++){

        /// inizializzazione array: numeri random con range dimensione array
        for (i = 0; i < n; i++){
            char comma;
            input_data >> A[i];
            input_data >> comma;
        }

        if (details){
            printf("caricato array di dimensione %d\n", n);
            print_array(A, n);
        }

        ct_swap = 0;
        ct_cmp = 0;
        ct_read = 0;

        int *A1 = new int[max_dim]; // per i negativi
        int *A2 = new int[max_dim]; // per i valori [0, pivot]
        int *A3 = new int[max_dim]; // per i vallori > pivot

        int index1 = 0, index2 = 0, index3 = 0; // indici dei sotto array
        int pivot = 300;

        // PASSO 1 -> suddivido l'asrray di partenza in 3 sotto vettori corrispondenti alle 3 fasce
        split(A, A1, A2, A3, n, pivot, index1, index2, index3);

        // PASSO 2 -> insertion sort per i valori negativi
        if(index1 > 0) insertion_sort(A1, index1);

        // PASSO 3 -> eseguo un counting per i valori che vanno da 0 a pivot di range
        if(index2 > 0) countingSort(A2, index2, pivot);

        // PASSO 4 -> quick sort da pivot1+1 in avanti
        if(index3 > 0) quickSort(A3, 0, index3-1);

        // PASSO 5 -> unificare i 3 sottoArray in uno solo
        merge(A, A1, A2, A3, index1, index2, index3);

        delete []A1;
        delete []A2;
        delete []A3;

        if (details){
            printf("Output:\n");
            print_array(A, n);
        }

        /// statistiche
        read_avg += ct_read;
        if (read_min < 0 || read_min > ct_read)
            read_min = ct_read;
        if (read_max < 0 || read_max < ct_read)
            read_max = ct_read;
        printf("Test %d %d\n", test, ct_read);
    }

    printf("Numero test: %d, Letture minime: %d, Letture media: %.1f, Letture massime: %d\n", ntests, read_min, (0.0 + read_avg) / ntests, read_max);
    
    delete []A;
    return 0;
}