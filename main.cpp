#include <iostream>
#include "ArrayList.h"
#include "LinkedList.h"

int main() {
    LinkedList<int> list;

    // Agregar elementos
    list.add(10);
    list.add(20);
    list.add(30);
    list.add(40);

    std::cout << "Lista inicial:\n";
    std::cout << "First: " << list.first() << "\n";  // 10
    std::cout << "Last: " << list.last() << "\n";    // 40
    std::cout << "Size: " << list.size() << "\n\n"; // 4

    // Mover current con next()
    std::cout << "Moviendo current con next():\n";
    try {
        list.next(); // current = 20
        list.next(); // current = 30
        list.next(); // current = 40
        list.next(); // Exception: Next is null
    }
    catch (const std::out_of_range& e) {
        std::cout << "Excepción: " << e.what() << "\n";
    }

    // Mover current con prior()
    std::cout << "\nMoviendo current con prior():\n";
    try {
        list.prior(); // current = 30
        list.prior(); // current = 20
        list.prior(); // current = 10
        list.prior(); // Exception: Prev is null
    }
    catch (const std::out_of_range& e) {
        std::cout << "Excepción: " << e.what() << "\n";
    }

    // Eliminar elementos
    std::cout << "\nEliminando elementos:\n";
    list.next(); // current = 20
    list.remove(); // elimina 20
    std::cout << "El tamaño después de eliminar 20: " << list.size() << "\n";
    std::cout << "Current ahora apunta a: " << list.get(1) << "\n"; // debería ser 30

    // Eliminar head
    list.prior(); // current = 10
    list.remove(); // elimina 10
    std::cout << "El tamaño después de eliminar head: " << list.size() << "\n";
    std::cout << "First ahora: " << list.first() << "\n"; // debería ser 30

    // Eliminar tail
    list.next(); // current = 40
    list.remove(); // elimina 40
    std::cout << "El tamaño después de eliminar tail: " << list.size() << "\n";
    std::cout << "Last ahora: " << list.last() << "\n"; // debería ser 30

    // Eliminar el último nodo
    list.remove(); // elimina 30
    std::cout << "Tamaño final: " << list.size() << "\n"; // 0

    /*ArrayList<int> lista;

    lista.add(10);
    lista.add(20);
    lista.add(30);
    lista.add(40);
    lista.add(50);
    lista.add(60);
    lista.add(70);
    lista.add(80);
    lista.add(90);

    for (int i = 0; i < lista.size(); i++) {
        std::cout << "Indice: " << std::endl;
        std::cout << i << std::endl;
        std::cout << lista.get(i) << std::endl;
    }

    std::cout << "Primer Indice:" << std::endl;
    std::cout << "0" << std::endl;
    std::cout << lista.first() << std::endl;

    std::cout << "Ultimo Indice:" << std::endl;
    std::cout << lista.size()-1 << std::endl;
    std::cout << lista.last() << std::endl;*/

    return 0;
}