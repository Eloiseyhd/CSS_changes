#include "Human.h"
#include <sstream>

using namespace std;



Human::Human(
    string hID,
    int hMemID,
    int age,
    char gen,
    unique_ptr<vector<vector<pair<string, double >> >> &paths,
    RandomNumGenerator& rGen,
    unsigned currDay,
    double FOI)
{
    houseID = hID;
    houseMemNum = hMemID;
    bday = currDay - 365 * age - rGen.getRandomNum(365);
    gender = gen;
    initiateBodySize(currDay,rGen);
    trajectories = move(paths);
    trajDay = 0;
    infection.reset(nullptr);
    updateAttractiveness(currDay);
    vaccinated = false;
    vaccineProfile = NULL;
    resetRecent();
    cohort = 0;
    tAge = 0;
    trialDay = 0;
    vaccineComplete = false;
    enrolledInTrial = false;
    seroStatusAtVaccination = false;
    for(int i = 0;i < 4; i++){
	preExposureAtVaccination[i] = false;
    }
    infected = false;
    symptomatic = false;
    hospitalized = false;
    vaccineImmunity = false;
    vaccineProtection = 0;
    vaccineDosesReceived = 0;
    lastDayContactedByTrial = 0;
    selfReportDay = -1;
    selfReportProb = 0.0;
    if(bday < currDay - 180){
        immunity_temp = false;
        setImmunityPerm(1, rGen.getHumanSeropositivity(FOI, double(currDay - bday)));
        setImmunityPerm(2, rGen.getHumanSeropositivity(FOI, double(currDay - bday)));
        setImmunityPerm(3, rGen.getHumanSeropositivity(FOI, double(currDay - bday)));
        setImmunityPerm(4, rGen.getHumanSeropositivity(FOI, double(currDay - bday)));
    } else {
        immunity_temp = true;
        immStartDay = bday;
        immEndDay = bday + 180;
        setImmunityPerm(1,false);
        setImmunityPerm(2,false);
        setImmunityPerm(3,false);
        setImmunityPerm(4,false);
    }
}



void Human::checkRecovered(unsigned currDay){
    if(infection->getEndDay() <= currDay){
       infection.reset(nullptr);
       infected = false;
       hospitalized = false;
       symptomatic = false;
       selfReportDay = -1;
    }
}



double Human::getAttractiveness() const {
    return attractiveness;
}



int Human::getAgeDays(unsigned currDay) const {
    return currDay - bday;
}



double Human::getBodySize() const {
    return bodySize;
}



std::string Human::getCurrentLoc(double time){
    std::vector<std::pair<std::string,double>>::iterator itrLoc = (*trajectories)[trajDay].begin();

    for(double runSum = 0; runSum < time && itrLoc != (*trajectories)[trajDay].end(); itrLoc++){
        runSum += itrLoc->second;
    }
    if((*trajectories)[trajDay].size() == 1){
        itrLoc = (*trajectories)[trajDay].begin();
    } else {
        itrLoc--;
    }

    return itrLoc->first;
}



char Human::getGender() const {
    return gender;
}



unsigned Human::getImmStartDay() const {
    return immStartDay;
}

unsigned Human::getVaxImmStartDay() const {
    return vaxImmStartDay;
}

unsigned Human::getImmEndDay() const {
    return immEndDay;
}

unsigned Human::getVaxImmEndDay() const {
    return vaxImmEndDay;
}

string Human::getHouseID() const {
    return houseID;
}



int Human::getHouseMemNum() const {
    return houseMemNum;
}



std::set<std::string> Human::getLocsVisited(){
    std::set<std::string> locsVisited;

    int numTrajs = trajectories->size();
    std::vector<std::pair<std::string,double>>::iterator itr;

    for(int i = 0; i < numTrajs; i++){
        for(itr = trajectories->at(i).begin(); itr != trajectories->at(i).end(); itr++){
            locsVisited.insert(itr->first);
        }
    }

    return locsVisited;
}



int Human::getPreviousInfections(){
    int previnf = 0;

    if(immunity_perm[1])
        previnf++;
    if(immunity_perm[2])
        previnf++;
    if(immunity_perm[3])
        previnf++;
    if(immunity_perm[4])
        previnf++;

    return previnf;
}



std::vector<std::pair<std::string, double >> const& Human::getTrajectory(unsigned i) const {
    return (*trajectories.get())[i];
}



