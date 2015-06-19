/* 
 * File:   Infection.h
 * Author: amit
 *
 * Created on August 28, 2014, 1:56 AM
 */

#ifndef INFECTION_H
#define	INFECTION_H

#include <string>

class Infection {
public:
    enum InfType {DENV1, DENV2, DENV3};
    int getStartDay() const;
    int getEndDay() const;
    double getInfectiousness() const;
    InfType getInfectionType() const;
    void setInfectiousness(double);
    std::string toString() const;
    Infection(unsigned, unsigned, double, InfType);
    Infection();
    Infection(const Infection& orig);
    virtual ~Infection();
private:
    int startDay;
    int endDay;
    double infectiousness;
    InfType infType;
};

#endif	/* INFECTION_H */

