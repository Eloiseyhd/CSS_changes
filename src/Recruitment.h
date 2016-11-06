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

class Recruitment {
    struct groupStruct{
        int min;
        int max;
        vector<Human *> placebo;
        vector<Human *> vaccine;
        vector<Human *> eligible;
    };

 private:
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
    void enrollTodayParticipants(int, RandomNumGenerator *);
    void enrollArmParticipants(vector<Human *> *, vector<Human *> *, string, int, int, int, int, int,unsigned, RandomNumGenerator *);
    string parseString(string);
    vector<string> getParamsLine(string);
    map<unsigned, Vaccine> vaccinesPtr;

 public:
    
    Recruitment(string);
    Recruitment();
    Recruitment(const Recruitment& orig);
    //virtual ~Recruitment();

    void update(int, RandomNumGenerator *);
    void updateArm(unsigned, vector<Human *> *, int, RandomNumGenerator *);
    void setupRecruitment(string, map<unsigned,Vaccine>, string, string);
    void addPossibleParticipant(Human *, int);
    void shuffleEligibleParticipants(RandomNumGenerator & refGen);
    void updateParticipants(int, RandomNumGenerator *);
    void finalizeTrial(int);
    void removeParticipant(Human *, int);
    int getVaccineSampleSize(){return vaccineSampleSize;}
    int getPlaceboSampleSize(){return placeboSampleSize;}
    int getRecruitmentStartDay(){return recruitmentStartDay;}
    long int getEligibleParticipantsSize();
};

#endif	/* RECRUITMENT_H */
