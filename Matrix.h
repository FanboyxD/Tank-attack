#include <iostream>
using namespace std;

#ifndef MATRIX_H
#define MATRIX_H



class Graph {
private:
    int** adjMatrix;
    int numVertices;

public:
    // Initialize the matrix to zero
    Graph(int numVertices) {
        this->numVertices = numVertices;
        adjMatrix = new int*[numVertices];
        for (int i = 0; i < numVertices; i++) {
            adjMatrix[i] = new int[numVertices];
            for (int j = 0; j < numVertices; j++)
                adjMatrix[i][j] = false;
        }
    }

    // Add edges
    void addEdge(int i, int j) {
        adjMatrix[i][j] = true;
        adjMatrix[j][i] = true;
    }

    // Remove edges
    void removeEdge(int i, int j) {
        adjMatrix[i][j] = false;
        adjMatrix[j][i] = false;
    }

    // Print the martix
    void toString() {
        for (int i = 0; i < numVertices; i++) {
            cout << i << " : ";
            for (int j = 0; j < numVertices; j++)
                cout << adjMatrix[i][j] << " ";
            cout << "\n";
        }
    }

    void alltrue() {
        for(int i = 0; i < numVertices; i++) {
            for(int j = 0; j < numVertices; j++) {
                adjMatrix[i][j] = true;
            }
        }
    }

    int xy(int i, int j) const{
        return adjMatrix[i][j];
    }
    int size() const{
        return numVertices;
    }

    ~Graph() {
        //for (int i = 0; i < numVertices; i++)
            //delete[] adjMatrix[i];
        //delete[] adjMatrix;
    }

};

Graph matrix(int vert) {
    Graph g(vert);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(2, 0);
    g.addEdge(2, 3);

    std::cout << g.size() << "\n";


    g.toString();


    return 0;
}

#endif //MATRIX_H
