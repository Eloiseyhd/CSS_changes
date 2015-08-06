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

double Mosquito::getDDay() const {
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

void Mosquito::setBiteStartDay(double d) {
    biteStartDay = d;
}

double Mosquito::getBiteStartDay() {
    return biteStartDay;
}

string Mosquito::printInfections() const {
    return "\n to do";
}

void Mosquito::takeBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    if(infection == nullptr){
        infectingBite(time, locNow, rGen, currentDay, numDays);
    }
    else if(infection->getInfectiousness() > 0){
        infectiousBite(time, locNow, rGen, currentDay, numDays, out);
    }
        // else{
        //     if(infection != nullptr)






        //     setState(Mosquito::MozState::REST);
        //     setBiteStartDay(currentDay + rGen->getMozRestDays());
        //     return;
        // }

}



Human * Mosquito::whoBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen)
{
    std::map<Human *,double> humanMap;
    std::set<Human *>* humans = locNow->getHumans();
    std::set<Human *>::iterator itrHum = humans->begin();

    for(; itrHum != humans->end(); itrHum++){
        std::string currentLoc = (*itrHum)->getCurrentLoc(time);
        if(currentLoc == locationID){
            humanMap.insert(std::pair<Human *,double>(*itrHum,(*itrHum)->getAttractiveness()));
        }
    }

    if(humanMap.empty())
        return NULL;

    double attractivenessSum = 0;
    std::map<Human *,double>::iterator mapItr = humanMap.begin();
    for(; mapItr != humanMap.end(); mapItr++)
        attractivenessSum += mapItr->second;

    double biteWho = rGen->getEventProbability() * attractivenessSum;
    attractivenessSum = 0;
    for(mapItr = humanMap.begin(); attractivenessSum < biteWho; mapItr++)
        attractivenessSum += mapItr->second;
    mapItr--;

    return &(*mapItr->first);
}



void Mosquito::infectingBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    int currentDay,
    int numDays)
{
    Human * humBite = whoBite(time, locNow, rGen);

    if(humBite != NULL){
        if(humBite->infection != nullptr){
            if(rGen->getEventProbability() < humBite->infection->getInfectiousness()){
                int sday = currentDay + rGen->getMozLatencyDays();
                int eday = numDays + 1;
                infection.reset(new Infection(sday, eday, 0, humBite->infection->getInfectionType()));
            }
        }        

        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());
    }
}



void Mosquito::infectiousBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    Human * humBite = whoBite(time, locNow, rGen);

    if(humBite != NULL){
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
            if(rGen->getEventProbability() < infection->getInfectiousness()){
                int sday = currentDay + rGen->getHuLatencyDays();
                int eday = sday + 9;
                humBite->infection.reset(new Infection(sday, eday, 0, infection->getInfectionType()));
                humBite->setImmunityPerm(infection->getInfectionType(),true);
                humBite->setImmunityTemp(true);
                humBite->setImmStartDay(currentDay);
                humBite->setImmEndDay(currentDay + 9 + rGen->getHumanImmunity());
                *out << currentDay << "," << infection->getInfectionType() << "," << humBite->getHouseID() << "," << humBite->getHouseMemNum();                        
                *out << "," << humBite->getAge(currentDay) << "," << humBite->getGender() << "," << sday << "," << locNow->getLocID() << "\n";
            }
        }

        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());
    }
}



Mosquito::Mosquito(unsigned long id, unsigned bd, double dd, double bsd, string loc) {
    mID = id;
    bday = bd;
    locationID = loc;
    dday = dd;
    mState = Mosquito::MozState::BITE;
    biteStartDay = bsd;
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
