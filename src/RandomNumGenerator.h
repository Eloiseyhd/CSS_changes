#ifndef RANDOMNUMGENERATOR_H
#define	RANDOMNUMGENERATOR_H

#include <random>
#include <map>

class RandomNumGenerator {
public:
    double getWaningTime(double);
    unsigned getMozEmerge(double);
    unsigned getMozEmerge(double, double);
    double getMozLifeSpan();
    unsigned getHumanTrajectory();
    unsigned getHumanImmunity();
    unsigned getVaxHumanImmunity(unsigned);
    bool getHumanSeropositivity(double, double);
    unsigned getRandomNum(unsigned);
    double getRandomNormal();
    double getEventProbability();
    double getMozLatencyDays();
    double getMozLatencyRate(double);
    double getMozRestDays();
    double getMozRestDays(double);
    unsigned getMozNextLoc(unsigned);
    void setSeed(unsigned);
    std::string toString() const;
    RandomNumGenerator(unsigned, unsigned, double, double, double, std::map<unsigned,double>);
    RandomNumGenerator();
    RandomNumGenerator(const RandomNumGenerator& orig);
    virtual ~RandomNumGenerator();
private:
    unsigned seed;
    std::mt19937 gen;
    unsigned huImmunity;
    double emergeFactor;
    double mozLife;
    double mozRest;
    std::map<unsigned,double> halflife;
};

#endif	/* RANDOMNUMGENERATOR_H */
