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
#include "Human.h"

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

void Recruitment::update(int currDay){
    printf("Updating trial day %d\n", currDay);
    if(currDay >= recruitmentStartDay){
	if(currDay < recruitmentStartDay + recruitmentTimeFrame){
	    this->enrollTodayParticipants(currDay);
	}
	if(currDay > recruitmentStartDay && currDay <= ( recruitmentStartDay + recruitmentTimeFrame + trialDurationDays)){
	    this->updateParticipants(currDay);
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
    // erase(tmpit) in updateArm accomplishes the same thing??
    // this linear scan is really expensive
    /*
    string arm_ = h->getTrialArm();
    vector<Human *> * arm_v;
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
    */
}

void Recruitment::finalizeTrial(int currDay){
    trialSurveillance.printRecords(outSurveillance, currDay);
}

void Recruitment::updateParticipants(int currDay){
    // update doses if needed, remove dead people  and test for denv: self-reported and calls
    for(int i = 0;i < ageGroups.size(); i++){
	updateArm(vaccineProfile, ageGroups[i].vaccine, currDay);
	updateArm(placeboProfile, ageGroups[i].placebo, currDay);
    }
}

void Recruitment::updateArm(unsigned vaxID, recruit_t & arm, int currDay){
    // boost vaccine, decide if dropout, remove death people from the list
    printf("Update Arm\n");
    if(vaccinesPtr.at(vaxID).getDoses() > 1){
        // added redundant continue statements 
        // to ensure that "it" only changes (delete or increment) once per for() 
	for(auto it = arm.begin(); it != arm.end(); ){
            // readability, reset after each continue
            Human * phum = (*it);
	    if(phum == nullptr){
		printf("We found a null pointer\n");
		it = arm.erase(it);
                continue;
	    } else {
		if(phum->isEnrolledInTrial() == true){
		    if(currDay < phum->getTrialEnrollmentDay() + trialDurationDays){
			if(currDay > recruitmentStartDay + recruitmentTimeFrame && rGen->getEventProbability() < dropoutRate){
			    removeParticipant(phum,currDay);
			    it = arm.erase(it);
                            continue;
			}else{
			    int pcr = trialSurveillance.update_human_surveillance(phum, currDay, rGen);
			    if(pcr >= 0){
                                printf("PERSON %s removed from trial for real\n", phum->getPersonID().c_str());
				removeParticipant(phum,currDay);
				it = arm.erase(it);
                                continue;
			    }else{
				if(phum->isFullyVaccinated() == false && phum->getNextDoseDay() == currDay){
				    printf("human boosting %s\n", phum->getPersonID().c_str());
				    phum->boostVaccine(currDay, rGen);
				}
				it++;
                                continue;
			    }
			}
		    }else{
			printf("Participant removed because time is over\n");
			removeParticipant(phum,currDay);
			it = arm.erase(it);
                        continue;
		    }
		}else{
		    printf("Participant is not really enrolled in trial\n");
		    it = arm.erase(it);
                    continue;
		}
            }
	}
    }
    printf("Arm updated\n");
}

void Recruitment::enrollTodayParticipants(int currDay){
    if(dailyVaccineRecruitmentRate <= 0 || dailyPlaceboRecruitmentRate <= 0){
	printf("Daily recruitment rate <= 0\n");
	exit(1);
    }
    for(int i = 0; i < ageGroups.size(); i ++){
	printf("Enroll participants day %d group %d, (eligible %zu).\n", currDay, i, ageGroups[i].eligible.size() );
	//Vaccine enrollment
	enrollArmParticipants(ageGroups[i].vaccine, ageGroups[i].eligible, "vaccine", currDay,vaccineSampleSize,dailyVaccineRecruitmentRate,ageGroups[i].min, ageGroups[i].max,vaccineProfile);
	enrollArmParticipants(ageGroups[i].placebo, ageGroups[i].eligible, "placebo", currDay,placeboSampleSize,dailyPlaceboRecruitmentRate,ageGroups[i].min, ageGroups[i].max,placeboProfile);
	printf("%zu vaccine and %zu placebo participants successfully enrolled at day %d\n", ageGroups[i].vaccine.size(), ageGroups[i].placebo.size(), currDay);
    }
}

void Recruitment::enrollArmParticipants(
					recruit_t & arm,
					eligible_t & eligible,
					string arm_str, int currDay,
					int sample_size,
					int rec_rate,
					int min_,
					int max_,
					unsigned vProfile)
{
    printf("Enroll participants today %d %s, (eligible %zu)\n", currDay, arm_str.c_str(), eligible.size());
    int nrecruit = 0;
    // process from end, possibly deleting as we go
    auto it = eligible.end();
    while(it >= eligible.begin() && nrecruit < rec_rate && arm.size() < sample_size){
        // back up from end
        it--;
        // convenience / clarity var
        // reset after each continue
        Human * phum = (*it);
        //
	if(phum == nullptr){
	    printf("Found a dead participant in enrollment day %d\n", currDay);
        // remove and restart loop
	    it = eligible.erase(it);
	    continue;
	}
	double temp_age = (double) phum->getAgeDays(currDay) / 365.0;
	if(temp_age >= max_){
        // remove and restart loop
	    it = eligible.erase(it);
	    continue;
	} else if(temp_age < min_){
        // skip, reshuffle at end?
	    continue;
            /*
	    Human * temp_h = eligible_vector->back();
	    eligible_vector->pop_back();
	    vector<Human *>::iterator it = eligible_vector->begin();
	    long unsigned pos = rGen->getRandomNum(eligible_vector->size());
	    eligible_vector->insert(it+pos,temp_h);
            */
	}else{
	    if(phum->isEnrolledInTrial() == false){
                // should phum be removed from eligible??
		phum->enrollInTrial(currDay, arm_str);
		arm.insert(phum);
		trialSurveillance.initialize_human_surveillance(phum, currDay);
		phum->vaccinateWithProfile(currDay, rGen, vaccinesPtr.at(vProfile));
		nrecruit++;
	    }
        continue; // added for clarity
	}
    }
    // reshuffle at the end of each enrollment
    shuffleEligibleParticipants();
}

void Recruitment::setupRecruitment(string file, map<unsigned,Vaccine> vaccines_, string outputPath, string simName_, RandomNumGenerator * _rGen){
    // store reference to simulator-wide rng
    rGen = _rGen;
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
    std::ifstream infile(file);

    if(!infile.good()){
		exit(1);
    }
    printf("Reading setuprecruitment file %s\n", file.c_str());
    // Read the trial recruitment parameters
    while(getline(infile,line,'\n')){
	string line2,line3;
	vector<string>param_line = getParamsLine(line);
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

vector<string> Recruitment::getParamsLine(string line_){
    std::stringstream linetemp;
    string line2_,line3_;
    linetemp.clear();
    linetemp << line_;
    getline(linetemp,line2_,'=');
    getline(linetemp,line3_,'=');
    linetemp.clear();
    linetemp << line2_;
    getline(linetemp,line2_,' ');
    vector<string> params;
    params.push_back(line2_);
    params.push_back(line3_);
    return params;
}

void Recruitment::addPossibleParticipant(Human * h, int currDay){
    // First verify that the age is in some group
    int group_ = getPossibleAgeGroup(h->getAgeDays(currDay),ageGroups,recruitmentTimeFrame);
    if(group_ < 0){
	return;
    } 
    // If it's random recruitment just push it into the list without any other requirement
    // otherwise??
    if(recruitmentStrategy == "random"){
	ageGroups[group_].eligible.push_back(h);
    }
}

void Recruitment::shuffleEligibleParticipants(){
    // counter
    int ii{};
    // process eligibles for each group
    for(auto & grp : ageGroups ){
        ii++;
	if(grp.eligible.size() >= vaccineSampleSize + placeboSampleSize){
	    // First make sure eligible participants have not died
	    for(auto it = grp.eligible.begin(); it !=  grp.eligible.end();){
		if( (*it) == nullptr){
		    printf("NULL pointer in shuffle eligible participants\n");
                    // erase and advance (vector safe)
		    it = grp.eligible.erase(it);
                    continue; // redund
		}else{
                    // skip
		    it++;
                    continue; // redund
		}
	    }
	    rGen->shuffle(grp.eligible);
	}else{
	    printf("%lu is less than the sample size sum %d for age group %d\n",grp.eligible.size(), vaccineSampleSize + placeboSampleSize, ii-1);
	    exit(1);
	}
    }
    dailyVaccineRecruitmentRate = ceil( (double) vaccineSampleSize / (double) recruitmentTimeFrame);
    dailyPlaceboRecruitmentRate = ceil( (double) placeboSampleSize / (double) recruitmentTimeFrame);
    printf("Shuffle participants finished\n");
}

int Recruitment::getAgeGroup(int age_, vector<groupStruct> groups_temp){
    vector<groupStruct>::iterator itAge = groups_temp.begin();
    int count = 0;
    for(; itAge != groups_temp.end(); itAge++){
	if((double )age_ / 365.0 >= (*itAge).min && (double) age_ / 365.0 < (*itAge).max){
	    return count;
	}
	count++;
    }
    return -1;
}

int Recruitment::getPossibleAgeGroup(int age_, vector<groupStruct> groups_temp, int time_temp){
    if(time_temp < 0){
	time_temp = 0;
    }
    vector<groupStruct>::iterator itAge = groups_temp.begin();
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

int Recruitment::parseInteger(string line){
    return strtol(line.c_str(), NULL, 10);
}

double Recruitment::parseDouble(string line){
    return strtod(line.c_str(), NULL);
}

string Recruitment::parseString(string line){
    size_t first_ = line.find_first_not_of(' ');
    size_t last_ = line.find_last_not_of(' ');
    return line.substr(first_,(last_ - first_ + 1));
}

void Recruitment::parseAges(string line, vector<groupStruct> * ages_temp){
    std::stringstream linetemp;
    string line2;
    linetemp.clear();
    linetemp << line;
    ages_temp->clear();
    while(getline(linetemp,line2,';')){
		std::stringstream lTemp; lTemp << line2;
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
void Recruitment::parseVector(string line, vector<double> * vector_temp){
    std::stringstream linetemp;
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

//Recruitment::~Recruitment() {
//}

