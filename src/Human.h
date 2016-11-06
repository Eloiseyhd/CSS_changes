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
//
#include "defines.h"
#include "Infection.h"
#include "Vaccine.h"
#include "RandomNumGenerator.h"

using std::pair;
using std::string;
using std::vector;
using std::unique_ptr;

class Human;
// type alias, used in containers of humans
// e.g. Location.h and Simulation.h
using sp_human_t = std::shared_ptr<Human>;
// used to contruct trajectories
using path_t = pair<string,double>;  
using vpath_t = vector<path_t>;
using traject_t = std::array<vpath_t, N_TRAJECTORY >;

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

    bool dead;
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

    Vaccine vaccineProfile;

    std::string houseID;
    std::string personID;
    std::string trialArm;
    std::map<unsigned,bool> immunity_perm;
    std::unique_ptr<traject_t> trajectories;
    // return by reference
    std::set<std::string> locsVisited;
public:
    std::unique_ptr<Infection> infection;
    //Human(std::string,int,int,char,std::unique_ptr<std::vector<std::vector<std::pair<std::string,double>>>>&,RandomNumGenerator&,unsigned,std::vector<double>);
    Human(std::string,int,char,int,int, RandomNumGenerator&);
    Human();
    //    Human(const Human &orig);
    //virtual ~Human();

    void setTrajectories(std::unique_ptr<traject_t> &);
    void initializeHuman(unsigned,std::vector<double>, RandomNumGenerator&);
    void kill(){dead = true;}
    void checkRecovered(unsigned);
    void setAgeTrialEnrollment(int age_){tAge = age_;}
    void enrollInTrial(int, std::string);
    void setSelfReportProb(double prob_){selfReportProb = prob_;}
    void infect(int, unsigned, RandomNumGenerator *, std::map<unsigned,double> *, std::map<unsigned,double> *);
    void initiateBodySize(unsigned,RandomNumGenerator&);
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
    void vaccinateWithProfile(int, RandomNumGenerator *, Vaccine);
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
    bool isDead(){return dead;}
    Vaccine * getVaccine(){return &vaccineProfile;}
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

    const std::string & getCurrentLoc(double);
    const std::string & getHouseID() const;
    const std::string & getPersonID() const;
    std::string getTrialArm(){return trialArm;}
    const std::set<std::string> & getLocsVisited();
    //traject_t const& getTrajectory(unsigned) const;

    struct sortid{
        bool operator() (const sp_human_t a, const sp_human_t b)const{
            return b->getPersonID() > a->getPersonID();
        }
    };
};

#endif	/* HUMAN_H */
