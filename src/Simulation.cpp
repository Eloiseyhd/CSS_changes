#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Simulation.h"

using namespace std;


void Simulation::simulate() {
    simEngine();
    out.close();
    outpop.close();
    outprevac.close();
}



string Simulation::readInputs() {
    readSimControlFile(configLine);

    outputPath.erase(remove(outputPath.begin(), outputPath.end(), '\"'), outputPath.end());
    outputReport.setupReport(reportsFile,outputPath,simName);

    RandomNumGenerator rgen(rSeed, huImm, emergeFactor, mlife, mbite, halflife);
    rGen = rgen;

    RandomNumGenerator rgen2(rSeedInf, huImm, emergeFactor, mlife, mbite, halflife);
    rGenInf = rgen2;

    readDiseaseRatesFile();

    readVaccineSettingsFile();

    readVaccineProfilesFile();

    if(vaccinationStrategy == "random_trial"){
	recruitmentTrial.setupRecruitment(trialSettingsFile, &vaccines, outputPath, simName);
	printf("Vax Sample: %d, Plac Sample: %d\n", recruitmentTrial.getVaccineSampleSize(), recruitmentTrial.getPlaceboSampleSize());
	printf("Recruitment day: %d\n",recruitmentTrial.getRecruitmentStartDay());
    }
    if(vaccinationStrategy == "sanofi_trial"){
	readVaccinationGroupsFile();
    }
    readLocationFile(locationFile);
    readHumanFile(trajectoryFile);
    return simName;
}



void Simulation::simEngine() {
    while(currentDay < numDays){
        for(auto itLoc = locations.begin(); itLoc != locations.end(); itLoc++){
            itLoc->second->updateInfectedVisitor();
        }
	if(vaccinationStrategy == "random_trial"){
	    if(currentDay == recruitmentTrial.getRecruitmentStartDay()){
		printf("Current Day : %d is recruitment Start Day\n",currentDay);
		selectEligibleTrialParticipants();
	    }
	    recruitmentTrial.update(currentDay, &rGenInf);
	}
        humanDynamics();
	outputReport.printReport(currentDay);
        mosquitoDynamics();
	
        if(ceil(double(currentDay + 1) / 365.0) != ceil(double(currentDay) / 365.0)){
            year++;
            updatePop();
        }
        currentDay++;
    }
    outputReport.finalizeReport();
    if(vaccinationStrategy == "random_trial"){
	recruitmentTrial.finalizeTrial(currentDay);
    }
}


void Simulation::updatePop(){
    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
        itHum->second->updateAttractiveness(currentDay);
    }
}


void Simulation::humanDynamics() {
    int diff, age;
    bool vaxd = false;
    int cohort = 0;

    if(currentDay >= vaccineDay){
        cohort = floor(double(currentDay - vaccineDay) / 365.0) + 1;
    }

    for(auto it = humans.begin(); it != humans.end(); ++it){
        // daily mortality for humans by age
        if(rGen.getEventProbability() < (deathRate * it->second->getAgeDays(currentDay))){
            it->second->reincarnate(currentDay);
	    if(vaccinationStrategy == "random_trial"){
		// remove from trial
		recruitmentTrial.removeParticipant((it->second).get(),currentDay);
	    }
	}
	
        // update temporary cross-immunity status if necessary
        if(currentDay == it->second->getImmEndDay())
            it->second->setImmunityTemp(false);

        // update infection status if necessary
        if(it->second->infection != nullptr)
            it->second->checkRecovered(currentDay);

        // select movement trajectory for the day
        (it->second)->setTrajDay(rGen.getRandomNum(5));

        // simulate possible imported infection
        if(rGen.getEventProbability() < ForceOfImportationTrial){
            for(int serotype = 1; serotype <= 4; serotype++){
                if(!it->second->isImmune(serotype)){
                    it->second->infect(currentDay, serotype, &rGenInf, &disRates, &hospRates);                
                }
            }
        }

	//update vaccine immunity if necessary

	if(it->second->isVaccinated()){
	    if(vaccines.at(vaccineID).mode == "advance" && currentDay == it->second->getVaxImmEndDay()){
		it->second->setVaxImmunity(false);
	    }
	}


    	// vaccination, first record the cohorts depending on the vaccination strategy, then vaccinate. 


    	age = it->second->getAgeDays(currentDay);
	if(vaccinationStrategy == "sanofi_trial"){
	    if(currentDay == vaccineDay){
		it->second->setAgeTrialEnrollment(age);
		it->second->setSeroStatusAtVaccination();
		it->second->setCohort(cohort);
		//one-time vaccination by age groups in the trial
    		if(checkAgeToVaccinate(age)){
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
			it->second->vaccinateWithProfile(currentDay, &rGenInf, &(vaccines.at(vaccineID)));
    		    }
    		}
    	    }
    	}else if(vaccinationStrategy == "routine"){
    	    if(currentDay >= vaccineDay){
    		// routine vaccination by age
		if(age == vaccineAge * 365){
		    it->second->setAgeTrialEnrollment(age);
		    it->second->setSeroStatusAtVaccination();
		    it->second->setCohort(cohort);
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
			it->second->vaccinateWithProfile(currentDay, &rGenInf, &(vaccines.at(vaccineID)));
    		    }
    		}
    	    }    
    	}

    	// catchup vaccination
    	if(catchupFlag == true && vaccineDay == currentDay){
    	    if(age > vaccineAge * 365 && age < 18 * 365){
    		if(rGenInf.getEventProbability() < vaccineCoverage){
		    it->second->vaccinateWithProfile(currentDay, &rGenInf, &(vaccines.at(vaccineID)));
    		}
    	    }
    	}
            
	outputReport.updateReport(currentDay,(it->second).get());
           

    }
}


