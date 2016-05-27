#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Surveillance.h"

using namespace std;

Surveillance::Surveillance(){
    contactFrequency = 0;
    firstContactDelay = 0;
    selfReportProb = 0.0;
    recordsDatabase.clear();
}

void Surveillance::setup(std::string file){
    if (file.length() == 0) {
		exit(1);
    }
    string line;
    ifstream infile(file);
    if(!infile.good()){
		exit(1);
    }
    while(getline(infile,line,'\n')){
	string line2,line3;
	std::vector<std::string>param_line = getParamsLine(line);
	line2 = param_line[0];
	line3 = param_line[1];
	if(line2 == "surveillance_contact_frequency"){
	    contactFrequency = this->parseInteger(line3);
	}
	if(line2 == "surveillance_first_contact_delay"){
	    firstContactDelay = this->parseInteger(line3);
	}
	if(line2 == "self_report_probability"){
	    selfReportProb = this->parseDouble(line3);
	}
    }
    printf("Surveillance setup contact frecuency %d first delay %d selfReport probability %.2f\n", contactFrequency, firstContactDelay, selfReportProb);
}

void Surveillance::initialize_human_surveillance(Human * h, int currDay){
    h->setContactByTrial(currDay + firstContactDelay);
    h->setSeroStatusAtVaccination();
    h->setSelfReportProb(selfReportProb);
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    hRecord tempRecord;
    tempRecord.ageDaysAtVaccination = h->getAgeTrialEnrollment();
    tempRecord.seroStatusAtVaccination = h->getSeroStatusAtVaccination();
    tempRecord.trialArm = h->getTrialArm();
    tempRecord.pcr.clear();
    tempRecord.primary.clear();
    for(int i = 0;i < 4; i++){
	tempRecord.leftCensored[i] = currDay;
	tempRecord.rightCensored[i] = -1;
	tempRecord.onset[i] = -1.0;
	tempRecord.symptoms[i] = -1;
	tempRecord.hosp[i] = -1;
	tempRecord.pcr.push_back("NA");
	tempRecord.primary.push_back("NA");
    }
    tempRecord.lastType = -1;
    recordsDatabase.insert(make_pair(id,tempRecord));
}

void Surveillance::update_human_surveillance(Human * h, int currDay, RandomNumGenerator * rGen){
    if(!(h->getTrialArm().empty())){
	std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
	if(h->isInfected()){
	    if(h->infection != NULL){
		// set IIP and serotype and symptoms
		unsigned serotype = h->infection->getInfectionType() - 1;
		recordsDatabase.find(id)->second.onset[serotype] = h->infection->getSymptomOnset();
		recordsDatabase.find(id)->second.lastType = h->infection->getInfectionType() - 1;
		recordsDatabase.find(id)->second.symptoms[serotype] = (h->isSymptomatic() == true) ? 1 : 0;
		recordsDatabase.find(id)->second.hosp[serotype] = (h->isHospitalized() == true) ? 1 : 0;
		recordsDatabase.find(id)->second.primary[serotype] = (h->infection->isPrimary()) ? "primary" : "secondary";
	    }
	}
	if(h->getLastContactByTrial() + contactFrequency == currDay){
	    int serotype_ = this->contactPerson(h, currDay, rGen);
	    if(serotype_ >= 0){
		recordsDatabase.find(id)->second.rightCensored[serotype_] = currDay;
	    }
	    for(int i = 0; i < 4; i++){
		if(recordsDatabase.find(id)->second.rightCensored[i] == -1){
		    recordsDatabase.find(id)->second.leftCensored[i] = currDay;
		}
	    }
	    h->setContactByTrial(currDay);
	}else if(h->getSelfReportDay() == currDay){
	    if(h->infection != NULL){
		if(recordsDatabase.find(id)->second.rightCensored[h->infection->getInfectionType() - 1] == -1){
		    int serotype_ = this->PCR_test(h,currDay,rGen);
		    if(serotype_ >= 0){
			recordsDatabase.find(id)->second.rightCensored[serotype_] = currDay;
		    }
		    //wrap up
		    for(int i = 0; i < 4; i++){
			if(recordsDatabase.find(id)->second.rightCensored[i] == -1){
			    recordsDatabase.find(id)->second.leftCensored[i] = currDay;
			}
		    }
		    h->setContactByTrial(currDay);
		}
	    }
	}
	//annual visits
	
    }
}


