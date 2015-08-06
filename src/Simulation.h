 /* 
 * File:   Simulation.h
 * Author: amit
 *
 * Created on August 25, 2014, 12:52 AM
 */

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
    double dnv[3][9] = {{0.992957746, 0.978873239, 0.950704225, 0.894366197, 0.718309859, 0.492957746, 0.26056338, 0.105633803, 0.035211268}, 
                        {0.964789, 0.929577, 0.880282, 0.78169, 0.661972, 0.492958, 0.330986, 0.211268, 0.126761},
                        {0.971631206, 0.929078014, 0.808510638, 0.609929078, 0.368794326, 0.184397163, 0.070921986, 0.021276596, 0.007092199}};
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
    void readMortalityFile(std::string);
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
    void simulate();
    virtual ~Simulation();
private:
    std::map <std::string,std::unique_ptr<Location>> locations;
    std::multimap<std::string,std::unique_ptr<Mosquito>> mosquitoes;
    std::multimap<std::string,std::unique_ptr<Human>> humans;
    std::map<int,double> mortalityHuman;
    unsigned currentDay;
    unsigned numDays;
    unsigned long mozID=0;
    std::string trajectoryFile;
    std::string configLine;
    std::string locationFile;
    std::string neighborhoodFile;
    std::string mortalityFile;
    std::string initialInfectionsFile;
    std::string outputFile;
    std::string simName;
    unsigned rSeed;
    std::string outputPath;
    unsigned humanInfectionDays;
    double emergeFactor;
    double biteProbablity;
    double mozInfectiousness;   
    double mozMoveProbability;
    double dnv1IniProp;
    double dnv2IniProp;
    double dnv3IniProp;
    std::ofstream out;
};

#endif  /* SIMULATION_H */
