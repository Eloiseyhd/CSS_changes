#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
#include <utility>
#include <sstream>
#include "Simulation.h"

using std::runtime_error;

void Simulation::simulate() {
    simEngine();
    out.close();
    outpop.close();
    outprevac.close();
}



string Simulation::readInputs() {
    readSimControlFile(configLine);

    readAegyptiFile(aegyptiRatesFile);

    outputPath.erase(remove(outputPath.begin(), outputPath.end(), '\"'), outputPath.end());
    
    RandomNumGenerator rgen(rSeed, huImm, emergeFactor, 1 / mozDailyDeathRate.back(), firstBiteRate.back(), halflife);
    rGen = rgen;
    
    RandomNumGenerator rgen2(rSeedInf, huImm, emergeFactor, 1 / mozDailyDeathRate.back(), firstBiteRate.back(), halflife);
    rGenInf = rgen2;
    
    readDiseaseRatesFile();
    
    readVaccineSettingsFile();
    
    readVaccineProfilesFile();

    if(vaccinationStrategy == "random_trial"){
	recruitmentTrial.setupRecruitment(trialSettingsFile, vaccines, outputPath, simName, &rGen);
	//	printf("Vax Sample: %d, Plac Sample: %d\n", recruitmentTrial.getVaccineSampleSize(), recruitmentTrial.getPlaceboSampleSize());
	//	printf("Recruitment day: %d\n",recruitmentTrial.getRecruitmentStartDay());
    }
    if(vaccinationStrategy == "sanofi_trial"){
	readVaccinationGroupsFile();
    }

    readLocationFile(locationFile);
    readBirthsFile(birthsFile);
    readTrajectoryFile(trajectoryFile);
    outputReport.setupReport(reportsFile,outputPath,simName);
    return simName;
}



