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

unsigned Infection::getInfectionType() const {
    return infType;
}

void Infection::setInfectiousnessMosquito(double in){
    infectiousness = in;
}

void Infection::setInfectiousnessHuman(int currentDay){
    if(symptomatic){
        if(primary){
            infectiousness = exp(-0.204981 * pow(double(currentDay - startDay) - 5.538213, 2)) / 0.9999993;
        }
        else{
            infectiousness = exp(-0.3836628 * pow(double(currentDay - startDay) - 5.8745411, 2)) / 0.9999921;
        }
    }
    else{
        if(primary){
            infectiousness = exp(-0.2503908 * pow(double(currentDay - startDay) - 5.5852691, 2)) / 0.9999944;
        }
        else{
            infectiousness = exp(-0.5841903 * pow(double(currentDay - startDay) - 4.8839302, 2)) / 0.999991;
        }
    }
}

string Infection::toString() const {
    stringstream ss;
    ss << infType << " " << startDay <<" " << endDay << " " << infectiousness;
    return ss.str();
}

Infection::Infection(unsigned sd, unsigned ed, double infn, unsigned ityp, bool prim, bool symp) {
    startDay = sd;
    endDay = ed;
    infectiousness = infn;
    infType = ityp;
    primary = prim;
    symptomatic = symp;
}

Infection::Infection() {
}

Infection::Infection(const Infection& orig) {
    startDay = orig.startDay;
    endDay = orig.endDay;
    infectiousness = orig.infectiousness;
    infType = orig.infType;
    primary = orig.primary;
    symptomatic = orig.symptomatic;
}

Infection::~Infection() {
}

