#ifndef SIMULATION_H
#define SIMULATION_H

#include <map>
#include <set>
#include <memory>
#include <fstream>
#include "Location.h"
#include "Mosquito.h"
#include "RandomNumGenerator.h"

class Simulation {
public:
    RandomNumGenerator rGen;

    Simulation(std::string);
    Simulation();
    Simulation(const Simulation& orig);
    std::string readInputs();
    void readHumanFile(std::string);
    void readInitialInfectionsFile(std::string);
    void readSimControlFile(std::string);
    void readLocationFile(std::string);
    void readNeighborhoodFile(std::string);
    void readMortalityFile();
    void readVaccineProfileFile();
    void setLocNeighborhood(double);
    void printLocations() const;
    void printHumans() const;
    void printMosquitoes() const; 
    void simEngine();
    void humanDynamics();
    void tests();
    void mosquitoDynamics();
    void generateMosquitoes();
    unsigned setInitialInfection(double, unsigned);
    void printSimulationParams() const;
    void writeNeighborhoodFile(std::string file) const;
    void createNeighborhoodFile(std::string, std::string, std::string);
    void updatePop();
    void writePop();
    void resetPop();
    void simulate();
    virtual ~Simulation();
private:
    std::map <std::string,std::unique_ptr<Location>> locations;
    std::multimap<std::string,std::unique_ptr<Mosquito>> mosquitoes;
    std::multimap<std::string,std::unique_ptr<Human>> humans;
    std::map<int,double> mortalityHuman;
    std::map<unsigned,double> VE_pos;
    std::map<unsigned,double> VE_neg;
    std::map<unsigned,double> halflife;
    unsigned currentDay;
    unsigned numDays;
    unsigned long mozID=0;
    std::string trajectoryFile;
    std::string configLine;
    std::string locationFile;
    std::string neighborhoodFile;
    std::string mortalityFile;
    std::string vaccineProfileFile;
    std::string vaccinationStrategy;
    std::string initialInfectionsFile;
    std::string outputFile;
    std::string outputPopFile;
    std::string simName;
    unsigned rSeed;
    std::string outputPath;
    unsigned humanInfectionDays;
    unsigned hllo;
    unsigned hlhi;
    unsigned huImm;
    double mlife;
    unsigned mlho;
    unsigned mlhi;
    unsigned mrestlo;
    unsigned mresthi;
    double emergeFactor;
    double biteProbablity;
    double mozInfectiousness;   
    double mozMoveProbability;
    double FOI;
    unsigned year;
    std::ofstream out;
    std::ofstream outpop;    
    std::map<std::pair<unsigned,unsigned>,unsigned> seroage_pop;
    std::map<std::pair<unsigned,unsigned>,unsigned> seroage_doses;
};

#endif  /* SIMULATION_H */