void Simulation::simEngine() {
    deathMoz = 0;
    lifeMoz = 0;
    while(currentDay < numDays){
        std::cout << "# day " << humans.size() << " " << mosquitoes.size() << std::endl;
        //rGen.showState(48, "# Daily RNG state excerpt: ");
	humanDeaths = 0;
        if(ceil(double(currentDay + 1) / 365.0) != ceil(double(currentDay) / 365.0)){
            year++;
            updatePop();
        }
	printf("day %d year %d\n",currentDay, year);
        for(auto itLoc = locations.begin(); itLoc != locations.end(); itLoc++){
            itLoc->second->updateInfectedVisitor();
        }
	if(vaccinationStrategy == "random_trial"){
	    //	    printf("Day: %d\n", currentDay);
	    if(currentDay == recruitmentTrial.getRecruitmentStartDay()){
		printf("Current Day : %d is recruitment Start Day\n",currentDay);
		selectEligibleTrialParticipants();
	    }
	    recruitmentTrial.update(currentDay);
	}
	
        humanDynamics();
	outputReport.printReport(currentDay);
        mosquitoDynamics();	
        currentDay++;
	//       	printf("tempStats, %d, %d, %lu\n", currentDay, humanDeaths, mosquitoes.size());
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
    int age;
    int cohort = 0;
    if(currentDay >= vaccineDay){
        cohort = floor(double(currentDay - vaccineDay) / 365.0) + 1;
    }
    map<unsigned,double>ForceOfImportation;
    ForceOfImportation.clear();
    ForceOfImportation = currentDay < dailyForceOfImportation.size() ? dailyForceOfImportation[currentDay] : dailyForceOfImportation.back();

    for(auto locIt = zones.begin(); locIt != zones.end();){
      std::string tmpstr = (*locIt);
      zones_counts[tmpstr] = 0;
      ++locIt;
    }
    
    // Newborns !!!
    auto newbornsRange = future_humans.equal_range(currentDay);
    // walk through today's births
    // add to humans, locations
    for (auto it = newbornsRange.first; it != newbornsRange.second; it++){
	//        printf("Human %s is born in day %d\n", it->second->getPersonID().c_str(), currentDay);
        // copy-construct shared pointer to human
        sp_human_t h = it->second;
        if(locations.find(h->getHouseID()) != locations.end()){
            for(const auto & loc_str : h->getLocsVisited()){
                // set.insert already performs check for presence
                auto the_loc = locations.find(loc_str);
                if (the_loc != locations.end() ) {
                    the_loc->second->addHuman(h);
                }
            }
            h->initializeHuman(currentDay, InitialConditionsFOI,rGen);
            // deep copy of string ref
            // unnecessary?
            string loc_copy(h->getHouseID());
            humans.insert(make_pair(loc_copy, h));
        }
    }
    // finally, delete range
    future_humans.erase(newbornsRange.first, newbornsRange.second);

    
    //update alive humans
    for(auto & phum : humans) {
        // check logic!! increment moved to loop
        if(phum.second->isDead()){
            // skip this person
            continue;
        }
        // daily mortality for humans by age
        // why the draw if humans are assigned death day?? -> that's a bug! FIXED with only checking for death day
        if(phum.second->getDeathday() == currentDay){
	    if(vaccinationStrategy == "random_trial" && phum.second->isEnrolledInTrial() == true){
                printf("Human %s removed from trial\n", phum.second->getPersonID().c_str());
                recruitmentTrial.removeParticipant(phum.second.get(),currentDay);
	    }
            // name of each location 
	    for(const auto & loc_str : phum.second->getLocsVisited() ){
                // please check logic
                // grab iterator to map element
                auto loc_it = locations.find(loc_str);
                // error??
                if (loc_it != locations.end() ) {
                    // reemove human from the location pointed to 
                    loc_it->second->removeHuman(phum.second);
                }
	    }
	    //	    printf("Human is dead %s\n", phum.second->getPersonID().c_str());
	    phum.second->kill();
	    humanDeaths++;
	    continue;
	}
        // update temporary cross-immunity status if necessary
        if(currentDay == phum.second->getImmEndDay())
            phum.second->setImmunityTemp(false);

        // update infection status if necessary
        if(phum.second->infection != nullptr){
            phum.second->checkRecovered(currentDay);
	}

        // select movement trajectory for the day
        phum.second->setTrajDay(rGen.getRandomNum(N_TRAJECTORY));

        // simulate possible imported infection
	for(unsigned serotype = 1; serotype <= (N_SERO); serotype++){
	    if(rGen.getEventProbability() < ForceOfImportation.at(serotype)){
		if(!phum.second->isImmune(serotype)){
		    outputReport.addImportation(serotype,(phum.second).get());
		    phum.second->infect(currentDay, serotype, &rGenInf, &disRates, &hospRates);                
		}
	    }
	}

	//update vaccine immunity if necessary
	if(phum.second->isVaccinated()){
	    phum.second->updateVaccineEfficacy(currentDay);
	    //	    phum.second->getVaccine()->printVaccine();
	}


    	// Routine vaccination: first record the cohorts depending on the vaccination strategy, then vaccinate. 
    	age = phum.second->getAgeDays(currentDay);
	if(vaccinationStrategy == "sanofi_trial"){
	    if(currentDay == vaccineDay){
		phum.second->setAgeTrialEnrollment(age);
		phum.second->setSeroStatusAtVaccination();
		phum.second->setCohort(cohort);
		//one-time vaccination by age groups in the trial
    		if(checkAgeToVaccinate(age)){
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
			phum.second->vaccinateWithProfile(currentDay, &rGenInf, vaccines.at(vaccineID));
    		    }
    		}
    	    }
    	}else if(vaccinationStrategy == "routine"){
    	    if(currentDay >= vaccineDay){
    		// routine vaccination by age
		if(age == vaccineAge * 365){
		    phum.second->setAgeTrialEnrollment(age);
		    phum.second->setSeroStatusAtVaccination();
		    phum.second->setCohort(cohort);
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
			phum.second->vaccinateWithProfile(currentDay, &rGenInf, vaccines.at(vaccineID));
    		    }
    		}
    	    }    
    	}

    	// catchup vaccination
    	if(catchupFlag == true && vaccineDay == currentDay && vaccinationStrategy == "routine"){
    	    if(age > vaccineAge * 365 && age < 18 * 365){
    		if(rGenInf.getEventProbability() < vaccineCoverage){
		    phum.second->vaccinateWithProfile(currentDay, &rGenInf, vaccines.at(vaccineID));
    		}
    	    }
    	}
	outputReport.updateReport(currentDay,(phum.second).get(), locations[phum.second->getHouseID()].get());
	Location * housetmp = locations[phum.second->getHouseID()].get();
	zones_counts[housetmp->getZoneID()]++;
    }
    /*    for(auto locIt = zones.begin(); locIt != zones.end();){
      printf("zone: %s counts: %u\n", (*locIt).c_str(), zones_counts[(*locIt)]);
      ++locIt;
      }*/
}