void Simulation::mosquitoDynamics(){
    double biteTime, dieTime;
    bool biteTaken;

    generateMosquitoes();

    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
        if(it->second->infection != nullptr){
            if(currentDay == it->second->infection->getStartDay())
                it->second->infection->setInfectiousnessMosquito(mozInfectiousness);
        }
 
        // determine if the mosquito will bite and/or die today, and if so at what time
        biteTime = double(numDays + 1);
        dieTime = double(numDays + 1); 
        if(it->second->getBiteStartDay() <= double(currentDay + 1)){
            biteTime = it->second->getBiteStartDay() - double(currentDay);
            if(biteTime < 0.0){
                biteTime = rGenInf.getEventProbability();
            }
        }
        if(it->second->getDDay() <= double(currentDay + 1)){
            dieTime = it->second->getDDay() - double(currentDay);
        }

        // if the mosquito dies first, then kill it
        if(dieTime <= biteTime && dieTime <= 1.0){
            auto it_temp = it;
            it++;
            mosquitoes.erase(it_temp);
            continue;
        }

        // if the mosquito bites first, then let it bite and then see about dying
        while(biteTime < dieTime && biteTime <= 1.0){
            biteTaken = it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,&disRates,&hospRates,currentDay,numDays,&out);
            it->second->setBiteStartDay(currentDay + rGen.getMozRestDays());
            biteTime = it->second->getBiteStartDay() - double(currentDay);

            if(!biteTaken){
                string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
                if(newLoc != "TOO_FAR_FROM_ANYWHERE"){
                    it->second->setLocation(newLoc);
                    mosquitoes.insert(make_pair(newLoc, move(it->second)));
                    auto it_temp = it;
                    it++;
                    mosquitoes.erase(it_temp);
                }
            }
        }

        if(dieTime < 1.0){
            auto it_temp = it;
            it++;
            mosquitoes.erase(it_temp);
            continue;
        }

        // let the mosquito move if that happens today 
    	double moveProb = rGen.getEventProbability();
        if(moveProb < mozMoveProbability){
            string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
            if(newLoc != "TOO_FAR_FROM_ANYWHERE"){
                it->second->setLocation(newLoc);
                mosquitoes.insert(make_pair(newLoc, move(it->second)));
                auto it_temp = it;
                it++;
                mosquitoes.erase(it_temp);
            }
            else{
                it++;
            }
        }
        else{
            it++;
        }
    }
}



void Simulation::generateMosquitoes(){
    int mozCount = 0;

    for(auto& x : locations){
        mozCount = rGen.getMozEmerge(x.second->getEmergenceRate());

        for(int i = 0; i < mozCount; i++){
            unique_ptr<Mosquito> moz(new Mosquito(
                double(currentDay) + rGen.getMozLifeSpan(), double(currentDay) + rGen.getMozRestDays(), x.first));
            mosquitoes.insert(make_pair(x.first, move(moz)));
        }
    }
}



void Simulation::setLocNeighborhood(double dist){
    for(auto it1 = locations.begin(); it1 != locations.end(); ++it1){
        auto it2 = it1;
        it2++;
        for(; it2 != locations.end(); ++it2){
            if(it1->second->getDistanceFromLoc(*it2->second) <= dist){
                it1->second->addCloseLoc(it2->first);
                it2->second->addCloseLoc(it1->first);
            }
        }
    }
}

