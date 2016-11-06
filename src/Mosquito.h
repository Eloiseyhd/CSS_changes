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
#include "Human.h"

class Mosquito {
public:
    std::string getLocationID() const;
    void setLocation(std::string);
    void setBirthDay(int currDay) {bday = currDay;}
    int getBirthDay(){return bday;}
    double getDDay() const;
    void setBiteStartDay(double);
    double getBiteStartDay();
    double getNumBites(){return nbites;}
    bool takeBite(double,Location *,RandomNumGenerator *, RandomNumGenerator *,std::map<unsigned,double> *,std::map<unsigned,double> *,int,int,std::ofstream *, double);
    sp_human_t whoBite(double,Location *,RandomNumGenerator *);
    bool infectingBite(double,Location *,RandomNumGenerator *,RandomNumGenerator *,int,int, double);
    bool infectiousBite(double,Location *,RandomNumGenerator *,RandomNumGenerator *,std::map<unsigned,double> *,std::map<unsigned,double> *,int,int,std::ofstream *);
    Mosquito(double, double, std::string);
    Mosquito();
    Mosquito(const Mosquito& orig);
    //virtual ~Mosquito();
    std::unique_ptr<Infection> infection;
private:
    std::string locationID;
    double dday;
    double biteStartDay;    
    int nbites;
    int bday;
};

#endif	/* MOSQUITO_H */
