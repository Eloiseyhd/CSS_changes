#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Surveillance.h"
#include "Recruitment.h"

using namespace std;


Recruitment::Recruitment(){
    vaccineSampleSize = 0;
    placeboSampleSize = 0;
    recruitmentTimeFrame = 0;
    dailyVaccineRecruitmentRate = 0;
    dailyPlaceboRecruitmentRate = 0;
    recruitmentStartDay = 0;

    recruitmentStrategy = "none";
    ageGroups.clear();

    vaccineProfile = 0;
    placeboProfile = 0;
}

void Recruitment::update(int currDay, RandomNumGenerator * rGen){
    if(currDay >= recruitmentStartDay){
	if(currDay < recruitmentStartDay + recruitmentTimeFrame){
	    this->enrollTodayParticipants(currDay, rGen);
	}
	if(currDay > recruitmentStartDay){
	    //update enrolled participants activities
	    this->updateParticipants(currDay, rGen);
	}
    }else if(currDay == recruitmentStartDay + recruitmentTimeFrame){
	// Check that everything went smoothly
	for(int i = 0; i < ageGroups.size(); i ++){
	    if(ageGroups[i].vaccine.size() != vaccineSampleSize || ageGroups[i].placebo.size() != placeboSampleSize ){
		printf("There's something strange. Sample size doesn't match actual enrolled size\n");
		printf("Take a look at the size  age group %d of vaccine %lu and placebo %lu\n", i, ageGroups[i].vaccine.size(),ageGroups[i].placebo.size());
		exit(1);
	    }else{
		printf("Final size  age group %d of vaccine %lu and placebo %lu\n", i, ageGroups[i].vaccine.size(),ageGroups[i].placebo.size());
		ageGroups[i].eligible.clear();
	    }
	}
    }
}
void Recruitment::finalizeTrial(int currDay){
    trialSurveillance.printRecords("trialRecords.csv", currDay);
}
void Recruitment::updateParticipants(int currDay, RandomNumGenerator * rGen){
    // Those who drop-out
    printf("UpdateParticipants day %d\n", currDay);

    if(currDay > recruitmentStartDay + recruitmentTimeFrame){
	for(int i = 0; i < ageGroups.size(); i++){
	    if(rGen->getEventProbability() < ageGroups[i].dropoutRate){
		if(rGen->getEventProbability() < 0.5){
		    int pos_ = rGen->getRandomNum(ageGroups[i].vaccine.size());
		    trialSurveillance.finalize_human_surveillance(ageGroups[i].vaccine[pos_], currDay);
		    ageGroups[i].vaccine.erase(ageGroups[i].vaccine.begin() + pos_);
		}else{
		    int pos_ = rGen->getRandomNum(ageGroups[i].placebo.size());
		    trialSurveillance.finalize_human_surveillance(ageGroups[i].vaccine[pos_], currDay);
		    ageGroups[i].placebo.erase(ageGroups[i].placebo.begin() + pos_);		
		}
	    }
	}
    }
    // update doses if needed, remove dead people  and test for denv: self-reported and calls
    for(int i = 0;i < ageGroups.size(); i++){
	if(vaccinesPtr->at(vaccineProfile).doses > 1){
	    std::vector<Human *>::iterator it;
	    for(it = ageGroups[i].vaccine.begin(); it != ageGroups[i].vaccine.end(); ){
		if((*it) != NULL){
		    if((*it)->isEnrolledInTrial() == true){
			trialSurveillance.update_human_surveillance((*it),currDay, rGen);
			if((*it)->isFullyVaccinated() == false && (*it)->getNextDoseDay() == currDay){
			    (*it)->boostVaccine(currDay, rGen);
			}
			++it;
		    }else{
			ageGroups[i].vaccine.erase(it);
		    }
		}else{
		    ++it;
		}
	    }
	}
	if(vaccinesPtr->at(placeboProfile).doses > 1){
	    std::vector<Human *>::iterator it;
	    for(it = ageGroups[i].placebo.begin(); it != ageGroups[i].placebo.end();){
		if((*it) != NULL){
		    if((*it)->isEnrolledInTrial() == true){
			trialSurveillance.update_human_surveillance((*it), currDay, rGen);
			if((*it)->isFullyVaccinated() == false && (*it)->getNextDoseDay() == currDay){
			    (*it)->boostVaccine(currDay, rGen);
			}
			++it;
		    }else{
			ageGroups[i].placebo.erase(it);
		    }
		}else{
		    ++it;
		}
	    }
	}
    }

}


