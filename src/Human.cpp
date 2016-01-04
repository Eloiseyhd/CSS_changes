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
    unsigned currDay)
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
    doses = 0;
    resetRecent();

    if(bday < currDay - 180){
        immunity_temp = false;
        setImmunityPerm(1,false);
        setImmunityPerm(2,false);
        setImmunityPerm(3,false);
        setImmunityPerm(4,false);
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



void Human::reincarnate(unsigned currDay){
    bday = currDay;
    updateAttractiveness(currDay);
    infection.reset(nullptr);
    immunity_temp = true;
    immStartDay = bday;
    immEndDay = bday + 180;
    setImmunityPerm(1,false);
    setImmunityPerm(2,false);
    setImmunityPerm(3,false);
    setImmunityPerm(4,false);
    vaccinated = false;
    doses = 0;
}



void Human::vaccinate(
    std::map<unsigned,double> * vepos,
    std::map<unsigned,double> * veneg,
    double partialEfficacy,
    int currDay)
{
    vaccinated = true;
    doses++;
    vday = currDay;

    unsigned infectionCount = 0;
    for(auto it = immunity_perm.begin(); it != immunity_perm.end(); it++){
        if(it->second){infectionCount++;}
    }

    if(infectionCount == 0){
        for(auto it = veneg->begin(); it != veneg->end(); it++){
            VE.insert(make_pair(it->first,1.0 - pow(1.0 - partialEfficacy * it->second, .5)));
        }
    } else {
        for(auto it = vepos->begin(); it != vepos->end(); it++){
            VE.insert(make_pair(it->first,1.0 - pow(1.0 - partialEfficacy * it->second, .5)));
        }
    }
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



string Human::getHouseID() const {
    return houseID;
}



int Human::getHouseMemNum() const {
    return houseMemNum;
}



int Human::getAge(unsigned currDay) const {
    return currDay - bday;
}



double Human::getBodySize() const {
    return bodySize;
}



double Human::getAttractiveness() const {
    return attractiveness;
}



char Human::getGender() const {
    return gender;
}



unsigned Human::getImmStartDay() const {
    return immStartDay;
}



unsigned Human::getImmEndDay() const {
    return immEndDay;
}



void Human::setImmStartDay(unsigned d) {
    immStartDay = d;
}



void Human::setImmEndDay(unsigned d) {
    immEndDay = d;
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



void Human::setImmunityPerm(unsigned serotype, bool status) {
    immunity_perm.insert(make_pair(serotype,status));
}



void Human::updateImmunityPerm(unsigned serotype, bool status) {
    immunity_perm.erase(serotype);
    immunity_perm.insert(make_pair(serotype,status));
}



void Human::setImmunityTemp(bool status) {
    immunity_temp = status;
}



void Human::resetRecent(){
    recent_inf = 0;
    recent_dis = 0;
    recent_hosp = 0;
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



std::vector<std::pair<std::string, double >> const& Human::getTrajectory(unsigned i) const {
    return (*trajectories.get())[i];
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



void Human::updateAttractiveness(unsigned currDay){
    updateBodySize(currDay);
    attractiveness = pow(bodySize, 1.541);
}



void Human::checkRecovered(unsigned currDay){
    if(infection->getEndDay() <= currDay){
       infection.reset(nullptr);
    }
}




Human::Human() {
}



Human::Human(const Human& orig) {
}



Human::~Human() {
}