int Surveillance::contactPerson(Human * h, int currDay, RandomNumGenerator * rGen){
    // Check level of symptoms in the last week (if is asymptomatic then there's no screening)
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    int serotype = (h->infection != NULL) ? h->infection->getInfectionType() -1 : recordsDatabase.find(id)->second.lastType;
    if(recordsDatabase.find(id)->second.rightCensored[serotype] == -1){
	// If the symptoms happened between last contact and today
	if(recordsDatabase.find(id)->second.symptoms[serotype] > 0 && recordsDatabase.find(id)->second.onset[serotype] <= currDay && recordsDatabase.find(id)->second.onset[serotype] > h->getLastContactByTrial()){
	    int pcr_result =  this->PCR_test(h, currDay, rGen);
	    recordsDatabase.find(id)->second.pcr[serotype] = (pcr_result >= 0) ? "TRUE" : "FALSE";
	    return pcr_result; 
	}
    }
    return -1;
}

int Surveillance::PCR_test(Human * h, int currDay, RandomNumGenerator * rGen){
    int serotype = -1;
    double sensitivity = 0.0;
    if(h->infection != NULL){
	// Is a primary or secondary infection? 
	double b1 = 0.0;
	double b2 = 0.0;
	if(h->infection->isPrimary() == true){
	    b1 = 13.185066;
	    b2 = -1.665468;
	}else{
	    b1 = 6.834631;
	    b2 = -1.166282;
	}
	sensitivity = (1.0 / (1.0 + exp(-1 * (b1 + b2 * (currDay - h->infection->getSymptomOnset())))));
	if(rGen->getEventProbability() < sensitivity){
	    serotype = h->infection->getInfectionType() - 1;
	}
    }
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    std::string pcr_ = serotype >= 0 ? "TRUE" : "FALSE";
    printf("%s, %s,  %.2f, %.4f, %s\n",id.c_str(), pcr_.c_str(),(double) currDay -  recordsDatabase.find(id)->second.onset[serotype],sensitivity,  recordsDatabase.find(id)->second.primary[serotype].c_str());
    return serotype;
}

void Surveillance::finalizeRecord(Human * h, int currDay){
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());

}
void Surveillance::printRecords(std::string file){
    if (file.length() == 0) {
	exit(1);
    }
    std::ofstream outSurveillance;
    outSurveillance.open(file);
    if (!outSurveillance.good()) {
	exit(1);
    }
    outSurveillance << "ID, Age, Arm, Serostatus, onset_1, isSympt_1, PCR_1, TYPE_1, TTEL_1, TTER_1, onset_2, isSympt_2, PCR_2, TYPE_2, TTEL_2, TTER_2, onset_3, isSympt_3, PCR_3, TYPE_3, TTEL_3, TTER_3, onset_4, isSympt_4, PCR_4, TYPE_4, TTEL_4, TTER_4\n";
    std::map<std::string, hRecord>::iterator it;
    for(it = recordsDatabase.begin(); it != recordsDatabase.end(); ++it){
	std::string serostatus = ((*it).second.seroStatusAtVaccination == true) ? "seropos" : "seroneg";
	outSurveillance << (*it).first.c_str() << ", " << (*it).second.ageDaysAtVaccination / 365 << ", " << (*it).second.trialArm.c_str() << ", " << serostatus.c_str() << ", ";
	for(int i = 0; i < 4; i++){
	    std::string sympt = ((*it).second.symptoms[i] == 1) ? "symptomatic" : "asymptomatic";
	    outSurveillance << (*it).second.onset[i] << ", " << sympt << ", " << (*it).second.pcr[i].c_str() << ", " << (*it).second.primary[i] << ", " << (*it).second.leftCensored[i] << ", " << (*it).second.rightCensored[i];
	    if(i == 3){
		outSurveillance << "\n";
	    }else{
		outSurveillance << ", ";
	    }
	} 
    }
    outSurveillance.close();

}


std::vector<std::string> Surveillance::getParamsLine(std::string line_){
    stringstream linetemp;
    string line2_,line3_;
    linetemp.clear();
    linetemp << line_;
    getline(linetemp,line2_,'=');
    getline(linetemp,line3_,'=');
    linetemp.clear();
    linetemp << line2_;
    getline(linetemp,line2_,' ');
    std::vector<std::string> params;
    params.push_back(line2_);
    params.push_back(line3_);
    return params;
}
int Surveillance::parseInteger(std::string line){
    return strtol(line.c_str(), NULL, 10);
}

double Surveillance::parseDouble(std::string line){
    return strtod(line.c_str(), NULL);
}

Surveillance::~Surveillance() {

}
