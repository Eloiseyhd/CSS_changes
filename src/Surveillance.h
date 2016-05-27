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

class Surveillance {
    struct hRecord{
	int ageDaysAtVaccination;
	int leftCensored[4];
	int rightCensored[4];
	int symptoms[4];
	int hosp[4];
	int lastType;
	double onset[4];
	bool seroStatusAtVaccination;
	std::string trialArm;
	std::vector<std::string> primary;
	std::vector<std::string> pcr;
    };
 public:
    Surveillance();
    Surveillance(const Surveillance& orig);
    virtual ~Surveillance();

    void update_human_surveillance(Human *, int, RandomNumGenerator *);
    void initialize_human_surveillance(Human *, int);
    void finalizeRecord(Human *, int);
    void setup(std::string);
    void printRecords(std::string);
 private:
    int contactPerson(Human *, int, RandomNumGenerator *);
    int contactFrequency;
    int firstContactDelay;
    double selfReportProb;

    int PCR_test(Human *, int, RandomNumGenerator *);
    int parseInteger(std::string);
    double parseDouble(std::string);
    std::vector<std::string> getParamsLine(std::string);
    std::map<std::string, hRecord> recordsDatabase;
};

#endif	/* SURVEILLANCE_H */