void Recruitment::enrollTodayParticipants(int currDay, RandomNumGenerator * rGen){
    if(dailyVaccineRecruitmentRate <= 0 || dailyPlaceboRecruitmentRate <= 0){
	printf("Daily recruitment rate <= 0\n");
	exit(1);
    }
    // First Vaccine and then placebo, it doesn't really matter which goes when since the vector is shuffled
    for(int i = 0; i < ageGroups.size(); i ++){
	// Should I include the surveillance class as an object of this class???
	int j = 0;
	//Vaccine enrollment
	while(!ageGroups[i].eligible.empty() && j < dailyVaccineRecruitmentRate && ageGroups[i].vaccine.size() < vaccineSampleSize){
	    double temp_age = (double) ageGroups[i].eligible.back()->getAgeDays(currDay) / 365.0;
	    if(temp_age >= ageGroups[i].max){
		ageGroups[i].eligible.pop_back();
	    }else if(temp_age < ageGroups[i].min){
		// if the person is younger than the age group, then move it to a random location in the vector
		// so that that person can have another opportunity later
		printf("Person: %s-%d in position %lu to be moved to another position\n", ageGroups[i].eligible.back()->getHouseID().c_str(),ageGroups[i].eligible.back()->getHouseMemNum(), ageGroups[i].eligible.size());
		Human * temp_h = ageGroups[i].eligible.back();
		ageGroups[i].eligible.pop_back();
		std::vector<Human *>::iterator it = ageGroups[i].eligible.begin();
		long unsigned pos = rGen->getRandomNum(ageGroups[i].eligible.size());
		ageGroups[i].eligible.insert(it+pos,temp_h);
		printf("Person: %s-%d moved to position %lu\n",ageGroups[i].eligible[pos]->getHouseID().c_str(),ageGroups[i].eligible[pos]->getHouseMemNum(),pos);
	    }else{
		if(ageGroups[i].eligible.back()->isEnrolledInTrial() == false){
		    ageGroups[i].eligible.back()->enrollInTrial(currDay, "vaccine");
		    ageGroups[i].vaccine.push_back(ageGroups[i].eligible.back());
		    ageGroups[i].vaccine.back()->vaccinateWithProfile(currDay, rGen, &(vaccinesPtr->at(vaccineProfile)));
		    trialSurveillance.initialize_human_surveillance(ageGroups[i].vaccine.back(), currDay);
		    j++;
		}
		ageGroups[i].eligible.pop_back();
	    }
	}
	j = 0;
	//Placebo enrollment
	while(!ageGroups[i].eligible.empty() && j < dailyPlaceboRecruitmentRate && ageGroups[i].placebo.size() < placeboSampleSize){
	    double temp_age = (double) ageGroups[i].eligible.back()->getAgeDays(currDay) / 365.0;
	    if(temp_age >= ageGroups[i].max){
		ageGroups[i].eligible.pop_back();
	    }else if(temp_age < ageGroups[i].min){
		printf("Placebo Person: %s-%d in position %lu to be moved to another position\n", ageGroups[i].eligible.back()->getHouseID().c_str(),ageGroups[i].eligible.back()->getHouseMemNum(), ageGroups[i].eligible.size());
		Human * temp_h = ageGroups[i].eligible.back();
		ageGroups[i].eligible.pop_back();
		std::vector<Human *>::iterator it = ageGroups[i].eligible.begin();
		long unsigned pos = rGen->getRandomNum(ageGroups[i].eligible.size());
		ageGroups[i].eligible.insert(it+pos,temp_h);
		printf("Placebo Person: %s-%d moved to position %lu\n",ageGroups[i].eligible[pos]->getHouseID().c_str(),ageGroups[i].eligible[pos]->getHouseMemNum(),pos);
	    }else{
		if(ageGroups[i].eligible.back()->isEnrolledInTrial() == false){
		    ageGroups[i].eligible.back()->enrollInTrial(currDay, "placebo");
		    ageGroups[i].placebo.push_back(ageGroups[i].eligible.back());
		    ageGroups[i].placebo.back()->vaccinateWithProfile(currDay, rGen, &(vaccinesPtr->at(placeboProfile)));
		    trialSurveillance.initialize_human_surveillance(ageGroups[i].placebo.back(), currDay);
		    j++;
		}
		ageGroups[i].eligible.pop_back();
	    }
	}
    }
}


