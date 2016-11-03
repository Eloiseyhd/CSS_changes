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
#include "Vaccine.h"
#include "RandomNumGenerator.h"

class Human {
private:

    unsigned immEndDay;
    unsigned vaxImmEndDay;
    unsigned immStartDay;
    unsigned vaxImmStartDay;

    char gender;

    int bday;
    int dday;
    int houseMemNum;
    int trajDay;
    int recent_dis;
    int recent_hosp;
    int recent_inf;
    int last_serotype;
    int vday;
    int tAge;
    int vaxWaning_pos;
    int vaxWaning_neg;
    int cohort;
    int trialDay;
    int vaccineDosesReceived;
    int lastDayContactedByTrial;
    int exposedCount[4];
    int preExposureAtVaccination[4];

    double attractiveness;
    double bodySize;
    double bodySizeAdult;
    double bodySizeBirth;
    double bodySizeSlope;
    double propInf;
    double normdev;
    double vaccineProtection;
    double selfReportProb;

    bool hospitalized;
    bool immunity_temp;
    bool infected;
    bool seroStatusAtVaccination;
    bool symptomatic;
    bool vaccineImmunity;
    bool vaccinated;
    bool vaccineComplete;
    bool enrolledInTrial;    
    bool reportSymptoms;

    Vaccine * vaccineProfile;

    std::string houseID;
    std::string personID;
    std::string trialArm;
    std::map<unsigned,bool> immunity_perm;
    std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>> trajectories;
    std::map<unsigned,double> * veneg;
    std::map<unsigned,double> * vepos;

public:
    std::unique_ptr<Infection> infection;

    Human(std::string,int,int,char,std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned,std::vector<double>);
    Human(std::string,int,char,int,int, RandomNumGenerator&);
    Human();
    Human(const Human& orig);
    virtual ~Human();

    void setTrajectories(std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&);
    void initializeHuman(unsigned,std::vector<double>, RandomNumGenerator&);
    
    void checkRecovered(unsigned);
    void setAgeTrialEnrollment(int age_){tAge = age_;}
    void enrollInTrial(int, std::string);
    void setSelfReportProb(double prob_){selfReportProb = prob_;}
    void infect(int, unsigned, RandomNumGenerator *, std::map<unsigned,double> *, std::map<unsigned,double> *);
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
    void setContactByTrial(int dayIn){lastDayContactedByTrial = dayIn;}
    void updateAttractiveness(unsigned);
    void updateBodySize(unsigned);
    void updateImmunityPerm(unsigned,bool);
    void updateRecent(int,int,int);
    void vaccinate(int);
    void vaccinateAdvanceMode(int, RandomNumGenerator&);
    void vaccinateGSKMode(int, RandomNumGenerator&);
    void vaccinateWithProfile(int, RandomNumGenerator *, Vaccine *);
    void updateVaccineEfficacy(int);
    void boostVaccine(int, RandomNumGenerator *);
    void unenrollTrial(){enrolledInTrial = false;}
    void setReportSymptoms(bool rIn){reportSymptoms = rIn;}

    double getAttractiveness() const;
    double getBodySize() const;

    char getGender() const;

    unsigned getImmEndDay() const;
    unsigned getImmStartDay() const;
    unsigned getVaxImmEndDay() const;
    unsigned getVaxImmStartDay() const;

    int getAgeDays(unsigned) const;
    int getBirthday()const {return bday;}
    int getDeathday()const {return dday;}
    int getHouseMemNum() const;
    int getPreviousInfections();
    int getRecentType(){return last_serotype;}
    int getRecentDis(){return recent_dis;}
    int getRecentHosp(){return recent_hosp;}
    int getRecentInf(){return recent_inf;}
    int getTrajDay(){return trajDay;}
    int getCohort(){return cohort;}
    int getVaccinationDay(){return vday;}
    int getAgeTrialEnrollment(){return tAge;}
    int getNextDoseDay();
    int getLastContactByTrial(){return lastDayContactedByTrial;}
    int getTrialEnrollmentDay(){return trialDay;}
    int getExposedCount(unsigned sero){return exposedCount[sero];}
    int getPreExposureAtVaccination(unsigned);

    bool isHospitalized(){return hospitalized;}
    bool isImmune(unsigned) const;
    bool isImmuneTemp(){return immunity_temp;}
    bool isImmuneVax(){return vaccineImmunity;}
    bool isInfected(){return infected;}
    bool isSymptomatic(){return symptomatic;}
    bool isVaccinated(){return vaccinated;}
    bool getSeroStatusAtVaccination(){return seroStatusAtVaccination;}
    bool isEnrolledInTrial(){return enrolledInTrial;}
    bool isFullyVaccinated(){return vaccineComplete;}
    bool getReportSymptoms(){return reportSymptoms;}

    std::string getCurrentLoc(double);
    std::string getHouseID() const;
    std::string getPersonID() const;
    std::string getTrialArm(){return trialArm;}
    std::set<std::string> getLocsVisited();
    std::vector<std::pair<std::string,double>> const& getTrajectory(unsigned) const;

    struct sortid{
        bool operator() (const Human *a, const Human *b)const{
            return b->getHouseID() + std::to_string(b->getHouseMemNum()) > a->getHouseID() + std::to_string(a->getHouseMemNum());
        }
    };
};

#endif	/* HUMAN_H */

