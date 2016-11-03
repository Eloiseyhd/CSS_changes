#include "Mosquito.h"
#include <sstream>

using namespace std;

string Mosquito::getLocationID() const {
    return locationID;
}

void Mosquito::setLocation(std::string loc) {
    locationID = loc;
}

double Mosquito::getDDay() const {
    return dday;
}

void Mosquito::setBiteStartDay(double d) {
    biteStartDay = d;
}

double Mosquito::getBiteStartDay() {
    return biteStartDay;
}

bool Mosquito::takeBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    RandomNumGenerator * rGenInf,
    std::map<unsigned,double> * disRates,
    std::map<unsigned,double> * hospRates,
    int currentDay,
    int numDays,
    std::ofstream * out,
    double mozEIP)
{
    if(infection == nullptr){
        if(locNow->getInfectedVisitor()){
            return infectingBite(time, locNow, rGen, rGenInf, currentDay, numDays, mozEIP);            
        } else {
	    nbites++;
            return true;
        }
    }
    else if(infection->getInfectiousness() >= 0.0){
        return infectiousBite(time, locNow, rGen, rGenInf, disRates, hospRates, currentDay, numDays, out);
    }
    return false;
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
        }
    }

    if(humanMap.empty())
        return NULL;

    double attractivenessSum = 0;
    std::map<Human *,double,Human::sortid>::iterator mapItr = humanMap.begin();
    for(; mapItr != humanMap.end(); mapItr++){
        attractivenessSum += mapItr->second;
	//	printf("attractiveness of %s is %f\n", mapItr->first->getPersonID().c_str(), mapItr->second);
    }

    double biteWho = rGen->getEventProbability() * attractivenessSum;
    attractivenessSum = 0;
    for(mapItr = humanMap.begin(); attractivenessSum < biteWho; mapItr++)
        attractivenessSum += mapItr->second;
    mapItr--;
    //    return &(*humanMap.begin()->first);
    return &(*mapItr->first); 
}



bool Mosquito::infectingBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    RandomNumGenerator * rGenInf,
    int currentDay,
    int numDays, 
    double mozEIP)
{
    Human * humBite = whoBite(time, locNow, rGen);
    if(humBite != NULL){
	if(humBite->infection != nullptr){
            humBite->infection->setInfectiousnessHuman(currentDay);
            if(rGenInf->getEventProbability() < humBite->infection->getInfectiousness()){
		//                double sday = double(currentDay) + rGenInf->getMozLatencyDays(mozEIP);
                int eday = numDays + 1;
		/*                infection.reset(new Infection(
				  round(sday), eday, 0.0, humBite->infection->getInfectionType(), 0, 0, 0.0));*/
                infection.reset(new Infection(
                    -1, eday, 0.0, humBite->infection->getInfectionType(), 0, 0, 0.0));
            }
        }
	nbites++;
        return true;
    }else{
        return false;
    }
}



bool Mosquito::infectiousBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen,
    RandomNumGenerator * rGenInf,
    std::map<unsigned,double> * disRates,
    std::map<unsigned,double> * hospRates,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    Human * humBite = whoBite(time, locNow, rGen);
    if(humBite != NULL){
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
            if(rGenInf->getEventProbability() < infection->getInfectiousness()){
                humBite->infect(currentDay, infection->getInfectionType(), rGenInf, disRates, hospRates);
            }
        }
	nbites++;
        return true;
    }else{
        return false;
    }
}



Mosquito::Mosquito(double dd, double bsd, string loc) {
    locationID = loc;
    dday = dd;
    biteStartDay = bsd;
    infection.reset(nullptr);
    nbites = 0;
    bday = 0;
}



Mosquito::Mosquito() {
}



Mosquito::Mosquito(const Mosquito& orig) {
}



Mosquito::~Mosquito() {
}
