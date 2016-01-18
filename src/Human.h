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
    int trajDay;
    std::string houseID;
    int houseMemNum;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    double bodySize;
    double bodySizeBirth;
    double bodySizeAdult;
    double bodySizeSlope;
    double attractiveness;
    char gender;
    std::map<unsigned,bool> immunity_perm;
    bool immunity_temp;
    unsigned immStartDay;
    unsigned immEndDay;
    bool seroStatusAtVaccination;
    bool vaccinated;
    unsigned doses;
    int vday;
    std::map<unsigned,double> VE;
    int recent_inf;
    int recent_dis;
    int recent_hosp;

public:
    std::unique_ptr<Infection> infection;
    void infect(int, unsigned, RandomNumGenerator *);
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
    void vaccinate(std::map<unsigned,double> *,std::map<unsigned,double> *,RandomNumGenerator&,double,int);
    void waneVaccination(){vaccinated = false;}
    void checkRecovered(unsigned);
    bool isVaccinated(){return vaccinated;}
    int getRecentInf(){return recent_inf;}
    int getRecentDis(){return recent_dis;}
    int getRecentHosp(){return recent_hosp;}
    bool getSeroStatusAtVaccination(){return seroStatusAtVaccination;}
    void resetRecent();
    void updateRecent(int,int,int);
    void updateSeroStatusAtVaccination();
    std::set<std::string> getLocsVisited();
    std::string getHouseID() const;
    int getHouseMemNum() const;
    int getAge(unsigned) const;
    double getBodySize() const;
    void initiateBodySize(unsigned,RandomNumGenerator&);
    void updateBodySize(unsigned);
    double getAttractiveness() const;
    void updateAttractiveness(unsigned);
    char getGender() const;
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;
    std::string getCurrentLoc(double);
    Human(std::string,int,int,char,std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned);
    Human();
    Human(const Human& orig);
    virtual ~Human();
    struct sortid{
      bool operator() (const Human *a, const Human *b)const{
	return b->getHouseID() + std::to_string(b->getHouseMemNum()) > a->getHouseID() + std::to_string(a->getHouseMemNum());
      }
    };

};

#endif	/* HUMAN_H */
