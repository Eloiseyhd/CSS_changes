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
    trialDurationDays = 0;
    dropoutRate = 0.0;

    recruitmentStrategy = "none";
    ageGroups.clear();

    vaccineProfile = 0;
    placeboProfile = 0;
}

void Recruitment::update(int currDay, RandomNumGenerator * rGen){
    printf("Updating trial day %d\n", currDay);
    if(currDay >= recruitmentStartDay){
	if(currDay < recruitmentStartDay + recruitmentTimeFrame){
	    this->enrollTodayParticipants(currDay, rGen);
	}
	if(currDay > recruitmentStartDay && currDay <= ( recruitmentStartDay + recruitmentTimeFrame + trialDurationDays)){
	    this->updateParticipants(currDay, rGen);
	}
    }else if(currDay == recruitmentStartDay + recruitmentTimeFrame){
	for(int i = 0; i < ageGroups.size(); i ++){
	    if(ageGroups[i].vaccine.size() != vaccineSampleSize || ageGroups[i].placebo.size() != placeboSampleSize ){
		printf("Take a look at the size  age group %d of vaccine %lu and placebo %lu\n", i, ageGroups[i].vaccine.size(),ageGroups[i].placebo.size());
		exit(1);
	    }else{
		ageGroups[i].eligible.clear();
	    }
	}
    }
    printf("Updating finished day %d\n", currDay);
}

void Recruitment::removeParticipant(Human * h, int currDay){
    trialSurveillance.finalize_human_surveillance(h, currDay);
    h->unenrollTrial();
    std::string arm_ = h->getTrialArm();
    std::vector<Human *> * arm_v;
    int i = getAgeGroup(h->getAgeTrialEnrollment(),ageGroups);
    if(i >=0){
	if(arm_ == "vaccine"){
	    arm_v = &ageGroups[i].vaccine;
	}else{
	    arm_v = &ageGroups[i].placebo;
	}
	for(auto it = arm_v->begin(); it !=  arm_v->end();){
	    if((*it)->getPersonID() == h->getPersonID()){
		printf("PERSON %s removed from trial for real\n", h->getPersonID().c_str());
		arm_v->erase(it);
		break;
	    }else{
		++it;
	    }
	}
    }else{
	printf("SOMETHING IS VERY STRANGE in removeparticipant\n");
	exit(1);
    }

    
}

void Recruitment::finalizeTrial(int currDay){
    trialSurveillance.printRecords(outSurveillance, currDay);
}

void Recruitment::updateParticipants(int currDay, RandomNumGenerator * rGen){
    // update doses if needed, remove dead people  and test for denv: self-reported and calls
    for(int i = 0;i < ageGroups.size(); i++){
	updateArm(vaccineProfile,&ageGroups[i].vaccine, currDay, rGen);
	updateArm(placeboProfile,&ageGroups[i].placebo, currDay, rGen);
    }
}

void Recruitment::updateArm(unsigned vaxID, std::vector<Human *> * arm, int currDay, RandomNumGenerator * rGen){
    // boost vaccine, decide if dropout, remove death people from the list
    printf("Update Arm\n");
    if(vaccinesPtr.at(vaxID).getDoses() > 1){
	std::vector<Human *>::iterator it;
	for(it = arm->begin(); it != arm->end(); ){
	    if((*it) != NULL){
		if((*it)->isEnrolledInTrial() == true){
		    if(currDay < (*it)->getTrialEnrollmentDay() + trialDurationDays){
			if(currDay > recruitmentStartDay + recruitmentTimeFrame && rGen->getEventProbability() < dropoutRate){
			    removeParticipant(*it,currDay);
			    auto tmpit = it;
			    ++it;
			    arm->erase(tmpit);
			}else{
			    int pcr = trialSurveillance.update_human_surveillance((*it),currDay, rGen);
			    if(pcr >= 0){
				removeParticipant(*it,currDay);
				auto tmpit = it;
				++it;
				arm->erase(tmpit);
			    }else{
				if((*it)->isFullyVaccinated() == false && (*it)->getNextDoseDay() == currDay){
				    printf("human boosting %s\n", (*it)->getPersonID().c_str());
				    (*it)->boostVaccine(currDay, rGen);
				}
				++it;
			    }
			}
		    }else{
			printf("Participant removed because time is over\n");
			removeParticipant(*it,currDay);
			auto tmpit = it;
			++it;
			arm->erase(tmpit);
		    }
		}else{
		    printf("Participant is not really enrolled in trial\n");
		    auto tmpit = it;
		    ++it;
		    arm->erase(tmpit);
		}
	    }else{
		printf("We found a null pointer\n");
		auto tmpit = it;
		++it;
		arm->erase(tmpit);
	    }
	}
    }
    printf("Arm updated\n");
}