void Simulation::mosquitoDynamics(){
    double biteTime, dieTime;
    bool biteTaken;
    generateMosquitoes();
    // Read entomological parameters that depend on temperature
    // If there are not enough values, take the last one
    double mozEIP = currentDay < meanDailyEIP.size() ? meanDailyEIP[currentDay] : meanDailyEIP.back();
    double mozDeath = currentDay < mozDailyDeathRate.size() ? mozDailyDeathRate[currentDay] : mozDailyDeathRate.back();
    double mozFBiteRate = currentDay < firstBiteRate.size() ? firstBiteRate[currentDay] : firstBiteRate.back();
    double mozSBiteRate = currentDay < secondBiteRate.size() ? secondBiteRate[currentDay] : secondBiteRate.back();

    
    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
	if(it->second->infection != nullptr){
	    if(it->second->infection->getStartDay() < 0 && rGenInf.getEventProbability() < rGenInf.getMozLatencyRate(mozEIP)){
		//            if(currentDay == it->second->infection->getStartDay())
		//		printf("Setting infection for mosquito day %d\n",currentDay);
                it->second->infection->setInfectiousnessMosquito(mozInfectiousness, currentDay);
	    }
        }
	
        // determine if the mosquito will bite and/or die today, and if so at what time
        biteTime = double(numDays + 1);
        dieTime = double(numDays + 1); 
        
	if(it->second->getBiteStartDay() <= double(currentDay + 1)){
            biteTime = it->second->getBiteStartDay() - double(currentDay);
            if(biteTime < 0.0){
                biteTime = rGen.getEventProbability();
            }
        }

	// If the mosquito dies today, then set a time to day during the day
	// Right now that time is being set with an uniform distribution -- Double check!!!!
        if(rGen.getEventProbability() < mozDeath){
            dieTime =rGen.getEventProbability();
	    // it->second->getDDay() - double(currentDay);
        }


        // if the mosquito dies first, then kill it
        if(dieTime <= biteTime && dieTime <= 1.0){
            auto it_temp = it;
	    lifeMoz += currentDay - it->second->getBirthDay();
	    deathMoz++;
            ++it;
            mosquitoes.erase(it_temp);
            continue;
        }

        // if the mosquito bites first, then let it bite and then see about dying
        while(biteTime < dieTime && biteTime <= 1.0){
            biteTaken = it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,&disRates,&hospRates,currentDay,numDays,&out, mozEIP);
	    if(biteTaken){
		it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozSBiteRate));
	    }else{
		if(it->second->getNumBites() == 0){
		    it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozFBiteRate));
		}else{
		    it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozSBiteRate));
		}
	    }
	   
	    biteTime = it->second->getBiteStartDay() - double(currentDay);
	    
	    if(!biteTaken){
		//		printf("Bite not taken trying to move day %d\n", currentDay);
                string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
                if(newLoc != "TOO_FAR_FROM_ANYWHERE"){
                    it->second->setLocation(newLoc);
                    mosquitoes.insert(make_pair(newLoc, move(it->second)));
                    auto it_temp = it;
                    ++it;
                    mosquitoes.erase(it_temp);
                }
            }
        }
        if(dieTime < 1.0){
            auto it_temp = it;
	    lifeMoz += currentDay - it->second->getBirthDay();
	    deathMoz++;
            ++it;
            mosquitoes.erase(it_temp);
            continue;
        }
	//update report for mosquitoes
	outputReport.updateMosquitoReport(currentDay,(it->second).get(), locations[it->second->getLocationID()].get());
	
        // let the mosquito move if that happens today 
    	double moveProb = rGen.getEventProbability();
        if(moveProb < mozMoveProbability){
            string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
            if(newLoc != "TOO_FAR_FROM_ANYWHERE"){
                it->second->setLocation(newLoc);
                mosquitoes.insert(make_pair(newLoc, move(it->second)));
                auto it_temp = it;
                ++it;
                mosquitoes.erase(it_temp);
            }
            else{
                ++it;
            }
        }
        else{
            ++it;
        }
    }
}



