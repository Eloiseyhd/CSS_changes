/* 
 * File:   RandomNumGenerator.h
 * Author: amit
 *
 * Created on August 28, 2014, 4:17 AM
 */

#ifndef RANDOMNUMGENERATOR_H
#define	RANDOMNUMGENERATOR_H

#include <random>
#include <map>

class RandomNumGenerator {
public:
    double getWaningTime(unsigned);
    unsigned getMozEmerge(double);
    double getMozLifeSpan();
    unsigned getHumanTrajectory();
    unsigned getHumanImmunity();
    bool getHumanSeropositivity(double, double);
    unsigned getRandomNum(unsigned);
    unsigned intialInfDaysLeft();
    double getEventProbability();
    unsigned getMozLatencyDays();
    unsigned getHuLatencyDays();
    double getMozRestDays();
    unsigned getMozNextLoc(unsigned);
    void setSeed(unsigned);
    std::string toString() const;
    RandomNumGenerator(unsigned, unsigned, unsigned, unsigned, double, double, unsigned, unsigned, unsigned, unsigned, std::map<unsigned,double>);
    RandomNumGenerator();
    RandomNumGenerator(const RandomNumGenerator& orig);
    virtual ~RandomNumGenerator();
private:
    unsigned seed;
    std::mt19937 gen;
    unsigned huLatencyLo;
    unsigned huLatencyHi;
    unsigned huImmunity;
    double emergeFactor;
    double mozLife;
    unsigned mozLatencyLo;
    unsigned mozLatencyHi;
    double mozRestLo;
    double mozRestHi;
    std::map<unsigned,double> halflife;
};

#endif	/* RANDOMNUMGENERATOR_H */

