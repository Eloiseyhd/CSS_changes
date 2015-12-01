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
    bool vaccinated;
    unsigned doses;
    int vday;
    std::map<unsigned,double> VE;
    int recent_inf;
    int recent_dis;
    int recent_hosp;

public:
    std::unique_ptr<Infection> infection;
    void reincarnate(unsigned);
    bool isImmune(unsigned) const;
    int getPreviousInfections();
    bool isImmuneTemp(){return immunity_temp;}
    void setImmunityTemp(bool);
    void setImmunityPerm(unsigned,bool);
    void updateImmunityPerm(unsigned,bool);
    void setImmStartDay(unsigned);
    void setImmEndDay(unsigned);
    unsigned getImmStartDay() const;
    unsigned getImmEndDay() const;
    unsigned getDoses();
    int getVaccinationDay(){return vday;}
    double getVE(unsigned sero){return VE.at(sero);}
    int getTrajDay(){return trajDay;}
    void setTrajDay(int dayIn){trajDay = dayIn;}
    void vaccinate(std::map<unsigned,double> *,std::map<unsigned,double> *,double,int);
    void waneVaccination(){vaccinated = false;}
    bool isVaccinated(){return vaccinated;}
    int getRecentInf(){return recent_inf;}
    int getRecentDis(){return recent_dis;}
    int getRecentHosp(){return recent_hosp;}
    void resetRecent();
    void updateRecent(int,int,int);
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

};

#endif	/* HUMAN_H */