void Simulation::generateMosquitoes(){
    int mozCount = 0;
    double mozFBiteRate = currentDay < firstBiteRate.size() ? firstBiteRate[currentDay] : firstBiteRate.back();
    double seasFactor = currentDay < dailyEmergenceFactor.size() ? dailyEmergenceFactor[currentDay] : dailyEmergenceFactor.back();

    for(auto& x : locations){
        mozCount = rGen.getMozEmerge(x.second->getEmergenceRate(), seasFactor * emergeFactor);

        for(int i = 0; i < mozCount; i++){
            unique_ptr<Mosquito> moz(new Mosquito(
                double(currentDay) + rGen.getMozLifeSpan(), double(currentDay) + rGen.getMozRestDays(mozFBiteRate), x.first));
	    moz->setBirthDay(currentDay);
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
    for(auto & hum : humans) {
	recruitmentTrial.addPossibleParticipant(hum.second.get(),currentDay);
    }
    //    printf("In total %lu participants are eligible out of %lu\n",recruitmentTrial.getEligibleParticipantsSize(),humans.size());
    recruitmentTrial.shuffleEligibleParticipants();
    //    recruitmentTrial.printEligibleGroups();
}

void Simulation::readAegyptiFile(string file){
    ifstream infile(file);
    if (!infile.good()) {
        //exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
    }
    string line;
    getline(infile,line);
    firstBiteRate.clear();
    secondBiteRate.clear();
    mozDailyDeathRate.clear();
    meanDailyEIP.clear();
    dailyEmergenceFactor.clear();

    while (getline(infile, line, ',')) {
	double eip_temp = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
	double fb = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
	double sb = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
	double dr = strtod(line.c_str(), NULL);
        getline(infile, line, '\n');
	double ef = strtod(line.c_str(), NULL);
	if(eip_temp + fb + sb + dr + ef > 0){
	    firstBiteRate.push_back(fb);
	    secondBiteRate.push_back(sb);
	    mozDailyDeathRate.push_back(dr);
	    meanDailyEIP.push_back(eip_temp);
	    dailyEmergenceFactor.push_back(ef);
	}
    }
    if(firstBiteRate.empty() || secondBiteRate.empty() || mozDailyDeathRate.empty() || meanDailyEIP.empty() || dailyEmergenceFactor.empty()){
        throw runtime_error("In Simulation.cpp, something missing");
	//exit(1);
    }
    infile.close();
}

void Simulation::readSimControlFile(string line) {
    std::stringstream infile;
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
    birthsFile = line;
    getline(infile, line, ',');
    string dailyFoiFile = line;
    getline(infile, line, ',');
    string foiFile = line;
    getline(infile, line, ',');
    huImm = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    emergeFactor = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mozInfectiousness = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mozMoveProbability = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    aegyptiRatesFile = line;

    readInitialFOI(foiFile);
    readDailyFOI(dailyFoiFile);

}

void Simulation::readInitialFOI(string fileIn){
    if(fileIn.length() == 0){
        throw runtime_error("In Simulation.cpp, something missing");
	//exit(1);
    }
    ifstream infile(fileIn);
    string line;
    if(!infile.good()){
        throw runtime_error("In Simulation.cpp, something missing");
	//exit(1);
    }
    getline(infile, line);
    for(int i = 0; i < N_SERO; i++){
	getline(infile, line, ',');
	double foiTemp = strtod(line.c_str(),NULL);
	InitialConditionsFOI.push_back(foiTemp);
    }
    if(InitialConditionsFOI.size() != N_SERO){
	throw runtime_error("InitialConditionsFOI not set\n");
	//exit(1);
    }
}

void Simulation::readDailyFOI(string fileIn){
    if(fileIn.length() == 0){
	printf("In Simulation.cpp, something missing in readDailyFoI %s\n", fileIn.c_str());
        throw runtime_error("In Simulation.cpp, something missing in readDailyFoI");
	//exit(1);
    }
    ifstream infile(fileIn);
    string line;
    if(!infile.good()){
	printf("In Simulation.cpp, something missing, file %s is not good in readDailyFoI\n", fileIn.c_str());
        throw runtime_error("In Simulation.cpp, something missing, file is not good in readDailyFoI");
	//exit(1);
    }
    getline(infile, line);
    while (getline(infile, line, ',')) {
	double d1 = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
	double d2 = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
	double d3 = strtod(line.c_str(), NULL);
        getline(infile, line, '\n');
	double d4 = strtod(line.c_str(), NULL);
	map<unsigned,double> map_temp;
	map_temp.clear();
	map_temp.insert(make_pair(1, d1));
	map_temp.insert(make_pair(2, d2));
	map_temp.insert(make_pair(3, d3));
	map_temp.insert(make_pair(4, d4));     
	dailyForceOfImportation.push_back(map_temp);
    }
    if(dailyForceOfImportation.size() == 0){
	throw runtime_error("ForceOfImportation not set\n");
	//exit(1);
    }
}

void Simulation::readLocationFile(string locFile) {
    if (locFile.length() == 0) {
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
    }
    string line, locID, locType, nID,zID;
    double x, y, mozzes;

    ifstream infile(locFile);
   if (!infile.good()) {
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
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
	zID = line;
	
        getline(infile, line, ',');
	nID = line;
        getline(infile, line, ',');
        locID = line;

        getline(infile, line);
        mozzes = strtod(line.c_str(), NULL);
	
        while (infile.peek() == '\n'){
            infile.ignore(1, '\n');
	}
        unique_ptr<Location> location(new Location(locID, locType, nID, zID, x, y, mozzes));
        locations.insert(make_pair(locID, move(location)));
	zones.insert(zID);
    }
    infile.close();
    outputReport.setupZones(zones);
    for(auto locIt = zones.begin(); locIt != zones.end();){
	std::string tmpstr = (*locIt);
	zones_counts.insert(make_pair(tmpstr,0));
	++locIt;
    }
}



void Simulation::readDiseaseRatesFile(){
    if(diseaseRatesFile.length() == 0){
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
    }
    string line;
    unsigned par = 0;
    double parDis;
    double parHosp;

    ifstream infile(diseaseRatesFile);
    if(!infile.good()){
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
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
    vaccinationStrategy = "none";
    if(vaccineSettingsFile.length() == 0){
	printf("In Simulation.cpp, readvaccinesettings, something missing length is 0 for %s\n", vaccineSettingsFile.c_str());
        throw runtime_error("In Simulation.cpp, readvaccinesettings, something missing length is 0 for vaccine file");
        //exit(1);
    }
    string line;
    ifstream infile(vaccineSettingsFile);
    if(!infile.good()){
        //exit(1);
	printf("In Simulation.cpp (readVaccineSettingsFile) something missing. file %s not good\n", vaccineSettingsFile.c_str());
        throw runtime_error("In Simulation.cpp (readVaccineSettingsFile) something missing. file of vaccine settings is not good\n");
    }
    printf("Reading vaccine settings file %s\n", vaccineSettingsFile.c_str());
    while(getline(infile,line,'\n')){
	string line2,line3;
	vector<string>param_line = getParamsLine(line);
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
    printf("file %s read\n", vaccineSettingsFile.c_str());
    //    printf("vStrategy %s day %d age %d coverage: %.2f: groups_file: %s: profilesFile: %s: ID: %d\n",vaccinationStrategy.c_str(), vaccineDay, vaccineAge,vaccineCoverage, vaccinationGroupsFile.c_str(), vaccineProfilesFile.c_str(), vaccineID);
}


void Simulation::readVaccineProfilesFile(){
    printf("reading vaccine profiles file\n");
    printf("file = %s\n", vaccineProfilesFile.c_str());
    
    if(vaccineProfilesFile.length() == 0){
        //exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
    }
    vaccines.clear();
    string line;
    ifstream infile(vaccineProfilesFile);
    if(!infile.good()){
        //exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
    }
    int numVaccines = 0;
    // First find the number of vaccines
    printf("reading vaccine profiles %s\n", vaccineProfilesFile.c_str());
    while(getline(infile,line,'\n')){
	string line2,line3;
	vector<string>param_line = getParamsLine(line);
	line2 = param_line[0];
	line3 = param_line[1];
	if(line2 == "vaccine_ids"){
	    numVaccines = this->parseInteger(line3);
	}
    }
    
    if(numVaccines > 0){
	//	printf("There are %d vaccines to read in the file\n",numVaccines);
	// Now we should read the rest of the parameters and store them in the appropiate vaccine structure
	for(unsigned i = 0;i < numVaccines; i++){
	    infile.clear();
	    infile.seekg(0);
	    Vaccine vaxTemp;
	    vaxTemp.setID(i);
	    while(getline(infile,line,'\n')){
		string line2,line3;
		vector<string>param_line = getParamsLine(line);
		line2 = param_line[0];
		line3 = param_line[1];
		string s_id = std::to_string(i);
		if(line2 == "vaccine_mode_" + s_id){
		    vaxTemp.setMode(this->parseString(line3));
		}
		if(line2 == "vaccine_name_" + s_id){
		    vaxTemp.setName(this->parseString(line3));
		}
		if(line2 == "vaccine_waning_" + s_id){
		    vaxTemp.setWaning(this->parseDouble(line3));
		}
		if(line2 == "vaccine_protection_" + s_id){
		    vaxTemp.setProtection(this->parseDouble(line3));
		}
		if(line2 == "vaccine_ve_" + s_id){
		    vaxTemp.setTotalVE(this->parseDouble(line3));
		}
		if(line2 == "vaccine_vepos_a_" + s_id){
		    vaxTemp.addVE_pos(this->parseDouble(line3),0);
		}
		if(line2 == "vaccine_veneg_a_" + s_id){
		    vaxTemp.addVE_neg(this->parseDouble(line3),0);
		}
		if(line2 == "vaccine_vepos_b_" + s_id){
		    vaxTemp.addVE_pos(this->parseDouble(line3),1);
		}
		if(line2 == "vaccine_veneg_b_" + s_id){
		    vaxTemp.addVE_neg(this->parseDouble(line3),1);
		}
		if(line2 == "vaccine_vepos_c_" + s_id){
		    vaxTemp.addVE_pos(this->parseDouble(line3),2);
		}
		if(line2 == "vaccine_veneg_c_" + s_id){
		    vaxTemp.addVE_neg(this->parseDouble(line3),2);
		}
		if(line2 == "vaccine_veneg_" + s_id){
		    vaxTemp.setVaccineEfficacy(false,this->parseDouble(line3));
		}
		if(line2 == "vaccine_vepos_" + s_id){
		    vaxTemp.setVaccineEfficacy(true,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRInfneg_" + s_id){
		    vaxTemp.setRRInf(false,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRInfpos_" + s_id){
		    vaxTemp.setRRInf(true,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRDisneg_" + s_id){
		    vaxTemp.setRRDis(false,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRDispos_" + s_id){
		    vaxTemp.setRRDis(true,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRHospneg_" + s_id){
		    vaxTemp.setRRHosp(false,this->parseDouble(line3));
		}
		if(line2 == "vaccine_RRHosppos_" + s_id){
		    vaxTemp.setRRHosp(true,this->parseDouble(line3));
		}
		if(line2 == "vaccine_waning_pos_" + s_id){
		    vaxTemp.setWaning(true,this->parseDouble(line3));
		}
		if(line2 == "vaccine_waning_neg_" + s_id){
		    vaxTemp.setWaning(false,this->parseDouble(line3));
		}
		if(line2 == "vaccine_prop_inf_" + s_id){
		    vaxTemp.setPropInf(this->parseDouble(line3));
		}
		if(line2 == "vaccine_normdev_" + s_id){
		    vaxTemp.setNormdev(this->parseDouble(line3));
		}
		if(line2 == "vaccine_schedule_" + s_id){
		    vector<int> rSchedule;
		    this->parseVector(line3, &(rSchedule));
		    vaxTemp.setRelativeSchedule(rSchedule);
		    rSchedule.clear();
		}
	    }
	    vaccines.insert(make_pair(i,vaxTemp));
	    vaxTemp.printVaccine();
	}
	for(unsigned j = 0;j < vaccines.size(); j++){
	    vaccines.at(j).printVaccine();
	}
    }else{
        string msg("There are no vaccines to read in ");
        msg += vaccineProfilesFile;
	throw runtime_error(msg);
	//exit(1);
    }
    infile.close();
}

vector<string> Simulation::getParamsLine(string line_){
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

int Simulation::parseInteger(string line){
    return strtol(line.c_str(), NULL, 10);
}

double Simulation::parseDouble(string line){
    return strtod(line.c_str(), NULL);
}

void Simulation::parseVector(string line, vector<int> * vector_temp){
    std::stringstream linetemp;
    string line2;
    linetemp.clear();
    linetemp << line;
    vector_temp->clear();

    while(getline(linetemp,line2,',')){
	int temp = strtol(line2.c_str(), NULL, 10);
	if(temp >= 0){
	    vector_temp->push_back(temp);
	}
    }

    if(vector_temp->empty()){
	throw runtime_error("Parsevector Vector_temp is empty\n");
        //exit(1);
    }
}
string Simulation::parseString(string line){
    size_t first_ = line.find_first_not_of(" \t#");
    size_t last_ = line.find_last_not_of(" \t#");
    return line.substr(first_,(last_ - first_ + 1));
}

void Simulation::readBirthsFile(string bFile){
    if(bFile.length() == 0){
	throw runtime_error("Incorrect births file\n");
	//exit(1);
    }
    ifstream infile(bFile);
    if(!infile.good()){
	std::cout << "births file is empty: " << bFile.c_str() << "\n";
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
    }
    string line, houseID,zID;
    int bday,dday;
    unsigned hMemID;
    char gen;
    
    while (getline(infile, line, ',')) {
	houseID = line;
	getline(infile, line, ',');
	hMemID = strtol(line.c_str(), NULL, 10);
        getline(infile, line, ',');
	gen = line[0];
        getline(infile, line, ',');
	bday = strtol(line.c_str(), NULL, 10);
	getline(infile, line, '\n');
	dday = strtol(line.c_str(), NULL, 10);
        // should this be an error condition?
	if(locations.find(houseID) == locations.end()){
	    printf("HouseID: %s not found\n", houseID.c_str());
        } else {
	    Location * housetmp = locations[houseID].get();
	    sp_human_t h(new Human(houseID, hMemID, housetmp->getZoneID(), gen, bday, dday, rGen));
	    //if(total_humans_by_id.find(h->getPersonID()) == total_humans_by_id.end()){
	    // map.insert checks, only inserts if none present
            total_humans_by_id.insert(make_pair(h->getPersonID(), h));
	    //}else{
		//		printf("Human is repeated %s\n", h->getPersonID().c_str());
	    //}
	}
    }
    infile.close();
    /*    for(auto it = total_humans_by_id.begin(); it != total_humans_by_id.end(); ++it){
	printf("humans by id %s -> house %s\n", it->first.c_str(),it->second->getHouseID().c_str() );
	}*/
}

void Simulation::readTrajectoryFile(string trajFile){
    if(trajFile.length() == 0){
        //exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
    }
    printf("reading %s file with trajectories\n", trajFile.c_str());
    string line, houseID, personID;
    unsigned hMemID;
    //
    ifstream infile(trajFile);
    if(!infile.good()){
        throw runtime_error("In Simulation.cpp, something missing");
        //exit(1);
    }
    while(getline(infile, line, ',')){
        unique_ptr<traject_t> ptrajectories(new traject_t);
        for(int itraj = 0; itraj < N_TRAJECTORY; itraj++){
            houseID = line;
            getline(infile, line, ',');
            hMemID = strtol(line.c_str(), NULL, 10);
	    personID =houseID + std::to_string(hMemID);
            vpath_t path;
            getline(infile, line);
            std::stringstream ss;
            ss << line;
            while(getline(ss, line, ',')){
                string hID = line;
                getline(ss, line, ',');
                double timeSpent = strtod(line.c_str(), NULL);
                path.push_back(make_pair(hID, timeSpent));
            }
            // directly insert path
            (*ptrajectories)[itraj] = move(path);
            if(itraj < N_TRAJECTORY-1){
                getline(infile, line, ',');
            }
        }

	//error condition? -> just ignore if not valid trajectory
	if(locations.find(houseID) != locations.end()){
	    auto ithum = total_humans_by_id.find(personID);
	    if( ithum != total_humans_by_id.end()){
		//auto tmpIt = ithum;
		//++ithum;
		sp_human_t h = ithum->second;
                // set
		h->setTrajectories(ptrajectories);
		if(h->getBirthday() < 0){
		    for(const auto & loc_str : h->getLocsVisited()) {
                        auto itloc = locations.find(loc_str);
			if( itloc != locations.end()){
			    itloc->second->addHuman(h);
			}
		    }
		    h->initializeHuman(currentDay, InitialConditionsFOI,rGen);
		    humans.insert(make_pair(houseID, h));
		}else{
		    future_humans.insert(make_pair(h->getBirthday(),h));
		}
                // cleanup at end
                // necessary? shouldn't each person be unique? -> I don't understand this question
		total_humans_by_id.erase(ithum);
	    }
	}

        while (infile.peek() == '\n'){
            infile.ignore(1, '\n');            
        }
    }
    infile.close();
    /*    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
	printf("Human %s attractiveness %f\n", itHum->second->getPersonID().c_str(), itHum->second->getAttractiveness());
	}*/
    total_humans_by_id.clear();
    printf("Trajectories have finished successfully\n");
}

void Simulation::readVaccinationGroupsFile(){
    if (vaccinationGroupsFile.length() == 0) {
	//exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
    }
    string line;
    int maxAge;
    int minAge;
    ifstream infile(vaccinationGroupsFile);
    if(!infile.good()){
	//exit(1);
        throw runtime_error("In Simulation.cpp, something missing");
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
    //map<int,int> & the_age
    for(const auto & the_age : ageGroups) {
	//      for(int k = (*itAge).first; k <= (*itAge).second; k++){
	if(age_ >= the_age.first * 365 && age_ <= the_age.second * 365){
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

//Simulation::Simulation() {
//}



//Simulation::Simulation(const Simulation & orig) {
//}

//Simulation::~Simulation() {
//}
