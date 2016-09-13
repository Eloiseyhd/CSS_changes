#ifndef SURVEILLANCE_H
#define	SURVEILLANCE_H

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

using namespace std;

class Surveillance {
    struct hRecord{
	int ageDaysAtVaccination;
	int TTL[4];
	int TTR[4];
	int numExp[4];
	int firstTTL;
	int firstTTR;
	int firstExp;
	int symptoms[4];
	int hosp[4];
	int pcrDay[4];
	int lastType;
	int dropoutDay;
	int enrollmentDay;
	int houseMemNum;
	double onset[4];
	bool seroStatusAtVaccination;
	bool previousExposure[4];
	std::string trialArm;
	std::vector<std::string> primary;
	std::vector<std::string> pcr;
	std::string firstPCR;
	std::string houseID;
    };
 public:
    Surveillance();
    Surveillance(const Surveillance& orig);
    virtual ~Surveillance();

    void update_human_surveillance(Human *, int, RandomNumGenerator *);
    void initialize_human_surveillance(Human *, int);
    void finalize_human_surveillance(Human *, int);
    void setup(std::string);
    void printRecords(std::string, int);

 private:
    int contactFrequency;
    int firstContactDelay;
    double selfReportProb;
    double reportTodayProb;

    std::map<std::string, hRecord> recordsDatabase;
    std::map<std::string, std::string> parameters;

    void contactPerson(Human *, int, RandomNumGenerator *);
    void addParameter(std::string);
    void join(const vector<std::string>& , char , string& );
    int PCR_test(Human *, int, RandomNumGenerator *);
    int parseInteger(std::string);
    int readParameter(std::string, int );
    double parseDouble(std::string);
    double readParameter(std::string, double );
};

#endif	/* SURVEILLANCE_H */