void Human::infect(
    int currentDay,
    unsigned infectionType,
    RandomNumGenerator * rGen,
    std::map<unsigned,double> * disRates,
    std::map<unsigned,double> * hospRates)
{
    double RR = 1.0;
    double RRInf = 1.0;
    double RRDis = 1.0;
    double totalVE = 0.0;
    
    int vaxAdvancement = 0;

    if(vaccinated){
	if(vaccineProfile->mode == "advance"){
    	    vaxAdvancement = 1;
    	}else if(vaccineProfile->mode == "age"){
    	    if(getPreviousInfections() > 0){
		totalVE = 1.0 - vepos->at(0) / (1.0 + exp(vepos->at(1) * (double(getAgeDays(currentDay)) / 365.0 - vepos->at(2))));
    	    }else{
                totalVE = 1.0 - veneg->at(0) / (1.0 + exp(veneg->at(1) * (double(getAgeDays(currentDay)) / 365.0 - veneg->at(2))));
    	    }
    	    RR = exp(log(1.0 - totalVE) + normdev * pow(1.0 / 100.5 + 1.0 / (100.0 * (1.0 - totalVE) + 0.5), 0.5));
    	    RRInf = pow(RR, propInf);
    	    RRDis = pow(RR, 1.0 - propInf);
    	}
    }
    
    double vax_protection = 1.0;
    if(isImmuneVax() == true && vaccineProfile->mode == "advance"){
    	vax_protection = 1.0 - vaccineProtection;
    }
    if(rGen->getEventProbability() < RRInf * vax_protection){
    	infected = true;
    	recent_inf = 1;
    	recent_dis = 0;
    	recent_hosp = 0;
    	if(getPreviousInfections() + vaxAdvancement == 0){
    	    if(rGen->getEventProbability() < (*disRates)[0] * RRDis){
        		recent_dis = infectionType;
        		symptomatic = true;
        		if(rGen->getEventProbability() < (*hospRates)[0]){
        		    recent_hosp = infectionType;
        		}
    	    }
    	} else if(getPreviousInfections() + vaxAdvancement == 1) {
    	    if(rGen->getEventProbability() < (*disRates)[1] * RRDis){
        		recent_dis = infectionType;
        		symptomatic = true;
        		if(rGen->getEventProbability() < (*hospRates)[1]){
        		    recent_hosp = infectionType;
        		}
    	    }
    	} else {
    	    if(rGen->getEventProbability() < (*disRates)[2] * RRDis){
        		recent_dis = infectionType;
        		symptomatic = true;
        		if(rGen->getEventProbability() < (*hospRates)[2]){
        		    recent_hosp = infectionType;
        		}
    	    }
    	}
    	infection.reset(new Infection(
    	      currentDay + 1, currentDay + 15, 0.0, infectionType, getPreviousInfections() == 0, recent_dis > 0, exp(rGen->getRandomNormal() * 0.2701716 + 1.750673)));

	if(symptomatic == true && enrolledInTrial == true){
	    if(rGen->getEventProbability() < selfReportProb){
		selfReportDay = rGen->getSelfReportDay(infection->getSymptomOnset() + rGen->getRandomNum(5));
	    }
	}

    	updateImmunityPerm(infectionType, true);
    	setImmunityTemp(true);
    	setImmStartDay(currentDay);
    	setImmEndDay(currentDay + 15 + rGen->getHumanImmunity());
    }
}



void Human::initiateBodySize(unsigned currDay, RandomNumGenerator& rGen){
    bodySizeBirth = -1.0;
    bodySizeAdult = -1.0;
    while(bodySizeBirth <= 0.0 || bodySizeAdult <= 0.0){
        if(gender == 'F'){
            bodySizeBirth = 0.3085318 + 0.09302602 * rGen.getRandomNormal();
            bodySizeAdult = 1.505055 + 0.12436 * rGen.getRandomNormal();
            bodySizeSlope = (bodySizeAdult - bodySizeBirth) / 6030.0;
        } else {
            bodySizeBirth = 0.3114736 + 0.1532624 * rGen.getRandomNormal();
            bodySizeAdult = 1.712391 + 0.1523652 * rGen.getRandomNormal();
            bodySizeSlope = (bodySizeAdult - bodySizeBirth) / 6809.0;
        }
    }
    updateBodySize(currDay);
}



bool Human::isImmune(unsigned serotype) const {
    bool immunity = false;

    if(immunity_temp){
	immunity = true;
    } else if(immunity_perm.at(serotype)) {
        immunity = true;
    }
    return immunity;
}



void Human::reincarnate(unsigned currDay){
    bday = currDay;
    updateAttractiveness(currDay);
    infection.reset(nullptr);
    infected = false;
    hospitalized = false;
    symptomatic = false;
    seroStatusAtVaccination = false;
    immunity_temp = true;
    vaccinated = false;
    vaccineProfile = NULL;
    enrolledInTrial = false;
    trialArm.clear();
    vaccineComplete = false;
    lastDayContactedByTrial = 0;
    selfReportDay = -1;
    immStartDay = bday;
    immEndDay = bday + 180;
    updateImmunityPerm(1,false);
    updateImmunityPerm(2,false);
    updateImmunityPerm(3,false);
    updateImmunityPerm(4,false);
    recent_inf = 0;
    recent_dis = 0;
    recent_hosp = 0;
    vaccineImmunity = false;
    vaccineProtection = 0;
}



void Human::resetRecent(){
    recent_inf = 0;
    recent_dis = 0; 
    recent_hosp = 0;
}



