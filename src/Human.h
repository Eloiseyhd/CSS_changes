/* 
 * File:   Human.h
 * Author: amit
 *
 * Created on August 25, 2014, 12:53 AM
 */

#ifndef HUMAN_H
#define	HUMAN_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <memory>
#include <cmath>
#include "Infection.h"
#include "RandomNumGenerator.h"

class Human {
private:
    int bday;
    int dday;
    int trajDay;
    std::string houseID;
    int houseMemNum;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    double bodySize;
    double attractiveness;
    char gender;
    std::map<unsigned,bool> immunity_perm;
    bool immunity_temp;
    unsigned immStartDay;
    unsigned immEndDay;

public:
    std::unique_ptr<Infection> infection;
    void reincarnate(unsigned);
    bool isImmune(unsigned) const;
    bool isImmuneTemp(){return immunity_temp;}
    void setImmunityPerm(unsigned,bool);
    void setImmunityTemp(bool);
    void setImmStartDay(unsigned);
    void setImmEndDay(unsigned);
    unsigned getImmStartDay() const;
    unsigned getImmEndDay() const;
    int getTrajDay(){return trajDay;}
    void setTrajDay(int dayIn){trajDay = dayIn;}
    std::set<std::string> getLocsVisited();
    std::string getHouseID() const;
    int getHouseMemNum() const;
    int getAge(unsigned) const;
    double getBodySize() const;
    double getAttractiveness() const;
    char getGender() const;
    std::string toString() const;
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;
    std::string getCurrentLoc(double);
    Human(std::string, int, int, double, char, std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned,double);
    Human();
    Human(const Human& orig);
    virtual ~Human();
private:

};

#endif	/* HUMAN_H */