void Recruitment::setupRecruitment(std::string file, std::map<unsigned,vProfile> * vaccines_){
    // Initialize vaccines profiles
    vaccinesPtr = vaccines_;

    //Setup the surveillance system for the trial
    trialSurveillance.setup(file);

    if (file.length() == 0) {
		exit(1);
    }

    string line;
    ifstream infile(file);

    if(!infile.good()){
		exit(1);
    }
    std::vector<double> dropoutTemp;
    dropoutTemp.clear();
    // Read the trial recruitment parameters
    while(getline(infile,line,'\n')){
	string line2,line3;
	std::vector<std::string>param_line = getParamsLine(line);
	line2 = param_line[0];
	line3 = param_line[1];
	//	printf("Line2: -%s- Line3: -%s-\n",line2.c_str(),line3.c_str());
	if(line2 == "trial_recruitment_strategy"){
	    recruitmentStrategy = this->parseString(line3);
	}
	if(line2 == "trial_recruitment_timeframe"){
	    recruitmentTimeFrame = this->parseInteger(line3);
	}
	if(line2 == "trial_recruitment_start_day"){
	    recruitmentStartDay = this->parseInteger(line3);
	}
	if(line2 == "trial_vaccine_sample_size"){
	    vaccineSampleSize = this->parseInteger(line3);
	}
	if(line2 == "trial_placebo_sample_size"){
	    placeboSampleSize = this->parseInteger(line3);
	}
	if(line2 == "trial_age_groups"){
	    this->parseAges(line3,&ageGroups);
	}
	if(line2 == "trial_vaccine_profile"){
	    vaccineProfile = this->parseInteger(line3);
	}
	if(line2 == "trial_placebo_profile"){
	    placeboProfile = this->parseInteger(line3);
	}
	if(line2 == "trial_daily_dropout_rate"){
	    parseVector(line3, &dropoutTemp);
	}
    }

    for(int i = 0;i < ageGroups.size();i++){
	if(i < dropoutTemp.size()){
	    if(dropoutTemp[i] > 0.0){
		ageGroups[i].dropoutRate = dropoutTemp[i];
	    }
	}
	printf("Group %d %d_%d. Dropout rate: %.6f\n",i,ageGroups[i].min,ageGroups[i].max, ageGroups[i].dropoutRate);
    }
    printf("Recruitment strategy: ---%s---\n",recruitmentStrategy.c_str());
    printf("Vaccine Profile ID: %d, placebo profile ID: %d\n",vaccineProfile, placeboProfile);
    if(recruitmentStrategy == "none"){
	printf("Please specify a recruitment Strategy\n");
	exit(1);
    }
    infile.close();
}

