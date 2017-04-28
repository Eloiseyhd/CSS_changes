#include "Mosquito.h"
#include <sstream>

using std::runtime_error;

string Mosquito::getLocationID() const {
    return locationID;
}

void Mosquito::setLocation(string loc) {
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
    ratemap_t disRates,
    ratemap_t hospRates,
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



sp_human_t Mosquito::whoBite(
    double time,
    Location * locNow,
    RandomNumGenerator * rGen)
{
    map<sp_human_t,double,Human::sortid> humanMap;
    // loop through set of 
    // (shared pointers to) humans at this place
    for(auto sphum :locNow->getHumans() ){
        const string & currentLoc = sphum->getCurrentLoc(time);
        if(currentLoc == locationID){
            humanMap.insert(make_pair(sphum,sphum->getAttractiveness()));
        }
    }

    if(humanMap.empty())
        return nullptr;

    double attractivenessSum = 0;
    // for each map element
    for(auto & hum : humanMap) {
        attractivenessSum += hum.second;
	//printf("attractiveness of %s is %f. Sum: %.2f\n", hum.first->getPersonID().c_str(), hum.second, attractivenessSum);
    }

    double biteWho = rGen->getEventProbability() * attractivenessSum;
    //printf("BiteWho: %.2f\n",biteWho);
    // find human based on cum attractiveness? 
    attractivenessSum = 0;
    // need mapItr at this scope
    auto mapItr = humanMap.begin();
    for(; attractivenessSum < biteWho; mapItr++) {
        attractivenessSum += mapItr->second;
	//	printf("CHOOSE ATTRACTIVENESS OF %s -> %f, sum: %.2f\n", mapItr->first->getPersonID().c_str(), mapItr->second,attractivenessSum);
    }
    // back up one?
    mapItr--;
    return mapItr->first; 
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
    sp_human_t humBite = whoBite(time, locNow, rGen);    
    if(humBite != nullptr){
	printf("BITE,%s_%d,%.2f,1\n",humBite->getHouseID().c_str(),humBite->getHouseMemNum(),humBite->getAttractiveness());
	if(humBite->infection != nullptr){
            humBite->infection->setInfectiousnessHuman(currentDay);
            if(rGenInf->getEventProbability() < humBite->infection->getInfectiousness()){
		//                double sday = double(currentDay) + rGenInf->getMozLatencyDays(mozEIP);
                int eday = numDays + 1;
		/*                infection.reset(new Infection(
				  round(sday), eday, 0.0, humBite->infection->getInfectionType(), 0, 0, 0.0));*/
		humanInfector = humBite;
                infection.reset(new Infection(
                    -1, eday, 0.0, humBite->infection->getInfectionType(), 0, 0, 0.0
                ));
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
    ratemap_t disRates,
    ratemap_t hospRates,
    int currentDay,
    int numDays,
    std::ofstream * out)
{
    sp_human_t humBite = whoBite(time, locNow, rGen);
    if(humBite != nullptr){	
	printf("BITE,%s_%d,%.2f,1\n",humBite->getHouseID().c_str(),humBite->getHouseMemNum(),humBite->getAttractiveness());
        if(infection != nullptr && humBite->infection == nullptr && !humBite->isImmune(infection->getInfectionType())){
            if(rGenInf->getEventProbability() < infection->getInfectiousness() && humanInfector != nullptr){		
                humBite->infect(currentDay, infection->getInfectionType(), rGenInf, disRates, hospRates, humanInfector);
            }else if(humanInfector == nullptr){
		throw runtime_error("In Mosquito.cpp:: Human infector is null");
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
    humanInfector = nullptr;
}

Mosquito::Mosquito() {
}

Mosquito::Mosquito(const Mosquito& orig) {
}

//Mosquito::~Mosquito() {
//}
