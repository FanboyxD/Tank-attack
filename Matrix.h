#include <iostream>
using namespace std;

#ifndef MATRIX_H
#define MATRIX_H

class Graph {
private:
    int** adjMatrix;  // Matriz de adyacencia que representa las conexiones entre vértices
    int numVertices;  // Número de vértices en el grafo

public:
    // Constructor que inicializa el grafo con un número de vértices dado
    // Crea una matriz de adyacencia inicializada a cero
    Graph(int numVertices) {
        this->numVertices = numVertices;
        adjMatrix = new int*[numVertices];  // Reserva memoria para la matriz de adyacencia
        for (int i = 0; i < numVertices; i++) {
            adjMatrix[i] = new int[numVertices];  // Reserva memoria para cada fila
            for (int j = 0; j < numVertices; j++)
                adjMatrix[i][j] = false;  // Inicializa la matriz en 0 (sin conexiones)
        }
    }

    // Función para añadir una arista entre los vértices 'i' y 'j'
    // Al ser un grafo no dirigido, se conecta en ambas direcciones
    void addEdge(int i, int j) {
        adjMatrix[i][j] = true;
        adjMatrix[j][i] = true;
    }

    // Función para eliminar una arista entre los vértices 'i' y 'j'
    // Desconecta los vértices en ambas direcciones
    void removeEdge(int i, int j) {
        adjMatrix[i][j] = false;
        adjMatrix[j][i] = false;
    }

    // Función para imprimir la matriz de adyacencia en formato legible
    void toString() {
        for (int i = 0; i < numVertices; i++) {
            cout << i << " : ";
            for (int j = 0; j < numVertices; j++)
                cout << adjMatrix[i][j] << " ";  // Imprime las conexiones del vértice 'i'
            cout << "\n";
        }
    }

    // Función que pone todas las entradas de la matriz de adyacencia en 'true'
    // Es decir, conecta todos los vértices entre sí (grafo completo)
    void alltrue() {
        for(int i = 0; i < numVertices; i++) {
            for(int j = 0; j < numVertices; j++) {
                adjMatrix[i][j] = true;
            }
        }
    }

    // Devuelve el valor en la posición (i, j) de la matriz de adyacencia
    // Es decir, si existe o no una conexión entre 'i' y 'j'
    int xy(int i, int j) const {
        return adjMatrix[i][j];
    }

    // Devuelve el número de vértices en el grafo
    int size() const {
        return numVertices;
    }

    // Destructor que libera la memoria reservada para la matriz de adyacencia
    ~Graph() {
        // Se comenta porque da problemas de doble liberación de memoria
        // for (int i = 0; i < numVertices; i++)
        //     delete[] adjMatrix[i];  // Libera cada fila
        // delete[] adjMatrix;  // Libera la matriz de punteros
    }
};

// Función auxiliar para crear un grafo y agregar algunas aristas
Graph matrix(int vert) {
    Graph g(vert);  // Crea un grafo con 'vert' vértices

    // Añade algunas aristas entre los vértices
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    g.addEdge(2, 3);

    // Imprime el número de vértices del grafo
    std::cout << g.size() << "\n";

    // Muestra la representación de la matriz de adyacencia
    g.toString();

    return 0;
}

#endif // MATRIX_H
