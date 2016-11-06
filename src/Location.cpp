#include "Location.h"
#include <sstream>
#include <iostream>
#include <cmath>

//using namespace std;

string Location::getLocID() const {
    return locID;
}

string Location::getLocType() const {
    return locType;
}

string Location::getNeighID() const {
    return neighborhoodID;
}

double Location::getLocX() const {
    return xCor;
}

double Location::getLocY() const {
    return yCor;
}

double Location::getEmergenceRate() const {
    return emergenceRate;
}

Location::Location(string lID, string lType, double x, double y, double e) {
    locID = lID;
    locType = lType;
    xCor = x;
    yCor = y;
    emergenceRate = e;
    closeLocs.reset(new vector<string>());
    infectedVisitor = false;
}

Location::Location(string lID, string lType, string NID, double x, double y, double e) {
    locID = lID;
    locType = lType;
    neighborhoodID = NID;
    xCor = x;
    yCor = y;
    emergenceRate = e;
    closeLocs.reset(new vector<string>());
    infectedVisitor = false;
}

string Location::getRandomCloseLoc(RandomNumGenerator& rGen) {
    int i = closeLocs->size();
    if (i > 0)
        return (*closeLocs)[rGen.getMozNextLoc(i)];
    else return "TOO_FAR_FROM_ANYWHERE";
}

double Location::getDistanceFromLoc(Location& loc) const {
    return sqrt((xCor - loc.getLocX()) * (xCor - loc.getLocX()) + (yCor - loc.getLocY()) * (yCor - loc.getLocY()));
}

void Location::addCloseLoc(string loc) {
    closeLocs->push_back(loc);
}

void Location::addHuman(sp_human_t h) {
    humans.insert(h);
}

void Location::removeHuman(sp_human_t h){
    // erase requires no check
    humans.erase(h);
}

//Location::Location() {
//}

//Location::Location(const Location& orig) {
//}

//Location::~Location() {
//}



void Location::updateInfectedVisitor(){
    infectedVisitor = false;
    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
        if((*itHum)->infection != nullptr){
            infectedVisitor = true;
            return;
        }
    }
}




