/* 
 * File:   Infection.cpp
 * Author: amit
 * 
 * Created on August 28, 2014, 1:56 AM
 */

#include "Infection.h"
#include <sstream>

using namespace std;

int Infection::getStartDay() const {
    return startDay;
}
int Infection::getEndDay() const {
    return endDay;
}
double Infection::getInfectiousness() const {
    return infectiousness;
}
Infection::InfType Infection::getInfectionType() const {
    return infType;
}
void Infection::setInfectiousness(double in) {
    infectiousness = in;
}

string Infection::toString() const {
    stringstream ss;
    ss << infType << " " << startDay <<" " << endDay << " " << infectiousness;
    return ss.str();
}

Infection::Infection(unsigned sd, unsigned ed, double infn, Infection::InfType ityp) {
    startDay = sd;
    endDay = ed;
    infectiousness = infn;
    infType = ityp;
}

Infection::Infection() {
}

Infection::Infection(const Infection& orig) {
    startDay = orig.startDay;
    endDay = orig.endDay;
    infectiousness = orig.infectiousness;
    infType = orig.infType;
}

Infection::~Infection() {
}

