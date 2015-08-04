/* 
 * File:   Mosquito.cpp
 * Author: amit
 * 
 * Created on August 25, 2014, 12:53 AM
 */


#include "Mosquito.h"
#include <sstream>

using namespace std;

unsigned Mosquito::getMosquitoID() const {
    return mID;
}

string Mosquito::getLocationID() const {
    return locationID;
}

void Mosquito::setLocation(std::string loc) {
    locationID = loc;
}

unsigned Mosquito::getDDay() const {
    return dday;
}

unsigned Mosquito::getMID() const {
    return mID;
}

Mosquito::MozState Mosquito::getState() const {
    return mState;
}

void Mosquito::setState(MozState st) {
    mState = st;
}

void Mosquito::setBiteStartDay(unsigned d) {
    biteStartDay = d;
}

unsigned Mosquito::getBiteStartDay() {
    return biteStartDay;
}

string Mosquito::printInfections() const {
    return "\n to do";
}

Mosquito::Mosquito(unsigned long id, unsigned bd, unsigned dd, string loc) {
    mID = id;
    bday = bd;
    locationID = loc;
    dday = dd;
    mState = Mosquito::MozState::BITE;
    biteStartDay = bd;
    infection.reset(nullptr);
    fly = false;
}

string Mosquito::toString() const {
    stringstream ss;
    ss << locationID << " " << mID <<" " << bday << " " << dday;
    infection->toString();
    return ss.str();
}

bool Mosquito::getFly() const {
    return fly;
}

void Mosquito::setFly(bool f) {
    fly = f;
}

Mosquito::Mosquito() {
}

Mosquito::Mosquito(const Mosquito& orig) {
}

Mosquito::~Mosquito() {
}

