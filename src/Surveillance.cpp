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
    tempRecord.houseID = h->getHouseID();
    tempRecord.houseMemNum = h->getHouseMemNum();
    tempRecord.dropoutDay = -1;
    tempRecord.enrollmentDay = currDay;
    tempRecord.pcr.clear();
    tempRecord.primary.clear();
    for(int i = 0;i < 4; i++){
	tempRecord.leftCensored[i] = currDay;
	tempRecord.rightCensored[i] = -1;
	tempRecord.onset[i] = -1.0;
	tempRecord.symptoms[i] = -1;
	tempRecord.hosp[i] = -1;
	tempRecord.previousExposure[i] = h->getPreExposureAtVaccination(i);
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
	//better to save a variable with the next contact day
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

void Surveillance::finalize_human_surveillance(Human *h, int currDay){
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    recordsDatabase.find(id)->second.dropoutDay = currDay;
}

int Surveillance::contactPerson(Human * h, int currDay, RandomNumGenerator * rGen){
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    int serotype = (h->infection != NULL) ? h->infection->getInfectionType() -1 : recordsDatabase.find(id)->second.lastType;
    if(recordsDatabase.find(id)->second.rightCensored[serotype] == -1){
	// If the symptoms happened between last contact and today
	if(recordsDatabase.find(id)->second.symptoms[serotype] > 0 && recordsDatabase.find(id)->second.onset[serotype] <= currDay && recordsDatabase.find(id)->second.onset[serotype] > h->getLastContactByTrial()){
	    int pcr_result =  this->PCR_test(h, currDay + rGen->getRandomNum(4), rGen);
	    recordsDatabase.find(id)->second.pcr[serotype] = (pcr_result >= 0) ? "POSITIVE" : "NA";
	    return pcr_result; 
	}
    }
    return -1;
}

int Surveillance::PCR_test(Human * h, int currDay, RandomNumGenerator * rGen){
    std::string id = h->getHouseID() + std::to_string(h->getHouseMemNum());
    int pcr_result = -1;
    int serotype = (h->infection != NULL) ? h->infection->getInfectionType() -1 : recordsDatabase.find(id)->second.lastType;
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
	    pcr_result = h->infection->getInfectionType() - 1;
	}
    }
    return pcr_result;
}


void Surveillance::printRecords(std::string file, int currDay){
    if (file.length() == 0) {
	exit(1);
    }
    std::ofstream outSurveillance;
    outSurveillance.open(file);
    if (!outSurveillance.good()) {
	exit(1);
    }
    outSurveillance << "ID, Age, Arm, Serostatus, Enrollment_day, Last_day, ";
    outSurveillance << "previous_exposure_1, onset_1, symptoms_1, severity_1, PCR_1, TYPE_1, TTEL_1, TTER_1, previous_exposure_2, onset_2, symptoms_2, severity_2, PCR_2, TYPE_2, TTEL_2, TTER_2, ";
    outSurveillance << "previous_exposure_3, onset_3, symptoms_3, severity_3, PCR_3, TYPE_3, TTEL_3, TTER_3, previous_exposure_4, onset_4, symptoms_4, severity_4, PCR_4, TYPE_4, TTEL_4, TTER_4\n";
    std::map<std::string, hRecord>::iterator it;
    for(it = recordsDatabase.begin(); it != recordsDatabase.end(); ++it){
	std::string serostatus = ((*it).second.seroStatusAtVaccination == true) ? "POSITIVE" : "NEGATIVE";
	int lastDay = (*it).second.dropoutDay > -1 ? (*it).second.dropoutDay : currDay;
	outSurveillance << (*it).second.houseID.c_str() << "_" << (*it).second.houseMemNum << ", " << (*it).second.ageDaysAtVaccination / 365 << ", " << (*it).second.trialArm.c_str() << ", " << serostatus.c_str() << ", ";
	outSurveillance << (*it).second.enrollmentDay << ", " << lastDay << ", ";
	for(int i = 0; i < 4; i++){
	    std::string sympt = "NO";
	    std::string hosp = "NA";
	    std::string onset = "NA";
	    if((*it).second.symptoms[i] == 1){
		sympt =  "symptomatic";
		onset = std::to_string((*it).second.onset[i]); 
		hosp = ((*it).second.hosp[i] == 1) ? "hospitalized" : "mild";
	    }
	    std::string preExposure = (*it).second.previousExposure[i] == true ? "POSITIVE" : "NEGATIVE";
	    std::string ttr = (*it).second.rightCensored[i] >= 0 ? std::to_string((*it).second.rightCensored[i]) : "NA";
	    outSurveillance << preExposure << ", " << onset << ", " << sympt << ", " << hosp << ", " << (*it).second.pcr[i].c_str() << ", " << (*it).second.primary[i] << ", " << (*it).second.leftCensored[i] << ", " << ttr;
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