void Recruitment::enrollTodayParticipants(int currDay, RandomNumGenerator * rGen){
    if(dailyVaccineRecruitmentRate <= 0 || dailyPlaceboRecruitmentRate <= 0){
	printf("Daily recruitment rate <= 0\n");
	exit(1);
    }
    for(int i = 0; i < ageGroups.size(); i ++){
	printf("Enroll participants day %d group %d, (%d eligible).\n", currDay, i, ageGroups[i].eligible.size() );
	//Vaccine enrollment
	enrollArmParticipants(&ageGroups[i].vaccine, &ageGroups[i].eligible, "vaccine", currDay,vaccineSampleSize,dailyVaccineRecruitmentRate,ageGroups[i].min, ageGroups[i].max,vaccineProfile, rGen);
	enrollArmParticipants(&ageGroups[i].placebo, &ageGroups[i].eligible, "placebo", currDay,placeboSampleSize,dailyPlaceboRecruitmentRate,ageGroups[i].min, ageGroups[i].max,placeboProfile, rGen);
	printf("%d vaccine and %d placebo participants successfully enrolled at day %d\n", ageGroups[i].vaccine.size(), ageGroups[i].placebo.size(), currDay);
    }
}

void Recruitment::enrollArmParticipants(
					std::vector<Human *> * arm,
					std::vector<Human *> * eligible_vector,
					std::string arm_str, int currDay,
					int sample_size,
					int rec_rate,
					int min_,
					int max_,
					unsigned vProfile,
					RandomNumGenerator * rGen)
{
    printf("Enroll participants today %d %s, (eligible %d)\n", currDay, arm_str.c_str(), eligible_vector->size());
    int j = 0;
    while(!eligible_vector->empty() && j < rec_rate && arm->size() < sample_size){
	if(eligible_vector->back() == nullptr){
	    printf("Found a dead participant in enrollment day %d\n", currDay);
	    eligible_vector->pop_back();
	    continue;
	}
	double temp_age = (double) eligible_vector->back()->getAgeDays(currDay) / 365.0;
	if(temp_age >= max_){
	    eligible_vector->pop_back();
	}else if(temp_age < min_){
	    Human * temp_h = eligible_vector->back();
	    eligible_vector->pop_back();
	    std::vector<Human *>::iterator it = eligible_vector->begin();
	    long unsigned pos = rGen->getRandomNum(eligible_vector->size());
	    eligible_vector->insert(it+pos,temp_h);
	}else{
	    if(eligible_vector->back()->isEnrolledInTrial() == false){
		eligible_vector->back()->enrollInTrial(currDay, arm_str);
		arm->push_back(eligible_vector->back());
		trialSurveillance.initialize_human_surveillance(arm->back(), currDay);
		arm->back()->vaccinateWithProfile(currDay, rGen, vaccinesPtr.at(vProfile));		   
		j++;
	    }
	    eligible_vector->pop_back();
	}
    }
}

void Recruitment::setupRecruitment(std::string file, std::map<unsigned,Vaccine> vaccines_, std::string outputPath, std::string simName_){
    // Initialize vaccines profiles
    vaccinesPtr = vaccines_;
    if(vaccinesPtr.size() == 0){
	printf("Recruitment::setupRecruitment there are no vaccines to copy\n");
	exit(1);
    }
    //Setup the surveillance system for the trial
    trialSurveillance.setup(file);

    if (file.length() == 0) {
		exit(1);
    }
    outSurveillance = outputPath + "/" + simName_ + "_trial.csv";

    string line;
    ifstream infile(file);

    if(!infile.good()){
		exit(1);
    }
    printf("Reading setuprecruitment file %s\n", file.c_str());
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
	if(line2 == "trial_length_days"){
	    trialDurationDays = this->parseInteger(line3);
	}
	if(line2 == "trial_avg_enrollment_days"){
	    dropoutRate = (double) 1.0 / (this->parseDouble(line3));
	}
    }


    printf("Recruitment strategy: |%s| duration %d dropoutRate %.6f\n",recruitmentStrategy.c_str(), trialDurationDays, dropoutRate);
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

void Recruitment::shuffleEligibleParticipants(RandomNumGenerator & refGen){
    for(int i = 0;i < ageGroups.size(); i++){
	if(ageGroups[i].eligible.size() >= vaccineSampleSize + placeboSampleSize){
	    // First make sure eligible participants have not died
	    for(auto it = ageGroups[i].eligible.begin(); it !=  ageGroups[i].eligible.end();){
		if((*it) == NULL){
		    printf("NULL pointer in shuffle eligible participants\n");
		    auto tmpit = it;
		    ++it;
		    ageGroups[i].eligible.erase(tmpit);
		}else{
		    ++it;
		}
	    }
	    refGen.shuffle(ageGroups[i].eligible);
	}else{
	    printf("%lu is less than the sample size sum %d for age group %d\n",ageGroups[i].eligible.size(), vaccineSampleSize + placeboSampleSize, i);
	    exit(1);
	}
    }
    dailyVaccineRecruitmentRate = ceil( (double) vaccineSampleSize / (double) recruitmentTimeFrame);
    dailyPlaceboRecruitmentRate = ceil( (double) placeboSampleSize / (double) recruitmentTimeFrame);
    printf("Shuffle participants finished\n");
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
    linetemp << line;
    vector_temp->clear();
    while(getline(linetemp,line2,',')){
	double temp = strtod(line2.c_str(), NULL);
	if(temp >= 0){
	    vector_temp->push_back(temp);
	}
    }
    if(vector_temp->empty()){
		exit(1);
    }
}

Recruitment::~Recruitment() {

}

