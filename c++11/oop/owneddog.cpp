#include "owneddog.hpp"

OwnedDog::OwnedDog() : Dog() {
    std::cout << "A owneddog has been constructed\n";
}

void OwnedDog::setOwner(const std::string& dogsOwner)
{
    owner = dogsOwner;
}

void OwnedDog::print() const
{
    Dog::print(); 
    std::cout << "Dog is owned by " << owner << "\n";
}