void Simulation::selectEligibleTrialParticipants(){
    for(auto it = humans.begin(); it != humans.end(); ++it){
	recruitmentTrial.addPossibleParticipant((it->second).get(),currentDay);
    }
    printf("In total %lu participants are eligible out of %lu\n",recruitmentTrial.getEligibleParticipantsSize(),humans.size());
    recruitmentTrial.shuffleEligibleParticipants();
}

void Simulation::readSimControlFile(string line) {
    stringstream infile;
    infile << line;
    getline(infile, line, ',');
    simName = line;
    getline(infile, line, ',');
    rSeed = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    rSeedInf = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    numDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccineSettingsFile = line;
    getline(infile, line, ',');
    outputPath = line;
    getline(infile, line, ',');
    reportsFile = line;
    getline(infile, line, ',');
    diseaseRatesFile = line;
    getline(infile, line, ',');
    locationFile = line;
    getline(infile, line, ',');
    trajectoryFile = line;
    getline(infile, line, ',');
    deathRate = strtod(line.c_str(), NULL);    
    getline(infile, line, ',');
    ForceOfImportation = strtod(line.c_str(),NULL);
    getline(infile, line, ',');
    ForceOfImportationTrial = strtod(line.c_str(),NULL);
    getline(infile, line, ',');
    huImm = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    emergeFactor = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mlife = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mozInfectiousness = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mozMoveProbability = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mbite = strtod(line.c_str(), NULL);
}

void Simulation::readLocationFile(string locFile) {
    if (locFile.length() == 0) {
        exit(1);
    }
    string line, locID, locType;
    double x, y, mozzes;

    ifstream infile(locFile);
   if (!infile.good()) {
        exit(1);
    }
    getline(infile, line);
    while (getline(infile, line, ',')) {
        x = strtod(line.c_str(), NULL);

        getline(infile, line, ',');
        y = strtod(line.c_str(), NULL);

        getline(infile, line, ',');
        locType = line;

        getline(infile, line, ',');
        getline(infile, line, ',');
        getline(infile, line, ',');
        getline(infile, line, ',');
        locID = line;

        getline(infile, line);
        mozzes = strtod(line.c_str(), NULL);

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');

        unique_ptr<Location> location(new Location(locID, locType, x, y, 1.0));
        locations.insert(make_pair(locID, move(location)));

    }
    infile.close();
}



void Simulation::readDiseaseRatesFile(){
    if(diseaseRatesFile.length() == 0){
        exit(1);
    }
    string line;
    unsigned par = 0;
    double parDis;
    double parHosp;

    ifstream infile(diseaseRatesFile);
    if(!infile.good()){
        exit(1);
    }
    while(getline(infile, line, ',')){
        parDis = strtod(line.c_str(), NULL);
        getline(infile, line, '\n');
        parHosp = strtod(line.c_str(), NULL);
        if(par <= 2){
            disRates.insert(make_pair(par, parDis));
            hospRates.insert(make_pair(par, parHosp));
        }
        par++;
    }
    infile.close();
}

void Simulation::readVaccineSettingsFile(){
    if(vaccineSettingsFile.length() == 0){
        exit(1);
    }
    string line;
    ifstream infile(vaccineSettingsFile);
    if(!infile.good()){
        exit(1);
    }
    while(getline(infile,line,'\n')){
	string line2,line3;
	std::vector<std::string>param_line = getParamsLine(line);
	line2 = param_line[0];
	line3 = param_line[1];
	if(line2 == "vaccination_strategy"){
	    vaccinationStrategy = this->parseString(line3);
	}
	if(line2 == "vaccine_day"){
	    vaccineDay = this->parseInteger(line3);
	}
	if(line2 == "vaccine_age"){
	    vaccineAge = this->parseInteger(line3);
	}
	if(line2 == "vaccine_coverage"){
	    vaccineCoverage = this->parseDouble(line3);
	}
	if(line2 == "vaccine_catchup"){
	    catchupFlag = this->parseInteger(line3) == 0 ? false : true;
	}
	if(line2 == "vaccine_groups_file"){
	    vaccinationGroupsFile = this->parseString(line3);
	}
	if(line2 == "vaccine_profiles_file"){
	    vaccineProfilesFile = this->parseString(line3);
	}
	if(line2 == "trial_settings_file"){
	    trialSettingsFile = this->parseString(line3);
	}
	if(line2 == "vaccine_ID"){
	    vaccineID = this->parseInteger(line3);
	}
    }
    infile.close();
    printf("vStrategy %s day %d age %d coverage: %.2f: groups_file: %s: profilesFile: %s: ID: %d\n",vaccinationStrategy.c_str(), vaccineDay, vaccineAge,vaccineCoverage, vaccinationGroupsFile.c_str(), vaccineProfilesFile.c_str(), vaccineID);
}


