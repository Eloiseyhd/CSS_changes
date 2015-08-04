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
#include <memory>
#include <fstream>
#include "RandomNumGenerator.h"
#include "Human.h"

class Location {
private:
    std::string locID;
    double xCor;
    double yCor;
    std::string locType;
    std::unique_ptr<std::vector<std::string>> closeLocs;
    //std::unique_ptr<std::vector<std::unique_ptr<Human>>> humans;
public:
    std::string getRandomCloseLoc(RandomNumGenerator&);
    //void printHumans() const;
    //void addHuman(std::unique_ptr<Human>);
    void addCloseLoc(std::string);
    double getDistanceFromLoc(Location &) const;
    double getLocX() const;
    double getLocY() const;
    std::string getLocID() const;
    std::string getLocType() const;
    std::string toString() const;
    void writeNeighToFile(std::ofstream&) const;
    Location(std::string, std::string, double, double);
    Location();
    Location(const Location& orig);
    virtual ~Location();
private:

};

#endif	/* LOCATION_H */

