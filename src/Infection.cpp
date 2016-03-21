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
            infectiousness = exp(-0.08100656 * pow(double(currentDay - startDay) - 8.01514839, 2)) / 1.41486939;
        }
        else{
            infectiousness = exp(-0.08451596 * pow(double(currentDay - startDay) - 7.67747506, 2)) / 1.61700365;
        }
    }
    else{
        if(primary){
            infectiousness = exp(-0.07656046 * pow(double(currentDay - startDay) - 8.33716190, 2)) / 1.53965462;
        }
        else{
            infectiousness = exp(-0.08387661 * pow(double(currentDay - startDay) - 7.87813540, 2)) / 1.81578963;
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