void Simulation::readVaccineProfilesFile(){
    if(vaccineProfilesFile.length() == 0){
        exit(1);
    }
    vaccines.clear();
    string line;
    ifstream infile(vaccineProfilesFile);
    if(!infile.good()){
        exit(1);
    }
    int numVaccines = 0;
    // First find the number of vaccines
    while(getline(infile,line,'\n')){
	string line2,line3;
	std::vector<std::string>param_line = getParamsLine(line);
	line2 = param_line[0];
	line3 = param_line[1];
	if(line2 == "vaccine_ids"){
	    numVaccines = this->parseInteger(line3);
	}
    }

    if(numVaccines > 0){
	printf("There are %d vaccines to read in the file\n",numVaccines);
	// Now we should read the rest of the parameters and store them in the appropiate vaccine structure
	for(unsigned i = 0;i < numVaccines; i++){
	    infile.clear();
	    infile.seekg(0);

	    vProfile vaxTemp;
	    vaxTemp.id = i;
	    vaxTemp.mode = " ";
	    vaxTemp.name = " ";
	    vaxTemp.waning = 0.0;
	    vaxTemp.protection = 0.0;
	    vaxTemp.propInf = 0.0;
	    vaxTemp.normdev = 0.0;
	    for(unsigned k = 0; k < 3; k++){
		vaxTemp.VE_pos.insert(make_pair(k,0.0));
		vaxTemp.VE_neg.insert(make_pair(k,0.0));
	    }
	    vaxTemp.total_VE = 0.0;
	    vaxTemp.doses = 1;
	    vaxTemp.relative_schedule.clear();

	    while(getline(infile,line,'\n')){
		string line2,line3;
		std::vector<std::string>param_line = getParamsLine(line);
		line2 = param_line[0];
		line3 = param_line[1];
		std::string s_id = std::to_string(i);
		if(line2 == "vaccine_mode_" + s_id){
		    vaxTemp.mode = this->parseString(line3);
		}
		if(line2 == "vaccine_name_" + s_id){
		    vaxTemp.name = this->parseString(line3);
		}
		if(line2 == "vaccine_waning_" + s_id){
		    vaxTemp.waning = this->parseDouble(line3);
		}
		if(line2 == "vaccine_protection_" + s_id){
		    vaxTemp.protection = this->parseDouble(line3);
		}
		if(line2 == "vaccine_ve_" + s_id){
		    vaxTemp.total_VE = this->parseDouble(line3);
		}
		if(line2 == "vaccine_vepos_a_" + s_id){
		    vaxTemp.VE_pos[0] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_veneg_a_" + s_id){
		    vaxTemp.VE_neg[0] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_vepos_b_" + s_id){
		    vaxTemp.VE_pos[1] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_veneg_b_" + s_id){
		    vaxTemp.VE_neg[1] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_vepos_c_" + s_id){
		    vaxTemp.VE_pos[2] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_veneg_c_" + s_id){
		    vaxTemp.VE_neg[2] = this->parseDouble(line3);
		}
		if(line2 == "vaccine_prop_inf_" + s_id){
		    vaxTemp.propInf = this->parseDouble(line3);
		}
		if(line2 == "vaccine_normdev_" + s_id){
		    vaxTemp.normdev = this->parseDouble(line3);
		}
		if(line2 == "vaccine_schedule_" + s_id){
		    this->parseVector(line3, &(vaxTemp.relative_schedule));
		}
	    }
	    if(vaxTemp.relative_schedule.size() > 0){
		vaxTemp.doses = vaxTemp.relative_schedule.size();
	    }else{
		vaxTemp.doses = 1;
	    }
	    vaccines.insert(make_pair(i,vaxTemp));
	}
    }else{
	printf("There are no vaccines to read in %s\n",vaccineProfilesFile.c_str());
	exit(1);
    }
    for (int i = 0;i < vaccines.size(); i++){
	printf("Vaccine ID: %u Name: %s Mode: %s Waning %.2f Protection %.2f Efficacy %.2f PropInf %.2f NormDev %.2f Doses %d\n",vaccines.at(i).id, vaccines.at(i).name.c_str(), vaccines.at(i).mode.c_str(), vaccines.at(i).waning,vaccines.at(i).protection,vaccines.at(i).total_VE,
	       vaccines.at(i).propInf, vaccines.at(i).normdev, vaccines.at(i).doses);
	for(int j = 0; j < 3; j++){
	    printf("PAR %d VE pos %.2f VE neg %.2f\n",j,vaccines.at(i).VE_pos.at(j), vaccines.at(i).VE_neg.at(j));
	}
	for(int j = 0; j < vaccines.at(i).relative_schedule.size(); j++){
	    printf("Dose %d Day %d\n", j + 1, vaccines.at(i).relative_schedule[j]);
	}
    }
    infile.close();
}

