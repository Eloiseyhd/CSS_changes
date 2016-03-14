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
    double xCor;
    double yCor;
    double emergenceRate;
    bool infectedVisitor;
    std::string locType;
    std::unique_ptr<std::vector<std::string>> closeLocs;
    std::set<Human *,Human::sortid> humans;

public:
    std::string getRandomCloseLoc(RandomNumGenerator&);
    void addHuman(Human *);
    std::set<Human *,Human::sortid>* getHumans(){return &humans;}
    void addCloseLoc(std::string);
    double getDistanceFromLoc(Location &) const;
    double getLocX() const;
    double getLocY() const;
    double getEmergenceRate() const;
    bool getInfectedVisitor(){return infectedVisitor;}
    std::string getLocID() const;
    std::string getLocType() const;
    Location(std::string, std::string, double, double, double);
    Location();
    Location(const Location& orig);
    virtual ~Location();
    void updateInfectedVisitor();

private:

};

#endif	/* LOCATION_H */

