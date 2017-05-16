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

using std::string;
using std::vector;

class Location {
private:
    string locID;
    string neighborhoodID;
    string zoneID;
    double xCor;
    double yCor;
    double emergenceRate;
    bool infectedVisitor;
    string locType;
    std::unique_ptr<vector<string>> closeLocs;
    std::set<sp_human_t,Human::sortid> humans;
    
public:
    string getRandomCloseLoc(RandomNumGenerator&);
    void addHuman(sp_human_t);
    void removeHuman(sp_human_t);
    std::set<sp_human_t,Human::sortid> & getHumans(){return humans;}
    void addCloseLoc(string);
    void printHumans();
    double getDistanceFromLoc(Location &) const;
    double getLocX() const;
    double getLocY() const;
    double getEmergenceRate() const;
    bool getInfectedVisitor(){return infectedVisitor;}
    string getLocID() const;
    string getLocType() const;
    string getNeighID() const;
    string getZoneID() const;
    Location(string, string, double, double, double);
    Location(string, string, string, string, double, double, double);
    Location();
    Location(const Location& orig);
    //virtual ~Location();
    void updateInfectedVisitor();

private:

};

#endif	/* LOCATION_H */

