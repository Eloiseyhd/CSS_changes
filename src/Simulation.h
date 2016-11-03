#ifndef SIMULATION_H
#define SIMULATION_H

#include <map>
#include <set>
#include <memory>
#include <fstream>
#include <queue>
#include "Location.h"
#include "Mosquito.h"
#include "RandomNumGenerator.h"
#include "Report.h"
#include "Vaccine.h"
#include "Recruitment.h"


class Simulation {
public:
    RandomNumGenerator rGen;
    RandomNumGenerator rGenInf;

    Simulation(std::string);
    Simulation();
    Simulation(const Simulation& orig);
    std::string readInputs();
    void readTrajectoryFile(std::string);
    void readHumanFile(std::string);
    void readBirthsFile(std::string);
    void readInitialInfectionsFile(std::string);
    void readSimControlFile(std::string);
    void readLocationFile(std::string);
    void readDiseaseRatesFile();
    void readVaccineProfilesFile();
    void readVaccinationGroupsFile();
    void readVaccineSettingsFile();
    void readAegyptiFile(std::string);
    void readInitialFOI(std::string);
    void readAnnualFOI(std::string);
    void setLocNeighborhood(double);
    void simEngine();
    void humanDynamics();
    void tests();
    void mosquitoDynamics();
    void generateMosquitoes();
    unsigned setInitialInfection(double, unsigned);
    void simulate();
    void updatePop();
    void selectEligibleTrialParticipants();
    bool checkAgeToVaccinate(int age_);
    virtual ~Simulation();
private:
    std::vector<std::string>getParamsLine(std::string);
    int parseInteger(std::string);
    std::string parseString(std::string);
    double parseDouble(std::string);
    void parseVector(std::string line, std::vector<int> *);

    std::map <std::string,std::unique_ptr<Location>> locations;
    std::map<unsigned,double> VE_pos;
    std::map<unsigned,double> VE_neg;
    std::map<unsigned,double> halflife;
    std::map<unsigned,double> disRates;
    std::map<unsigned,double> hospRates;
    std::map<int,int> ageGroups;
    std::map<unsigned, Vaccine> vaccines;
    std::multimap<std::string,std::unique_ptr<Mosquito>> mosquitoes;
    std::multimap<std::string,std::unique_ptr<Human>> humans;
    std::multimap<int,std::unique_ptr<Human>> future_humans;
    std::map<std::string, std::unique_ptr<Human>> total_humans_by_id;

    unsigned currentDay;
    unsigned numDays;
    std::string trajectoryFile;
    std::string birthsFile;
    std::string configLine;
    std::string locationFile;
    std::string vaccineProfilesFile;
    std::string vaccineSettingsFile;
    std::string trialSettingsFile;
    std::string vaccinationStrategy;
    std::string reportsFile;
    std::string diseaseRatesFile;
    std::string vaccinationGroupsFile;
    std::string aegyptiRatesFile;

    bool routineVaccination;
    bool catchupFlag;
    bool trialVaccination;
    bool randomTrial;
    int vaccineID;
    unsigned vaccineDay;
    unsigned vaccineAge;
    double vaccineCoverage;
    Report outputReport;
    Recruitment recruitmentTrial;
    std::string outputFile;
    std::string outputPopFile;
    std::string outputPrevacFile;
    std::string simName;
    unsigned rSeed;
    unsigned rSeedInf;
    std::string outputPath;
    double deathRate;
    unsigned humanInfectionDays;
    unsigned huImm;
    double mlife;
    double mbite;
    double emergeFactor;
    double biteProbablity;
    double mozInfectiousness;   
    double mozMoveProbability;
    double selfReportProb;
    unsigned year;

    int deathMoz;
    int lifeMoz;
    int humanDeaths;
    std::vector<double> meanDailyEIP;
    std::vector<double> firstBiteRate;
    std::vector<double> secondBiteRate;
    std::vector<double> mozDailyDeathRate;
    std::vector<double> dailyEmergenceFactor;
    std::vector<double> InitialConditionsFOI;
    std::vector<std::map<unsigned, double>> annualForceOfImportation;

    std::ofstream out;
    std::ofstream outpop;
    std::ofstream outprevac;
};

#endif  /* SIMULATION_H */
