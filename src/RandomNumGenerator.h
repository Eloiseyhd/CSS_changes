#ifndef RANDOMNUMGENERATOR_H
#define	RANDOMNUMGENERATOR_H

#include <random>
#include <map>

class RandomNumGenerator {
public:
    unsigned getMozEmerge(double);
    unsigned getHumanTrajectory();
    unsigned getHumanImmunity();
    unsigned getVaxHumanImmunity(unsigned);
    unsigned getRandomNum(unsigned);
    unsigned getMozNextLoc(unsigned);

    bool getHumanSeropositivity(double, double);

    double getWaningTime(double);
    double getMozLifeSpan();
    double getMozLifeSpan(double);
    double getMozDeathRate(double);
    double getRandomNormal();
    double getEventProbability();
    double getMozLatencyDays(double);
    double getMozLatencyRate(double);
    double getMozRestDays();
    double getMozRestDays(double);

    int getSelfReportDay(double);

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
