/* 
 * File:   Human.cpp
 * Author: amit
 * 
 * Created on August 25, 2014, 12:53 AM
 */

#include "Human.h"
#include <sstream>

using namespace std;

Human::Human(
    string hID,
    int hMemID,
    int age,
    double bSize,
    char gen,
    unique_ptr<vector<vector<pair<string, double >> >> &paths,
    RandomNumGenerator& rGen,
    unsigned currDay,
    double FOI)
{
    houseID = hID;
    houseMemNum = hMemID;
    bday = currDay - 365 * age - rGen.getRandomNum(365);
    bodySize = bSize;
    gender = gen;
    trajectories = move(paths);
    trajDay = 0;
    infection.reset(nullptr);
    attractiveness = pow(bSize, 1.541);
    vaccinated = false;

    if(bday < currDay - 180){
        immunity_temp = false;
        setImmunityPerm(1,rGen.getHumanSeropositivity(FOI, double(age / 365)));
        setImmunityPerm(2,rGen.getHumanSeropositivity(FOI, double(age / 365)));
        setImmunityPerm(3,rGen.getHumanSeropositivity(FOI, double(age / 365)));
        setImmunityPerm(4,rGen.getHumanSeropositivity(FOI, double(age / 365)));
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
    infection.reset(nullptr);
    immunity_temp = true;
    immStartDay = bday;
    immEndDay = bday + 180;
    setImmunityPerm(1,false);
    setImmunityPerm(2,false);
    setImmunityPerm(3,false);
    setImmunityPerm(4,false);
}

void Human::vaccinate(
    std::map<unsigned,double> * vepos,
    std::map<unsigned,double> * veneg,
    double partialEfficacy,
    int currDay)
{
    vaccinated = true;
    vday = currDay;

    unsigned infectionCount = 0;
    for(auto it = immunity_perm.begin(); it != immunity_perm.end(); it++){
        if(it->second){infectionCount++;}
    }

    if(infectionCount == 0){
        for(auto it = veneg->begin(); it != veneg->end(); it++){
            VE.insert(make_pair(it->first,pow(partialEfficacy * it->second, .5)));
        }
    } else {
        for(auto it = vepos->begin(); it != vepos->end(); it++){
            VE.insert(make_pair(it->first,pow(partialEfficacy * it->second, .5)));
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

void Human::setImmunityPerm(unsigned serotype, bool status) {
    immunity_perm.insert(make_pair(serotype,status));
}

void Human::setImmunityTemp(bool status) {
    immunity_temp = status;
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

string Human::toString() const {
    stringstream ss;
    ss << bday << " " << dday << " " << houseID << " " << houseMemNum << " " << bodySize << " " << gender;
    ss << "\n" << infection->toString();
    for (int i = 0; i < trajectories->size(); i++) {
        auto path = (*trajectories.get())[i];
        ss << "\n";
        for (auto pa : path) {
            ss << pa.first << " " << pa.second << " ";
        }
    }
    ss << "\n";
    return ss.str();
}

Human::Human() {
}

Human::Human(const Human& orig) {
}

Human::~Human() {
}

