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
        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());
        if(humBite->infection != nullptr){
            humBite->infection->setInfectiousnessHuman(currentDay);
            if(rGen->getEventProbability() < humBite->infection->getInfectiousness()){
                double sday = currentDay + rGen->getMozLatencyDays();
                int eday = numDays + 1;
                infection.reset(new Infection(
                    round(sday), eday, 0.0, humBite->infection->getInfectionType(), 0, 0));
            }
        }        
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

    // if someone is found to bite
    if(humBite != NULL){
        // update the mosquito's status
        setState(Mosquito::MozState::REST);
        setBiteStartDay(currentDay + rGen->getMozRestDays());

        // if the mosquito is infectious, the human susceptible, and the infection successful
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
            if(rGen->getEventProbability() < infection->getInfectiousness()){
                // clinical disease state
                int disease = 0;
                int hospitalized = 0;
                int vaxAdvancement = 0;
                if(humBite->isVaccinated()){
                    vaxAdvancement = 1;
                }
                if(humBite->getPreviousInfections() + vaxAdvancement == 0){
                    if(rGen->getEventProbability() < 0.3){
                        disease = infection->getInfectionType();
                        if(rGen->getEventProbability() < 0.111){
                            hospitalized = infection->getInfectionType();
                        }
                    }
                }else if(humBite->getPreviousInfections() + vaxAdvancement == 1){
                    if(rGen->getEventProbability() < 0.6){
                        disease = infection->getInfectionType();
                        if(rGen->getEventProbability() < 0.20868){
                            hospitalized = infection->getInfectionType();
                        }
                    }
                }else{
                    if(rGen->getEventProbability() < 0.1){
                        disease = infection->getInfectionType();
                        if(rGen->getEventProbability() < 0.05217){
                            hospitalized = infection->getInfectionType();
                        }
                    }
                }

                // effect of vaccine on preventing infection and disease
                if(humBite->isVaccinated()){
                    if(rGen->getEventProbability() < humBite->getVE(infection->getInfectionType())){
                        if(humBite->getVaccinationDay() + rGen->getWaningTime(infection->getInfectionType()) > currentDay){
                            return;
                        } else {
                            humBite->waneVaccination();
                        }
                    } else {
                        if(disease > 0 && rGen->getEventProbability() < humBite->getVE(infection->getInfectionType()))
                            disease = 0;   
                    }
                }

                // check to see whether the human is immune
                if(humBite->isImmune(infection->getInfectionType()))
                    return;

                // record infection and update immune status
                int sday = currentDay + 1;
                int eday = sday + 14;
                humBite->infection.reset(new Infection(
                    sday, eday, 0.0, infection->getInfectionType(), humBite->getPreviousInfections() == 0, disease > 0));
                humBite->updateImmunityPerm(infection->getInfectionType(),true);
                humBite->setImmunityTemp(true);
                humBite->setImmStartDay(currentDay);
                humBite->setImmEndDay(currentDay + 14 + rGen->getHumanImmunity());
                humBite->updateRecent(1, disease > 0, hospitalized > 0);

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
