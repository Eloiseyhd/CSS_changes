#ifndef LOCATION_H
#define	LOCATION_H

#include <string>
#include <vector>
#include <set>
#include <forward_list>
#include <memory>
#include <fstream>
#include "RandomNumGenerator.h"
#include "Human.h"

class Location {
private:
    std::string locID;
    std::string neighborhoodID;
    double xCor;
    double yCor;
    double emergenceRate;
    bool infectedVisitor;
    std::string locType;
    std::unique_ptr<std::vector<std::string>> closeLocs;
    std::set<sp_human_t,Human::sortid> humans;

public:
    std::string getRandomCloseLoc(RandomNumGenerator&);
    void addHuman(sp_human_t);
    void removeHuman(sp_human_t);
    std::set<sp_human_t,Human::sortid> & getHumans(){return humans;}
    void addCloseLoc(std::string);
    double getDistanceFromLoc(Location &) const;
    double getLocX() const;
    double getLocY() const;
    double getEmergenceRate() const;
    bool getInfectedVisitor(){return infectedVisitor;}
    std::string getLocID() const;
    std::string getLocType() const;
    std::string getNeighID() const;
    Location(std::string, std::string, double, double, double);
    Location(std::string, std::string, std::string, double, double, double);
    Location();
    Location(const Location& orig);
    //virtual ~Location();
    void updateInfectedVisitor();

private:

};

#endif	/* LOCATION_H */

