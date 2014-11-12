#include <iostream>

#include "owneddog.hpp"

int main() {
    std::cout << "DOG:" << std::endl;
    Dog myDog;
    myDog.setName("Barkley");
    myDog.setWeight(10);
    myDog.print();
    std::cout << "OWNEDDOG:" << std::endl;
    OwnedDog ownedDog;
    ownedDog.setName("OwnedBarkley");
    ownedDog.setOwner("Ale");
    ownedDog.setWeight(20);
    ownedDog.print(); 
    return 0;
} 
