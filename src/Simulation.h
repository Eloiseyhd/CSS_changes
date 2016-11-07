#ifndef SIMULATION_H
#define SIMULATION_H

#include <map>
#include <set>
#include <memory>
#include <fstream>
#include "Location.h"
#include "Mosquito.h"
#include "RandomNumGenerator.h"
#include "Report.h"

class Simulation {
public:
    RandomNumGenerator rGen;
    RandomNumGenerator rGenInf;

    Simulation(std::string);
    Simulation();
    Simulation(const Simulation& orig);
    std::string readInputs();
    void readHumanFile(std::string);
    void readInitialInfectionsFile(std::string);
    void readSimControlFile(std::string);
    void readLocationFile(std::string);
    void readDiseaseRatesFile();
    void readVaccineProfileFile();
    void readVaccinationGroupsFile();
    void readAegyptiFile(std::string);
    void readInitialFOI(std::string);
    void readAnnualFOI(std::string);
    void setLocNeighborhood(double);
    void simEngine();
    void humanDynamics();
    void tests();
    void mosquitoDynamics();
    void generateMosquitoes();
    void simulate();
    void updatePop();
    unsigned setInitialInfection(double, unsigned);
    bool checkAgeToVaccinate(int age_);
    virtual ~Simulation();

private:
    std::multimap<std::string,std::unique_ptr<Mosquito>> mosquitoes;
    std::multimap<std::string,std::unique_ptr<Human>> humans;
    std::map <std::string,std::unique_ptr<Location>> locations;
    std::map<unsigned,double> VE_pos;
    std::map<unsigned,double> VE_neg;
    std::map<unsigned,double> halflife;
    std::map<unsigned,double> disRates;
    std::map<unsigned,double> hospRates;
    std::map<int,int> ageGroups;

    bool routineVaccination;
    bool catchupFlag;
    bool trialVaccination;
    bool vaccineAdvanceMode;
    unsigned vaccineDay;
    unsigned vaccineAge;
    unsigned humanInfectionDays;
    unsigned huImm;
    unsigned year;
    unsigned currentDay;
    unsigned numDays;
    unsigned rSeed;
    unsigned rSeedInf;

    Report outputReport;

    std::string outputFile;
    std::string outputPopFile;
    std::string outputPrevacFile;
    std::string simName;
    std::string outputPath;
    std::string trajectoryFile;
    std::string configLine;
    std::string locationFile;
    std::string vaccineProfileFile;
    std::string vaccinationStrategy;
    std::string reportsFile;
    std::string diseaseRatesFile;
    std::string vaccinationGroupsFile;
    std::string aegyptiRatesFile;

    double normdev;
    double vaccineProtection;
    double vaccineWaning;
    double propInf;
    double vaccineCoverage;
    double deathRate;
    double mlife;
    double mbite;
    double emergeFactor;
    double biteProbablity;
    double mozInfectiousness;   
    double mozMoveProbability;

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
