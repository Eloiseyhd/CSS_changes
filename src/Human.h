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
    double attractiveness;
    double bodySize;
    double bodySizeAdult;
    double bodySizeBirth;
    double bodySizeSlope;
    int bday;
    int cohort;
    char gender;
    bool hospitalized;
    std::string houseID;
    int houseMemNum;
    unsigned immEndDay;
    unsigned vaxImmEndDay;
    std::map<unsigned,bool> immunity_perm;
    bool immunity_temp;
    unsigned immStartDay;
    unsigned vaxImmStartDay;
    bool infected;
    double propInf;
    int recent_dis;
    int recent_hosp;
    int recent_inf;
    bool seroStatusAtVaccination;
    bool symptomatic;
    bool vaccineAdvanceMode;
    bool vaccineImmunity;
    double vaccineProtection;
    int trajDay;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    bool vaccinated;
    int vday;
    int tAge;
    std::map<unsigned,double> * veneg;
    std::map<unsigned,double> * vepos;
    
public:
    std::unique_ptr<Infection> infection;

    Human(std::string,int,int,char,std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned,double);
    Human();
    Human(const Human& orig);
    virtual ~Human();

    void checkRecovered(unsigned);
    int getAgeDays(unsigned) const;
    double getAttractiveness() const;
    double getBodySize() const;
    int getCohort(){return cohort;}
    std::string getCurrentLoc(double);
    char getGender() const;
    std::string getHouseID() const;
    int getHouseMemNum() const;
    unsigned getImmEndDay() const;
    unsigned getImmStartDay() const;
    unsigned getVaxImmEndDay() const;
    unsigned getVaxImmStartDay() const;
    std::set<std::string> getLocsVisited();
    int getPreviousInfections();
    int getRecentDis(){return recent_dis;}
    int getRecentHosp(){return recent_hosp;}
    int getRecentInf(){return recent_inf;}
    bool getSeroStatusAtVaccination(){return seroStatusAtVaccination;}
    int getTrajDay(){return trajDay;}
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;
    int getVaccinationDay(){return vday;}
    void setAgeTrialEnrollment(int age_){tAge = age_;}
    int getAgeTrialEnrollment(){return tAge;}
    void infect(int, unsigned, RandomNumGenerator *, std::map<unsigned,double> *, std::map<unsigned,double> *, double);
    void initiateBodySize(unsigned,RandomNumGenerator&);
    bool isHospitalized(){return hospitalized;}
    bool isImmune(unsigned) const;
    bool isImmuneTemp(){return immunity_temp;}
    bool isImmuneVax(){return vaccineImmunity;}
    bool isInfected(){return infected;}
    bool isSymptomatic(){return symptomatic;}
    bool isVaccinated(){return vaccinated;}
    void reincarnate(unsigned);
    void resetRecent();
    void setCohort(int c_){cohort = c_;}
    void setImmStartDay(unsigned);
    void setImmEndDay(unsigned);
    void setVaxImmStartDay(unsigned);
    void setVaxImmEndDay(unsigned);
    void setVaxImmunity(bool);
    void setImmunityTemp(bool);
    void setImmunityPerm(unsigned,bool);
    void setSeroStatusAtVaccination();
    void setTrajDay(int dayIn){trajDay = dayIn;}
    void updateAttractiveness(unsigned);
    void updateBodySize(unsigned);
    void updateImmunityPerm(unsigned,bool);
    void updateRecent(int,int,int);
    void vaccinate(std::map<unsigned,double> *,std::map<unsigned,double> *,double,int);
    void vaccinateAdvanceMode(int currDay, RandomNumGenerator&, double, double);
    void waneVaccination(){vaccinated = false;}

    struct sortid{
        bool operator() (const Human *a, const Human *b)const{
            return b->getHouseID() + std::to_string(b->getHouseMemNum()) > a->getHouseID() + std::to_string(a->getHouseMemNum());
        }
    };
};

#endif	/* HUMAN_H */
