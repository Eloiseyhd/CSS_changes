/* 
 * File:   Location.h
 * Author: amit
 *
 * Created on August 25, 2014, 12:55 AM
 */


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
    double mozzes;
    std::string locType;
    std::unique_ptr<std::vector<std::string>> closeLocs;
    // std::forward_list<std::unique_ptr<Human>> humansVisit;
    std::set<Human *> humans;
public:
    std::string getRandomCloseLoc(RandomNumGenerator&);
    // void printHumans() const;
    void addHuman(Human *);
    std::set<Human *>* getHumans(){return &humans;}
    void addCloseLoc(std::string);
    double getDistanceFromLoc(Location &) const;
    double getLocX() const;
    double getLocY() const;
    double getMozzes() const;
    std::string getLocID() const;
    std::string getLocType() const;
    std::string toString() const;
    void writeNeighToFile(std::ofstream&) const;
    Location(std::string, std::string, double, double, double);
    Location();
    Location(const Location& orig);
    virtual ~Location();
private:

};

#endif	/* LOCATION_H */

