/* 
 * File:   Mosquito.h
 * Author: amit
 *
 * Created on August 25, 2014, 12:53 AM
 */

#ifndef MOSQUITO_H
#define	MOSQUITO_H

#include <string>
#include <memory>
#include <vector>
#include "Infection.h"

class Mosquito {
public:
    enum MozState {BITE, REST};
    unsigned getMosquitoID() const;
    std::string getLocationID() const;
    void setLocation(std::string);
    unsigned getDDay() const;
    unsigned getMID() const;
    MozState getState() const;
    void setState(MozState);
    void setBiteStartDay(unsigned);
    unsigned getBiteStartDay();
    void setFly(bool);
    bool getFly() const;
    std::string printInfections() const;
    std::string toString() const;
    Mosquito(unsigned long, unsigned, unsigned, std::string);
    Mosquito();
    Mosquito(const Mosquito& orig);
    virtual ~Mosquito();
    std::unique_ptr<Infection> infection;
private:
    unsigned long mID;
    MozState mState;
    std::string locationID;
    unsigned bday;
    unsigned dday;
    unsigned biteStartDay;
    bool fly;
    
};

#endif	/* MOSQUITO_H */