std::vector<std::string> Simulation::getParamsLine(std::string line_){
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

int Simulation::parseInteger(std::string line){
    return strtol(line.c_str(), NULL, 10);
}

double Simulation::parseDouble(std::string line){
    return strtod(line.c_str(), NULL);
}

void Simulation::parseVector(std::string line, std::vector<int> * vector_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    vector_temp->clear();

    while(getline(linetemp,line2,',')){
	int temp = strtol(line2.c_str(), NULL, 10);
	if(temp >= 0){
	    vector_temp->push_back(temp);
	}
    }

    if(vector_temp->empty()){
	printf("Vector_temp is empty\n");
		exit(1);
    }
}
std::string Simulation::parseString(std::string line){
    size_t first_ = line.find_first_not_of(" \t#");
    size_t last_ = line.find_last_not_of(" \t#");
    return line.substr(first_,(last_ - first_ + 1));
}
void Simulation::readHumanFile(string humanFile) {
    if(humanFile.length() == 0){
        exit(1);
    }
    string line, houseID;
    int age;
    unsigned hMemID;
    char gen;

    ifstream infile(humanFile);
    if(!infile.good()){
        exit(1);
    }
    while(getline(infile, line, ',')){
        unique_ptr < vector < vector < pair<string, double >> >> trajectories(new vector < vector < pair<string, double >> >());

        for(int i = 0; i < 5; i++){
            houseID = line;
            getline(infile, line, ',');
            hMemID = strtol(line.c_str(), NULL, 10);
            getline(infile, line, ',');
            gen = line[0];
            getline(infile, line, ',');
            age = strtol(line.c_str(), NULL, 10);

            vector<pair<string,double>> path;
            getline(infile, line);
            stringstream ss;
            ss << line;
            while(getline(ss, line, ',')){
                string hID = line;
                getline(ss, line, ',');
                double timeSpent = strtod(line.c_str(), NULL);
                path.push_back(make_pair(hID, timeSpent));
            }
            trajectories->push_back(move(path));
            if(i < 4){
                getline(infile, line, ',');
            }
        }

        unique_ptr<Human> h(new Human(houseID, hMemID, age, gen, trajectories, rGen, currentDay, ForceOfImportation));

        std::set<std::string> locsVisited = h->getLocsVisited();
        for(std::set<std::string>::iterator itrSet = locsVisited.begin(); itrSet != locsVisited.end(); itrSet++){
            if(locations.find(*itrSet) != locations.end()){
                locations.find(*itrSet)->second->addHuman(h.get());
            }
        }

        humans.insert(make_pair(houseID, move(h)));

        while (infile.peek() == '\n'){
            infile.ignore(1, '\n');            
        }
    }
    infile.close();
}

void Simulation::readVaccinationGroupsFile(){
    if (vaccinationGroupsFile.length() == 0) {
	exit(1);
    }
    string line;
    int maxAge;
    int minAge;
    ifstream infile(vaccinationGroupsFile);
    if(!infile.good()){
	exit(1);
    }
    while(getline(infile, line, ',')){
        minAge = strtol(line.c_str(), NULL, 10);
        getline(infile, line, '\n');
        maxAge = strtol(line.c_str(), NULL,10);
	ageGroups.insert(make_pair(minAge,maxAge));
    }
    infile.close();
}


bool Simulation::checkAgeToVaccinate(int age_){
    std::map<int,int>::iterator itAge = ageGroups.begin();
    for(; itAge != ageGroups.end(); itAge++){
	//      for(int k = (*itAge).first; k <= (*itAge).second; k++){
	if(age_ >= (*itAge).first * 365 && age_ <= (*itAge).second * 365){
	    return true;
	}
    }
    return false;
}

Simulation::Simulation(string line) {
    currentDay = 0;
    year = 0;
    configLine = line;
}

Simulation::Simulation() {
}



Simulation::Simulation(const Simulation & orig) {
}

Simulation::~Simulation() {
}
