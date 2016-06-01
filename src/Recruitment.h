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
#include "Human.h"
#include "Surveillance.h"


class Recruitment {
    struct groupStruct{
	int min;
	int max;
	double dropoutRate;
	std::vector<Human *> placebo;
	std::vector<Human *> vaccine;
	std::vector<Human *> eligible;
    };

 private:
    Surveillance trialSurveillance;

    int vaccineSampleSize;
    int placeboSampleSize;
    int recruitmentTimeFrame;
    int recruitmentStartDay;
    int dailyVaccineRecruitmentRate;
    int dailyPlaceboRecruitmentRate;

    std::string recruitmentStrategy;
    std::vector<groupStruct> ageGroups;

    unsigned vaccineProfile;
    unsigned placeboProfile;


    int getAgeGroup(int, std::vector<groupStruct>);
    int getPossibleAgeGroup(int, std::vector<groupStruct>, int);
    int parseInteger(std::string);
    double parseDouble(std::string); 

    void parseAges(std::string line, std::vector<groupStruct> *);
    void parseVector(std::string line, std::vector<double> *);
    void enrollTodayParticipants(int, RandomNumGenerator *);

    std::string parseString(std::string);
    std::vector<std::string> getParamsLine(std::string);
    std::map<unsigned, vProfile> * vaccinesPtr;
 public:
    
    Recruitment(std::string);
    Recruitment();
    Recruitment(const Recruitment& orig);
    virtual ~Recruitment();

    void update(int, RandomNumGenerator *);
    void setupRecruitment(std::string, std::map<unsigned,vProfile> *);
    void addPossibleParticipant(Human *, int);
    void shuffleEligibleParticipants();
    void updateParticipants(int, RandomNumGenerator *);
    void finalizeTrial(int);

    int getVaccineSampleSize(){return vaccineSampleSize;}
    int getPlaceboSampleSize(){return placeboSampleSize;}
    int getRecruitmentStartDay(){return recruitmentStartDay;}
    long int getEligibleParticipantsSize();
};

#endif	/* RECRUITMENT_H */
