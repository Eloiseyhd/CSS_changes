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
    RandomNumGenerator * rGenInf,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    if(infection == nullptr){
      infectingBite(time, locNow, rGen, rGenInf, currentDay, numDays);
    }
    else if(infection->getInfectiousness() > 0.0){
      infectiousBite(time, locNow, rGen, rGenInf, currentDay, numDays, out);
    }
}


Human * Mosquito::whoBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen)
{
    std::map<Human *,double,Human::sortid> humanMap;
    std::set<Human *,Human::sortid>* humans = locNow->getHumans();
    std::set<Human *,Human::sortid>::iterator itrHum = humans->begin();
    for(; itrHum != humans->end(); itrHum++){
        std::string currentLoc = (*itrHum)->getCurrentLoc(time);
        if(currentLoc == locationID){
            humanMap.insert(std::pair<Human *,double>(*itrHum,(*itrHum)->getAttractiveness()));
	    //	    printf("Mosquito %lu in loc %s can bite human %s-%d\n",mID,locNow->getLocID().c_str(),(*itrHum)->getHouseID().c_str(),(*itrHum)->getHouseMemNum());
        }
    }

    if(humanMap.empty())
        return NULL;

    double attractivenessSum = 0;
    std::map<Human *,double,Human::sortid>::iterator mapItr = humanMap.begin();
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
    RandomNumGenerator * rGenInf,
    int currentDay,
    int numDays)
{
    Human * humBite = whoBite(time, locNow, rGen);

    if(humBite != NULL){
      //      printf("Mosquito %lu in loc %s bit human %s-%d\n",mID,locNow->getLocID().c_str(),humBite->getHouseID().c_str(),humBite->getHouseMemNum());
        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());
        if(humBite->infection != nullptr){
            humBite->infection->setInfectiousnessHuman(currentDay);
	    //	    printf("Possible infection for Mosquito from human %s-%d\n",humBite->getHouseID().c_str(),humBite->getHouseMemNum());
            if(rGenInf->getEventProbability() < humBite->infection->getInfectiousness()){
                double sday = double(currentDay) + rGenInf->getMozLatencyDays();
                int eday = numDays + 1;
                infection.reset(new Infection(
                    round(sday), eday, 0.0, humBite->infection->getInfectionType(), 0, 0));
		//		printf("Mosquito bit human %s-%d is infected\n",humBite->getHouseID().c_str(),humBite->getHouseMemNum());
            }
        }        
    }else{
      //      printf("Mosquito %lu in loc %s could not find anybody to bite\n",mID,locNow->getLocID().c_str());
    }
}



void Mosquito::infectiousBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    RandomNumGenerator * rGenInf,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    Human * humBite = whoBite(time, locNow, rGen);

    // if someone is found to bite
    if(humBite != NULL){
        // update the mosquito's status
        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());

        // if the mosquito is infectious, the human not actively infected, and the infection successful
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
	  double RRInf = humBite -> getRRInf();
	  if(rGenInf->getEventProbability() < infection->getInfectiousness() * (1 - RRInf)){
                humBite->infect(currentDay, infection->getInfectionType(), rGenInf);
            }
        }
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
