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

        // if the mosquito is infectious, the human susceptible, and the infection successful
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
            if(rGenInf->getEventProbability() < infection->getInfectiousness()){
                // clinical disease state
                int disease = 0;
                int hospitalized = 0;
                int vaxAdvancement = 0;
		if(humBite->isVaccinated()){
		  vaxAdvancement = 1;
                }
                if(humBite->getPreviousInfections() + vaxAdvancement == 0){
                    if(rGenInf->getEventProbability() < 0.3){
                        disease = infection->getInfectionType();
                        if(rGenInf->getEventProbability() < 0.111){
                            hospitalized = infection->getInfectionType();
                        }
		    }
                }else if(humBite->getPreviousInfections() + vaxAdvancement == 1){
                    if(rGenInf->getEventProbability() < 0.6){
                        disease = infection->getInfectionType();
                        if(rGenInf->getEventProbability() < 0.20868){
                            hospitalized = infection->getInfectionType();
                        }
                    }
                }else{
                    if(rGenInf->getEventProbability() < 0.1){
                        disease = infection->getInfectionType();
                        if(rGenInf->getEventProbability() < 0.05217){
                            hospitalized = infection->getInfectionType();
                        }
                    }
                }

                // effect of vaccine on preventing infection and disease
                if(humBite->isVaccinated()){
                    if(rGenInf->getEventProbability() < humBite->getVE(infection->getInfectionType())){
                        if(humBite->getVaccinationDay() + rGenInf->getWaningTime(infection->getInfectionType()) > currentDay){
                            return;
                        } else {
                            humBite->waneVaccination();
                        }
                    } else {
                        if(disease > 0 && rGenInf->getEventProbability() < humBite->getVE(infection->getInfectionType()))
                            disease = 0;   
                    }
                }
		//		printf("Possible infection to human: %s - %d\n", humBite->getHouseID().c_str(),humBite->getHouseMemNum());
                // check to see whether the human is immune
                if(humBite->isImmune(infection->getInfectionType()))
		  //		  printf("Is immune. Human: %s - %d\n", humBite->getHouseID().c_str(),humBite->getHouseMemNum());
                    return;

                // record infection and update immune status
                int sday = currentDay + 1;
                int eday = sday + 14;
                humBite->infection.reset(new Infection(
                    sday, eday, 0.0, infection->getInfectionType(), humBite->getPreviousInfections() == 0, disease > 0));
                humBite->updateImmunityPerm(infection->getInfectionType(),true);
                humBite->setImmunityTemp(true);
                humBite->setImmStartDay(currentDay);
                humBite->setImmEndDay(currentDay + 14 + rGenInf->getHumanImmunity());
                humBite->updateRecent(1, disease > 0, hospitalized > 0);
		//		printf("Successful infection to human: %s - %d\n", humBite->getHouseID().c_str(),humBite->getHouseMemNum());
                // write data about infection to output file
                // *out << currentDay << "," << infection->getInfectionType() << "," << disease << ",";
                // *out << humBite->getAge(currentDay) << "," << humBite->getPreviousInfections() << ",";
                // *out << humBite->isVaccinated() << "\n";
                // *out << "," << humBite->getHouseID() << "," << humBite->getHouseMemNum();                        
                // *out << "," << humBite->getAge(currentDay) << "," << humBite->getGender();
                // *out << "," << sday << "," << locNow->getLocID() << "\n";
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