std::vector<std::string> Recruitment::getParamsLine(std::string line_){
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

void Recruitment::addPossibleParticipant(Human * h,int currDay){
    // First verify that the age is in some group
    int group_ = getPossibleAgeGroup(h->getAgeDays(currDay),ageGroups,recruitmentTimeFrame);
    if(group_ < 0){
	return;
    } 
    // If it's random recruitment just push it into the list without any other requirement
    if(recruitmentStrategy == "random"){
	ageGroups[group_].eligible.push_back(h);
    }
}

void Recruitment::shuffleEligibleParticipants(){
    for(int i = 0;i < ageGroups.size(); i++){
	if(ageGroups[i].eligible.size() >= vaccineSampleSize + placeboSampleSize){
	    std::random_shuffle(ageGroups[i].eligible.begin(),ageGroups[i].eligible.end());
	}else{
	    printf("%lu is less than the sample size sum %d for age group %d\n",ageGroups[i].eligible.size(), vaccineSampleSize + placeboSampleSize, i);
	    exit(1);
	}
    }
    dailyVaccineRecruitmentRate = ceil( (double) vaccineSampleSize / (double) recruitmentTimeFrame);
    dailyPlaceboRecruitmentRate = ceil( (double) placeboSampleSize / (double) recruitmentTimeFrame);
}

int Recruitment::getAgeGroup(int age_, std::vector<groupStruct> groups_temp){
    std::vector<groupStruct>::iterator itAge = groups_temp.begin();
    int count = 0;
    for(; itAge != groups_temp.end(); itAge++){
	if((double )age_ / 365.0 >= (*itAge).min && (double) age_ / 365.0 < (*itAge).max){
	    return count;
	}
	count++;
    }
    return -1;
}

int Recruitment::getPossibleAgeGroup(int age_, std::vector<groupStruct> groups_temp, int time_temp){
    if(time_temp < 0){
	time_temp = 0;
    }
    std::vector<groupStruct>::iterator itAge = groups_temp.begin();
    int count = 0;
    for(; itAge != groups_temp.end(); itAge++){
	if((double ) (age_ + time_temp) / 365.0 >= (*itAge).min && (double) age_ / 365.0 < (*itAge).max){
	    return count;
	}
	count++;
    }
    return -1;
}

long int Recruitment::getEligibleParticipantsSize(){
    long int s = 0;
    for(int i = 0; i < ageGroups.size(); i++ ){
	s += ageGroups[i].eligible.size();
    }
    printf("Size: %lu\n",s);
    return s;
}

int Recruitment::parseInteger(std::string line){
    return strtol(line.c_str(), NULL, 10);
}

double Recruitment::parseDouble(std::string line){
    return strtod(line.c_str(), NULL);
}

std::string Recruitment::parseString(std::string line){
    size_t first_ = line.find_first_not_of(' ');
    size_t last_ = line.find_last_not_of(' ');
    return line.substr(first_,(last_ - first_ + 1));
}

void Recruitment::parseAges(std::string line, std::vector<groupStruct> * ages_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    ages_temp->clear();
    while(getline(linetemp,line2,';')){
		stringstream lTemp; lTemp << line2;
		string line3;
		groupStruct rangeTemp;
		rangeTemp.vaccine.clear();
		rangeTemp.placebo.clear();
		rangeTemp.eligible.clear();
		rangeTemp.dropoutRate = 0.0;
		getline(lTemp,line3,',');
		rangeTemp.min = strtol(line3.c_str(), NULL, 10);
		getline(lTemp,line3,',');
		rangeTemp.max = strtol(line3.c_str(), NULL, 10);
		if(rangeTemp.min + rangeTemp.max > 0){
		    ages_temp->push_back(rangeTemp);
		}
    }

    if(ages_temp->empty()){
		exit(1);
    }
}
void Recruitment::parseVector(std::string line, std::vector<double> * vector_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    vector_temp->clear();

    while(getline(linetemp,line2,',')){
	double temp = strtod(line2.c_str(), NULL);
	if(temp >= 0){
	    vector_temp->push_back(temp);
	}
    }

    if(vector_temp->empty()){
	printf("Vector_temp is empty\n");
		exit(1);
    }
}

Recruitment::~Recruitment() {

}

