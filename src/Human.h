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
    double propInf;
    double vaccineProtection;

    int bday;
    int cohort;
    int houseMemNum;
    int recent_dis;
    int recent_hosp;
    int recent_inf;
    int last_serotype;
    int trajDay;
    int vday;
    int tAge;

    char gender;

    bool hospitalized;
    bool immunity_temp;
    bool infected;
    bool seroStatusAtVaccination;
    bool symptomatic;
    bool vaccineAdvanceMode;
    bool vaccineImmunity;
    bool vaccinated;

    std::string houseID;

    unsigned immEndDay;
    unsigned vaxImmEndDay;
    unsigned immStartDay;
    unsigned vaxImmStartDay;

    std::map<unsigned,bool> immunity_perm;
    std::map<unsigned,double> * veneg;
    std::map<unsigned,double> * vepos;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    
public:
    std::unique_ptr<Infection> infection;

    Human(std::string,int,int,char,std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned,std::vector<double>);
    Human();
    Human(const Human& orig);
    virtual ~Human();

    void checkRecovered(unsigned);
    void setAgeTrialEnrollment(int age_){tAge = age_;}
    void infect(int, unsigned, RandomNumGenerator *, std::map<unsigned,double> *, std::map<unsigned,double> *, double);
    void initiateBodySize(unsigned,RandomNumGenerator&);
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

    int getAgeDays(unsigned) const;
    int getPreviousInfections();
    int getRecentType(){return last_serotype;}
    int getRecentDis(){return recent_dis;}
    int getRecentHosp(){return recent_hosp;}
    int getRecentInf(){return recent_inf;}
    int getCohort(){return cohort;}
    int getHouseMemNum() const;
    int getTrajDay(){return trajDay;}
    int getVaccinationDay(){return vday;}
    int getAgeTrialEnrollment(){return tAge;}

    unsigned getImmEndDay() const;
    unsigned getImmStartDay() const;
    unsigned getVaxImmEndDay() const;
    unsigned getVaxImmStartDay() const;

    double getAttractiveness() const;
    double getBodySize() const;

    char getGender() const;

    std::string getCurrentLoc(double);
    std::string getHouseID() const;
    std::set<std::string> getLocsVisited();
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;

    bool getSeroStatusAtVaccination(){return seroStatusAtVaccination;}
    bool isHospitalized(){return hospitalized;}
    bool isImmune(unsigned) const;
    bool isImmuneTemp(){return immunity_temp;}
    bool isImmuneVax(){return vaccineImmunity;}
    bool isInfected(){return infected;}
    bool isSymptomatic(){return symptomatic;}
    bool isVaccinated(){return vaccinated;}

    struct sortid{
        bool operator() (const Human *a, const Human *b)const{
            return b->getHouseID() + std::to_string(b->getHouseMemNum()) > a->getHouseID() + std::to_string(a->getHouseMemNum());
        }
    };
};

#endif	/* HUMAN_H */
