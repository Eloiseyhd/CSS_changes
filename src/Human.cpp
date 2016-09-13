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
    std::vector<double> FOI)
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
    vday = -1;
    trialDay = 0;
    vaxWaning_pos = 0;
    vaxWaning_neg = 0;
    vaccineComplete = false;
    enrolledInTrial = false;
    seroStatusAtVaccination = false;
    for(int i = 0;i < 4; i++){
	preExposureAtVaccination[i] = false;
	exposedCount[i] = 0;
    }
    infected = false;
    symptomatic = false;
    hospitalized = false;
    vaccineImmunity = false;
    reportSymptoms = false;
    vaccineDosesReceived = 0;
    lastDayContactedByTrial = 0;
    selfReportProb = 0.0;
    if(bday < (currDay - 180)){
        immunity_temp = false;
	// Set the initial conditions for the immune profile by serotype
        setImmunityPerm(1, rGen.getHumanSeropositivity(FOI[0], double(currDay - bday)));
        setImmunityPerm(2, rGen.getHumanSeropositivity(FOI[1], double(currDay - bday)));
        setImmunityPerm(3, rGen.getHumanSeropositivity(FOI[2], double(currDay - bday)));
        setImmunityPerm(4, rGen.getHumanSeropositivity(FOI[3], double(currDay - bday)));
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
       reportSymptoms = false;
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
    double RRHosp = 1.0;
    
    int vaxAdvancement = 0;

    if(vaccinated){
	if(vaccineProfile == NULL){
	    printf("Human::infect vaccineProfile is NULL and person is vaccinated\n");
	    exit(1);
	}
	
	// There are multiple vaccines supported: GSK, advance (Sanofi), or age
	// We have to specify the effects for each of these vaccine modes

	if(vaccineProfile->getMode() == "advance"){
    	    vaxAdvancement = 1;
    	}else if(vaccineProfile->getMode() == "age"){
	    RR =  vaccineProfile->getRR(getPreviousInfections(), double(getAgeDays(currentDay)));
    	    RRInf = pow(RR, vaccineProfile->getPropInf());
    	    RRDis = pow(RR, 1.0 - vaccineProfile->getPropInf());
    	}else if(vaccineProfile->getMode() == "GSK"){

	    // After the waning period there's no effect of the vaccine in the reduction of the relative risk of infection
	    // The waning time is approximately tau * 4, being waning = exp(-t/tau), the RR should go up from RR(0) to 1
	    double vaxWaning = getPreviousInfections() > 0 ? vaxWaning_pos : vaxWaning_neg;
	    double wan_ = exp(-double(currentDay - vday) / (vaxWaning));
	    RRInf = 1 - (1 - vaccineProfile->getRRInf(getPreviousInfections() > 0)) * wan_ ;
	    RRDis = 1 - (1 - vaccineProfile->getRRDis(getPreviousInfections() > 0)) * wan_;
	    RRHosp = 1 - (1 - vaccineProfile->getRRHosp(getPreviousInfections() > 0)) * wan_;
	    /*
	    if(vday == 15){
		printf("ID %s-%d day %d vday %d RRInf %.4f RRDis %.4f RRHosp %.4f\n", houseID.c_str(), houseMemNum,currentDay, vday, RRInf, RRDis, RRHosp);
		}*/
	    //   printf("ID %s-%d day %d vday %d RRInf %.4f RRDis %.4f RRHosp %.4f\n", houseID.c_str(), houseMemNum,currentDay, vday, RRInf, RRDis, RRHosp);
	}
    }
    
    double vax_protection = 1.0;
    if(isImmuneVax() == true && vaccineProfile->getMode() == "advance"){
    	vax_protection = 1.0 - vaccineProfile->getVaccineProtection();
    }
    if(rGen->getEventProbability() < RRInf * vax_protection){
    	infected = true;
    	recent_inf = 1;
    	recent_dis = 0;
    	recent_hosp = 0;
	last_serotype = infectionType;

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
        		if(rGen->getEventProbability() < (*hospRates)[1] * RRHosp){
        		    recent_hosp = infectionType;
        		}
    	    }
    	} else {
    	    if(rGen->getEventProbability() < (*disRates)[2] * RRDis){
        		recent_dis = infectionType;
        		symptomatic = true;
			hospitalized = true;
        		if(rGen->getEventProbability() < (*hospRates)[2] * RRHosp){
        		    recent_hosp = infectionType;
        		}
    	    }
    	}
    	infection.reset(new Infection(
    	      currentDay + 1, currentDay + 15, 0.0, infectionType, getPreviousInfections() == 0, recent_dis > 0, exp(rGen->getRandomNormal() * 0.2701716 + 1.750673)));

	if(symptomatic == true && enrolledInTrial == true){
	    if(rGen->getEventProbability() < selfReportProb){
		reportSymptoms = true;
	    }
	}
	exposedCount[infectionType - 1]++;
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
    immunity_temp = false;
    vaccinated = false;
    vaccineProfile = NULL;
    enrolledInTrial = false;
    trialArm.clear();
    vaccineComplete = false;
    reportSymptoms = false;
    lastDayContactedByTrial = 0;
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
    trialDay = 0;
    vaxWaning_pos = 0;
    vaxWaning_neg = 0;
    vday = -1;
    for(int i = 0;i < 4; i++){
	preExposureAtVaccination[i] = false;
	exposedCount[i] = 0;
    }
}



