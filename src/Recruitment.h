#ifndef RECRUITMENT_H
#define	RECRUITMENT_H
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <algorithm>
#include <memory>
#include <cmath>
#include "RandomNumGenerator.h"
#include "Vaccine.h"
#include "Surveillance.h"
#include "Human.h"

using std::string;
using std::vector;
using std::map;
// allow for shuffle without random access
using eligible_t = vector<Human *>;
// filled from above
using recruit_t = std::set<Human *>;

class Recruitment {
    struct groupStruct{
        int min;
        int max;
        eligible_t eligible;
        recruit_t placebo;
        recruit_t vaccine;
    };

 private:
    RandomNumGenerator * rGen;
    Surveillance trialSurveillance;

    int vaccineSampleSize;
    int placeboSampleSize;
    int recruitmentTimeFrame;
    int recruitmentStartDay;
    int dailyVaccineRecruitmentRate;
    int dailyPlaceboRecruitmentRate;
    int trialDurationDays;

    string recruitmentStrategy;
    string outSurveillance;
    vector<groupStruct> ageGroups;

    unsigned vaccineProfile;
    unsigned placeboProfile;

    double dropoutRate;

    int getAgeGroup(int, vector<groupStruct>);
    int getPossibleAgeGroup(int, vector<groupStruct>, int);
    int parseInteger(string);
    double parseDouble(string); 

    void parseAges(string line, vector<groupStruct> *);
    void parseVector(string line, vector<double> *);
    void enrollTodayParticipants(int);
    void enrollArmParticipants(recruit_t &, eligible_t &, string, int, int, int, int, int,unsigned);
    string parseString(string);
    vector<string> getParamsLine(string);
    map<unsigned, Vaccine> vaccinesPtr;

 public:
    
    Recruitment(string);
    Recruitment();
    Recruitment(const Recruitment& orig);
    //virtual ~Recruitment();

    void update(int);
    void updateArm(unsigned, recruit_t &, int);
    // pseudo-ctor
    void setupRecruitment(string, map<unsigned,Vaccine>, string, string, RandomNumGenerator * _rGen);
    void addPossibleParticipant(Human *, int);
    void shuffleEligibleParticipants();
    void updateParticipants(int);
    void finalizeTrial(int);
    void removeParticipant(Human *, int);
    int getVaccineSampleSize(){return vaccineSampleSize;}
    int getPlaceboSampleSize(){return placeboSampleSize;}
    int getRecruitmentStartDay(){return recruitmentStartDay;}
    long int getEligibleParticipantsSize();
};

#endif	/* RECRUITMENT_H */
