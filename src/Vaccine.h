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
    double seroposVE;
    double seronegVE;
    double seroposWaning;
    double seronegWaning;
    double RRInf_seropos;
    double RRInf_seroneg;
    double RRDis_seropos;
    double RRDis_seroneg;
    double RRHosp_seropos;
    double RRHosp_seroneg;

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
    void setWaning(bool seroposIn, double w);
    void setVaccineEfficacy(bool seroposIn, double w);
    void setRRInf(bool seroposIn, double rr);
    void setRRDis(bool seroposIn, double rr);
    void setRRHosp(bool seroposIn, double rr);
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
    double getVE(){return total_VE;}
    double getVaccineEfficacy(bool seroposIn);
    double getRRInf(bool seroposIn);
    double getRRDis(bool seroposIn);
    double getRRHosp(bool seroposIn);
    double getWaning(bool seroposIn);
};

#endif	/* VACCINE_H */