void Human::resetRecent(){
    recent_inf = 0;
    recent_dis = 0; 
    recent_hosp = 0;
    last_serotype = -1;
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
      if(immunity_perm[i+1]){
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



void Human::vaccinate(int currDay)
{
    vaccinated = true;
    vday = currDay;
}

void Human::vaccinateAdvanceMode(int currDay, RandomNumGenerator& rGen)
{
    vaccinated = true;
    vday = currDay;
    setVaxImmunity(true);
    setVaxImmStartDay(currDay);
    setVaxImmEndDay(currDay + 365.0 + rGen.getVaxHumanImmunity(rGen.getWaningTime(vaccineProfile->getWaning())));
}

void Human::vaccinateGSKMode(int currDay, RandomNumGenerator& rGen)
{
    vaccinated = true;
    vday = currDay;
    vaxWaning_neg = rGen.getVaxHumanImmunity(vaccineProfile->getWaning(false));
    vaxWaning_pos = rGen.getVaxHumanImmunity(vaccineProfile->getWaning(true));
}

void Human::vaccinateWithProfile(int currDay, RandomNumGenerator * rGen, Vaccine * vax){
    vaccineProfile = vax;
    if(vaccineProfile != NULL){
	for(int i = 0;i < 4; i++){
	    exposedCount[i] = 0;
	}
	vaccineDosesReceived = 1;
	vday = currDay;
	if(vaccineProfile->getMode() == "advance"){
	    this->vaccinateAdvanceMode(currDay, (*rGen));
	}else if(vaccineProfile->getMode() == "age"){
	    this->vaccinate(currDay);
	}else if(vaccineProfile->getMode() == "GSK"){
	    this->vaccinateGSKMode(currDay, (*rGen));
	}
	if(vaccineProfile->getDoses() == vaccineDosesReceived){
	    vaccineComplete = true;
	}
    }else{
	printf("VaccineProfile is NULL in Human::vaccinateWithProfile\n");
	exit(1);
    }
}

void Human::boostVaccine(int currDay, RandomNumGenerator * rGen){
    if(vaccineProfile != NULL){
	vaccineDosesReceived++;
	if(vaccineProfile->getMode() == "advance"){
	    this->vaccinateAdvanceMode(currDay, (*rGen));
	}else if(vaccineProfile->getMode() == "age"){
	    this->vaccinate(currDay);
	}else if(vaccineProfile->getMode() == "GSK"){
	    this->vaccinateGSKMode(currDay, (*rGen));
	}
	if(vaccineProfile->getDoses() == vaccineDosesReceived){
	    vaccineComplete = true;
	}
    }else{
	printf("VaccineProfile is NULL in Human::boostVaccine\n");
	exit(1);
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
	if(vaccineProfile != NULL){
	    return vaccineProfile->getNextDoseDay(vday,vaccineDosesReceived);
	}else{
	    printf("VaccineProfile is NULL in Human::getNextDoseDay\n");
	    exit(1);
	}
    }
}



void Human::updateVaccineEfficacy(int currDay){
    // Sanofi-like vaccine includes a temporary complete immunity that wanes with time
    if(vaccineProfile->getMode() == "advance" && currDay == this->getVaxImmEndDay()){
	this->setVaxImmunity(false);
    }
    /*else if (vaccineProfile->getMode() == "GSK" && houseID == "BG111"){
	double vaxWaning = getPreviousInfections() > 0 ? vaxWaning_pos : vaxWaning_neg;
	double wan_ = exp(-double(currDay - vday) / (vaxWaning));
	double RRInf = 1 - (1 - vaccineProfile->getRRInf(getPreviousInfections() > 0)) * wan_ ;
	double RRDis = 1 - (1 - vaccineProfile->getRRDis(getPreviousInfections() > 0)) * wan_;
	double RRHosp = 1 - (1 - vaccineProfile->getRRHosp(getPreviousInfections() > 0)) * wan_;
	printf("ID %s-%d day %d vday %d Previous %d RRInf %.4f RRDis %.4f RRHosp %.4f wanPos %d wanNeg %d\n", houseID.c_str(), houseMemNum,currDay, vday, getPreviousInfections(), RRInf, RRDis, RRHosp, vaxWaning_pos, vaxWaning_neg);
	}*/
}

Human::Human() {
}



Human::Human(const Human& orig) {
}



Human::~Human() {
}

