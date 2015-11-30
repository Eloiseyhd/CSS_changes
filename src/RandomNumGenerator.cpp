#include "RandomNumGenerator.h"
#include <iostream>
#include <cmath>
#include <sstream>

using namespace std;

double RandomNumGenerator::getWaningTime(unsigned sero) {
    exponential_distribution<> d(1./(log(2) * halflife.at(sero)));
    return d(gen);
}

unsigned RandomNumGenerator::getMozEmerge(double mozMean) {    
    poisson_distribution<> dis(emergeFactor * mozMean);
    return dis(gen);
}

double RandomNumGenerator::getMozLifeSpan() {    
    exponential_distribution<> d(1./mozLife);
    return d(gen);
}

double RandomNumGenerator::getMozLatencyDays() {
    lognormal_distribution<> d(1.648721, 0.451754);
    return d(gen);
}

double RandomNumGenerator::getMozRestDays() {
    uniform_real_distribution<> dis(mozRestLo, mozRestHi);
    return dis(gen);
}

unsigned RandomNumGenerator::getMozNextLoc(unsigned num) {
    uniform_int_distribution<> dis(0, num-1);
    return dis(gen);
}

unsigned RandomNumGenerator::getHumanTrajectory() {
    uniform_int_distribution<> dis(0, 4);
    return dis(gen);
}

unsigned RandomNumGenerator::getHumanImmunity() {
    exponential_distribution<> d(1./huImmunity);
    return ceil(d(gen));
}

bool RandomNumGenerator::getHumanSeropositivity(double FOI, double age) {
    if(getEventProbability() < 1 - exp(-FOI * age)){
        return true;
    } else {
        return false;
    }
}

unsigned RandomNumGenerator::getRandomNum(unsigned num) {
    uniform_int_distribution<> dis(0, num-1);
    return dis(gen);
}

double RandomNumGenerator::getEventProbability() {
    uniform_real_distribution<> dis(0, 1);
    return dis(gen);
}

void RandomNumGenerator::setSeed(unsigned s) {
    seed = s;
    gen.seed(s);
}

unsigned RandomNumGenerator::intialInfDaysLeft() {
    uniform_int_distribution<> dis(1, 9);
    return dis(gen);
}

string RandomNumGenerator::toString() const {
    stringstream ss;
    ss <<"huLatencyLo:" << huLatencyLo;
    ss <<" huLatencyHi:" << huLatencyHi;
    ss <<" huImmunity:" << huImmunity;
    ss <<" emergeFactor:" << emergeFactor;
    ss <<" mozLife:" << mozLife;
    ss <<" mozLatencyLo:" << mozLatencyLo;
    ss <<" mozLatencyHi:" << mozLatencyHi;
    ss <<" mozRestLo:" << mozRestLo;
    ss <<" mozRestHi:" << mozRestHi;
    return ss.str();
}

RandomNumGenerator::RandomNumGenerator(
    unsigned s, unsigned hllo, unsigned hlhi, unsigned huImm, double efactor, double mlife,
    unsigned mllo, unsigned mlhi, unsigned mrestlo, unsigned mresthi, std::map<unsigned,double> hlife)
{
    seed = s;
    gen.seed(s);
    huLatencyLo = hllo;
    huLatencyHi = hlhi;
    huImmunity = huImm;
    emergeFactor = efactor;
    mozLife = mlife;
    mozLatencyLo = mllo;
    mozLatencyHi = mlhi;
    mozRestLo = mrestlo;
    mozRestHi = mresthi;
    halflife = hlife;
}

RandomNumGenerator::RandomNumGenerator() {
}

RandomNumGenerator::RandomNumGenerator(const RandomNumGenerator& orig) {
}

RandomNumGenerator::~RandomNumGenerator() {
}

