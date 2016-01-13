#include "Location.h"
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

string Location::getLocID() const {
    return locID;
}

string Location::getLocType() const {
    return locType;
}

double Location::getLocX() const {
    return xCor;
}

double Location::getLocY() const {
    return yCor;
}

double Location::getMozzes() const {
    return mozzes;
}

Location::Location(string lID, string lType, double x, double y, double m) {
    locID = lID;
    locType = lType;
    xCor = x;
    yCor = y;
    mozzes = m;
    closeLocs.reset(new vector<string>());
    //humans.reset(new vector<unique_ptr<Human>>());
}

string Location::getRandomCloseLoc(RandomNumGenerator& rGen) {
    
    int i = closeLocs->size();
    if (i > 0)
        return (*closeLocs)[rGen.getMozNextLoc(i)];
    else return "TOO_FAR_FROM_ANYWHERE";
}

double Location::getDistanceFromLoc(Location& loc) const {
    double d =  sqrt((xCor - loc.getLocX()) * (xCor - loc.getLocX()) + (yCor - loc.getLocY()) * (yCor - loc.getLocY()));
    //cout<< " d:" << d;
    return d;
}

void Location::addCloseLoc(string loc) {
    closeLocs->push_back(loc);
}

void Location::writeNeighToFile (std::ofstream& o) const{
    for (int i=0; i<closeLocs->size(); i++) {
        o << "," << (*closeLocs.get())[i];
    }
}

string Location::toString() const {
    stringstream ss;
    ss << locID << " " << locType << " " << xCor << " " << yCor << " neigh:";
    for(auto it=closeLocs->begin(); it!=closeLocs->end(); ++it) {
        ss << " " << *it;
    }
    return ss.str();
}

void Location::addHuman(Human * h) {
  humans.insert(h);
}

// void Location::printHumans() const {
//     for (int i=0; i<humans->size(); i++) {
//         cout << "\n" << (*humans.get())[i]->toString();
//     }
// }


Location::Location() {
}

Location::Location(const Location& orig) {
}

Location::~Location() {
}

