#ifndef MOSQUITO_H
#define	MOSQUITO_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include "Infection.h"
#include "Location.h"
#include "RandomNumGenerator.h"

class Mosquito {
public:
    enum MozState {BITE, REST};
    unsigned getMosquitoID() const;
    std::string getLocationID() const;
    void setLocation(std::string);
    double getDDay() const;
    unsigned getMID() const;
    MozState getState() const;
    void setState(MozState);
    void setBiteStartDay(double);
    double getBiteStartDay();
    void setFly(bool);
    bool getFly() const;
    void takeBite(double,Location *,RandomNumGenerator *, RandomNumGenerator *,int,int,std::ofstream *);
    Human * whoBite(double,Location *,RandomNumGenerator *);
    void infectingBite(double,Location *,RandomNumGenerator *,RandomNumGenerator *,int,int);
    void infectiousBite(double,Location *,RandomNumGenerator *,RandomNumGenerator *,int,int,std::ofstream *);
    std::string printInfections() const;
    std::string toString() const;
    Mosquito(unsigned long, unsigned, double, double, std::string);
    Mosquito();
    Mosquito(const Mosquito& orig);
    virtual ~Mosquito();
    std::unique_ptr<Infection> infection;
private:
    unsigned long mID;
    MozState mState;
    std::string locationID;
    unsigned bday;
    double dday;
    double biteStartDay;
    bool fly;
    
};

#endif	/* MOSQUITO_H */

