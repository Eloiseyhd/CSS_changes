#include "RandomNumGenerator.h"
#include <iostream>
#include <cmath>
#include <sstream>

using namespace std;

double RandomNumGenerator::getWaningTime(unsigned wan) {
  //    exponential_distribution<> d(1./(log(2) * halflife.at(sero)));
    exponential_distribution<> d(1./wan);
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
    exponential_distribution<> d(mozRest);
    return d(gen);
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

unsigned RandomNumGenerator::getVaxHumanImmunity(unsigned immdays) {
  exponential_distribution<> d(1./immdays);
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

double RandomNumGenerator::getRandomNormal(){
    normal_distribution<> d(0.0, 1.0);
    return d(gen);
}

double RandomNumGenerator::getEventProbability() {
    uniform_real_distribution<> dis(0, 1);
    return dis(gen);
}

void RandomNumGenerator::setSeed(unsigned s) {
    seed = s;
    gen.seed(s);
}

string RandomNumGenerator::toString() const {
    stringstream ss;
    ss <<" huImmunity:" << huImmunity;
    ss <<" emergeFactor:" << emergeFactor;
    ss <<" mozLife:" << mozLife;
    ss <<" mozRest:" << mozRest;
    return ss.str();
}

RandomNumGenerator::RandomNumGenerator(
    unsigned s, unsigned huImm, double efactor, double mlife,
    double mbite, std::map<unsigned,double> hlife)
{
    seed = s;
    gen.seed(s);
    huImmunity = huImm;
    emergeFactor = efactor;
    mozLife = mlife;
    mozRest = mbite;
    halflife = hlife;
}

RandomNumGenerator::RandomNumGenerator() {
}

RandomNumGenerator::RandomNumGenerator(const RandomNumGenerator& orig) {
}

RandomNumGenerator::~RandomNumGenerator() {
}