void Human::setImmEndDay(unsigned d) {
    immEndDay = d;
}

void Human::setVaxImmEndDay(unsigned d){
    vaxImmEndDay = d;
}


void Human::setImmStartDay(unsigned d) {
    immStartDay = d;
}

void Human::setVaxImmStartDay(unsigned d){
    vaxImmStartDay = d;
}

void Human::setImmunityPerm(unsigned serotype, bool status) {
    immunity_perm.erase(serotype);
    immunity_perm.insert(make_pair(serotype,status));
}



void Human::setImmunityTemp(bool status) {
    immunity_temp = status;
}

void Human::setVaxImmunity(bool status) {
    vaccineImmunity = status;
}

void Human::setSeroStatusAtVaccination(){
  if(getPreviousInfections() > 0){
    seroStatusAtVaccination = true;
  }
  for(int i = 0; i < 4;i++){
      if(immunity_perm[i]){
	  preExposureAtVaccination[i] = true;
      }
  }
}

bool Human::getPreExposureAtVaccination(unsigned sero){
    return preExposureAtVaccination[sero];
}

void Human::updateAttractiveness(unsigned currDay){
    updateBodySize(currDay);
    attractiveness = pow(bodySize, 1.541);
}



void Human::updateBodySize(unsigned currDay){
    if(gender == 'F'){
        if(currDay - bday >= 6030){
            bodySize = bodySizeAdult;
        } else{
            bodySize = bodySizeBirth + bodySizeSlope * (currDay - bday);
        }
    } else {
        if(currDay - bday >= 6809){
            bodySize = bodySizeAdult;
        } else{
            bodySize = bodySizeBirth + bodySizeSlope * (currDay - bday);
        }
    }
}



void Human::updateImmunityPerm(unsigned serotype, bool status) {
    immunity_perm.erase(serotype);
    immunity_perm.insert(make_pair(serotype,status));
}



void Human::updateRecent(int infIn, int disIn, int hospIn){
    if(infIn > 0){
        recent_inf = infIn;
    }
    if(disIn > 0){
        recent_dis = disIn;
    }
    if(hospIn > 0){
        recent_hosp = hospIn;
    }
}



void Human::vaccinate(
    std::map<unsigned,double> * veposIn,
    std::map<unsigned,double> * venegIn,
    double propInfIn,
    int currDay,
    double normdevIn)
{
    vaccinated = true;
    vepos = veposIn;
    veneg = venegIn;
    propInf = propInfIn;
    vday = currDay;
    normdev = normdevIn;
}

void Human::vaccinateAdvanceMode(int currDay, RandomNumGenerator& rGen, double protec_, double wan)
{
    vaccinated = true;
    vday = currDay;
    setVaxImmunity(true);
    setVaxImmStartDay(currDay);
    setVaxImmEndDay(currDay + 365.0 + rGen.getVaxHumanImmunity(rGen.getWaningTime(wan)));
    vaccineProtection = protec_;
}

void Human::vaccinateWithProfile(int currDay, RandomNumGenerator * rGen, vProfile * vax){
    vaccineProfile = vax;
    vaccineDosesReceived = 1;
    vday = currDay;
    if(vaccineProfile->mode == "advance"){
	this->vaccinateAdvanceMode(currDay, (*rGen), vaccineProfile->protection, vaccineProfile->waning);
    }else if(vaccineProfile->mode == "age"){
	this->vaccinate(&vaccineProfile->VE_pos, &vaccineProfile->VE_neg, vaccineProfile->propInf, currDay, vaccineProfile->normdev);
    }
    if(vax->doses == vaccineDosesReceived){
	vaccineComplete = true;
    }
}

void Human::enrollInTrial(int currDay, std::string arm_){
    tAge = getAgeDays(currDay);
    trialDay = currDay;
    enrolledInTrial = true;
    trialArm = arm_;
}

int Human::getNextDoseDay(){
    if(vaccineComplete == true){
	return -1;
    }else{
	if(vaccineDosesReceived < vaccineProfile->doses){
	    return (vaccineProfile->relative_schedule[vaccineDosesReceived] + vday);
	}else{
	    return (vaccineProfile->relative_schedule.back() + vday);
	}
    }
}
void Human::boostVaccine(int currDay, RandomNumGenerator * rGen){
    vaccineDosesReceived++;
    if(vaccineProfile->mode == "advance"){
	this->vaccinateAdvanceMode(currDay, (*rGen), vaccineProfile->protection, vaccineProfile->waning);
    }else if(vaccineProfile->mode == "age"){
	this->vaccinate(&vaccineProfile->VE_pos, &vaccineProfile->VE_neg, vaccineProfile->propInf, currDay, vaccineProfile->normdev);
    }
    if(vaccineProfile->doses == vaccineDosesReceived){
	vaccineComplete = true;
    }
}

Human::Human() {
}



Human::Human(const Human& orig) {
}



Human::~Human() {
}

