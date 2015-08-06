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
    int getStartDay() const;
    int getEndDay() const;
    double getInfectiousness() const;
    unsigned getInfectionType() const;
    void setInfectiousness(double);
    std::string toString() const;
    Infection(unsigned, unsigned, double, unsigned);
    Infection();
    Infection(const Infection& orig);
    virtual ~Infection();
private:
    int startDay;
    int endDay;
    double infectiousness;
    unsigned infType;
};

#endif	/* INFECTION_H */

