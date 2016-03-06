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

void Location::addHuman(Human * h) {
  humans.insert(h);
}


Location::Location() {
}

Location::Location(const Location& orig) {
}

Location::~Location() {
}
