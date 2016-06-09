#ifndef VACCINE_H
#define	VACCINE_H

#include <string>
#include <iostream>
#include <utility>
#include <memory>
#include <cmath>
#include <map>
#include <vector>
#include <cstdlib>

#include "RandomNumGenerator.h"

class Vaccine {

private:
    unsigned vaccineID;
    int doses;
    double waning;
    double protection;
    double total_VE;
    double propInf;
    double normdev;

    std::string mode;
    std::string name;

    std::map<unsigned,double> VE_pos;
    std::map<unsigned,double> VE_neg;
    std::vector<int> relative_schedule;

public:
    Vaccine();
    virtual ~Vaccine();

    void setID(unsigned id){vaccineID = id;}
    void setMode(std::string m){mode = m;}
    void setName(std::string n){name = n;}
    void setWaning(double w){waning = w;}
    void setProtection(double p){protection = p;}
    void setTotalVE(double ve){total_VE = ve;}
    void addVE_pos(double v, unsigned a){VE_pos.at(a) = v;}
    void addVE_neg(double v, unsigned a){VE_neg.at(a) = v;}
    void setPropInf(double p){this->propInf = p;}
    void setNormdev(double n){this->normdev = n;}
    void setRelativeSchedule(std::vector<int>);
    void printVaccine();

    unsigned getVaccineID(){return vaccineID;}
    std::string getMode(){return mode;}
    int getDoses(){return doses;}
    int getNextDoseDay(int, int);
    double getPropInf(){return propInf;}
    double getVaccineProtection(){return protection;}
    double getWaning(){return waning;}
    double getRR(double, double);

};

#endif	/* VACCINE_H */
