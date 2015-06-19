/* 
 * File:   Human.cpp
 * Author: amit
 * 
 * Created on August 25, 2014, 12:53 AM
 */

#include "Human.h"
#include <sstream>

using namespace std;

Human::Human(string hID, unsigned hMemID, int age, double bSize, char gen, unique_ptr<vector<vector<pair<string, double >> >> &paths) {
    houseID = hID;
    houseMemNum = hMemID;
    bday = -365 * age;
    bodySize = bSize;
    gender = gen;
    trajectories = move(paths);
    infection.reset(nullptr);
    immunity = false;
    attractiveness = pow(bSize, 1.541);
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

unsigned Human::getHouseMemNum() const {
    return houseMemNum;
}

int Human::getAge(unsigned currDay) const {
    return currDay - bday;
}

double Human::getBodySize() const {
    return bodySize;
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

bool Human::isImmune() const {
    return immunity;
}

void Human::setImmunity(bool im) {
    immunity = im;
}

std::vector<std::pair<std::string, double >> const& Human::getTrajectory(unsigned i) const {
    return (*trajectories.get())[i];
}

std::pair<std::string,double>* Human::getCurrentLoc(double time){
    std::vector<std::pair<std::string,double>>::iterator itrLoc = (*trajectories)[trajDay].begin();

    for(double runSum = 0; runSum < time; itrLoc++)
        runSum += itrLoc->second;

    return &(*(--itrLoc));
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

