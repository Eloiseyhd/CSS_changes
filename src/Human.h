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
#include <iostream>
#include <utility>
#include <memory>
#include <cmath>
#include "Infection.h"

class Human {
private:
    int bday;
    int dday;
    int trajDay;
    std::string houseID;
    unsigned houseMemNum;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    double bodySize;
    double attractiveness;
    char gender;
    bool immunity;
    unsigned immStartDay;
    unsigned immEndDay;
    
public:
    std::unique_ptr<Infection> infection;
    bool isImmune() const;
    void setImmunity(bool);
    void setImmStartDay(unsigned);
    void setImmEndDay(unsigned);
    unsigned getImmStartDay() const;
    unsigned getImmEndDay() const;
    int getTrajDay(){return trajDay;}
    void setTrajDay(int dayIn){trajDay = dayIn;}
    std::set<std::string> getLocsVisited();
    std::string getHouseID() const;
    unsigned getHouseMemNum() const;
    int getAge(unsigned) const;
    double getBodySize() const;
    double getAttractiveness(){return attractiveness;}
    char getGender() const;
    std::string toString() const;
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;
    std::pair<std::string,double>* getCurrentLoc(double);
    Human(std::string, unsigned, int, double, char, std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&);
    Human();
    Human(const Human& orig);
    virtual ~Human();
private:

};

#endif	/* HUMAN_H */

