#ifndef INFECTION_H
#define	INFECTION_H

#include <cmath>
#include <string>
#include <iostream>

class Infection {
public:
    int getStartDay() const;
    int getEndDay() const;
    double getInfectiousness() const;
    unsigned getInfectionType() const;
    void setInfectiousnessHuman(int);
    void setInfectiousnessMosquito(double);
    std::string toString() const;
    Infection(unsigned, unsigned, double, unsigned, bool, bool);
    Infection();
    Infection(const Infection& orig);
    virtual ~Infection();
private:
    int startDay;
    int endDay;
    double infectiousness;
    unsigned infType;
    bool primary;
    bool symptomatic;
};

#endif	/* INFECTION_H */

