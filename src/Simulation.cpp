#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include <math.h>
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

    readAegyptiFile(aegyptiRatesFile);

    outputPath.erase(remove(outputPath.begin(), outputPath.end(), '\"'), outputPath.end());
    outputReport.setupReport(reportsFile,outputPath,simName);

    RandomNumGenerator rgen(rSeed, huImm, emergeFactor, 1 / mozDailyDeathRate.back(),firstBiteRate.back(), halflife);
    rGen = rgen;

    RandomNumGenerator rgen2(rSeedInf, huImm, emergeFactor, 1 / mozDailyDeathRate.back(),firstBiteRate.back(), halflife);
    rGenInf = rgen2;

    readDiseaseRatesFile();
    if(vaccineAdvanceMode == false){
	readVaccineProfileFile();
    }
    if(trialVaccination){
	readVaccinationGroupsFile();
    }
    readLocationFile(locationFile);
    readHumanFile(trajectoryFile);
    return simName;
}



void Simulation::simEngine() {
    while(currentDay < numDays){
	//	printf("Simulation day: %d\n", currentDay);
        for(auto itLoc = locations.begin(); itLoc != locations.end(); itLoc++){
            itLoc->second->updateInfectedVisitor();
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


    std::map<unsigned,double> ForceOfImportation;
    ForceOfImportation.clear();
    ForceOfImportation = (year - 1) < annualForceOfImportation.size() ? annualForceOfImportation[year - 1] : annualForceOfImportation.back();

    for(auto it = humans.begin(); it != humans.end(); ++it){
        // daily mortality for humans by age
        if(rGen.getEventProbability() < (deathRate * it->second->getAgeDays(currentDay)))
            it->second->reincarnate(currentDay);

        // update temporary cross-immunity status if necessary
        if(currentDay == it->second->getImmEndDay())
            it->second->setImmunityTemp(false);

        // update infection status if necessary
        if(it->second->infection != nullptr)
            it->second->checkRecovered(currentDay);

        // select movement trajectory for the day
        (it->second)->setTrajDay(rGen.getRandomNum(5));

        // simulate possible imported infection
	for(unsigned serotype = 1; serotype <= 4; serotype++){
	    if(!it->second->isImmune(serotype)){
		if(rGen.getEventProbability() < ForceOfImportation.at(serotype)){         
                    it->second->infect(currentDay, serotype, &rGenInf, &disRates, &hospRates, normdev);                
                }
            }
        }

	//update vaccine immunity if necessary
	if(it->second->isVaccinated() && vaccineAdvanceMode == true){
	    if(currentDay == it->second->getVaxImmEndDay()){
		it->second->setVaxImmunity(false);
	    }
	}
    	// vaccination, first record the cohorts depending on the vaccination strategy, then vaccinate. 


    	age = it->second->getAgeDays(currentDay);
	if(trialVaccination == true){
	    if(currentDay == vaccineDay){
		it->second->setAgeTrialEnrollment(age);
		it->second->setSeroStatusAtVaccination();
		it->second->setCohort(cohort);
		//one-time vaccination by age groups in the trial
    		if(checkAgeToVaccinate(age)){
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
    			if(vaccineAdvanceMode == true){
    			    it->second->vaccinateAdvanceMode(currentDay,rGenInf,vaccineProtection,vaccineWaning);
    			}else{
    			    it->second->vaccinate(&VE_pos, &VE_neg, propInf, currentDay);
    			}
    		    }
    		}
    	    }
    	}else if(routineVaccination == true){
    	    if(currentDay >= vaccineDay){
    		// routine vaccination by age
		if(age == vaccineAge * 365){
		    it->second->setAgeTrialEnrollment(age);
		    it->second->setSeroStatusAtVaccination();
		    it->second->setCohort(cohort);
    		    if(rGenInf.getEventProbability() < vaccineCoverage){
    			if(vaccineAdvanceMode == true){
    			    it->second->vaccinateAdvanceMode(currentDay,rGenInf,vaccineProtection,vaccineWaning);
    			}else{
    			    it->second->vaccinate(&VE_pos, &VE_neg, propInf, currentDay);
    			}
    		    }
    		}
    	    }    
    	}

    	// catchup vaccination
    	if(catchupFlag == true && vaccineDay == currentDay){
    	    if(age > vaccineAge * 365 && age < 18 * 365){
    		if(rGenInf.getEventProbability() < vaccineCoverage){
    		    if(vaccineAdvanceMode == true){
    			it->second->vaccinateAdvanceMode(currentDay,rGenInf,vaccineProtection,vaccineWaning);
    		    }else{
    			it->second->vaccinate(&VE_pos, &VE_neg, propInf, currentDay);
    		    }
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

    // Read entomological parameters that depend on temperature
    // If there are not enough values, take the last one
    double mozEIP = currentDay < meanDailyEIP.size() ? meanDailyEIP[currentDay] : meanDailyEIP.back();
    double mozDeath = currentDay < mozDailyDeathRate.size() ? mozDailyDeathRate[currentDay] : mozDailyDeathRate.back();
    double mozFBiteRate = currentDay < firstBiteRate.size() ? firstBiteRate[currentDay] : firstBiteRate.back();
    double mozSBiteRate = currentDay < secondBiteRate.size() ? secondBiteRate[currentDay] : secondBiteRate.back();

    //    printf("Day %d EIP %.4f death %.4f fbite %.4f sbite %.4f\n", currentDay, mozEIP, mozDeath, mozFBiteRate, mozSBiteRate);
    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
	outputReport.updateMosquitoReport(currentDay, (it->second).get());

        if(it->second->infection != nullptr){
	    if(it->second->infection->getStartDay() < 0 && rGenInf.getEventProbability() < rGenInf.getMozLatencyRate(mozEIP)){
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
            ++it;
            mosquitoes.erase(it_temp);
            continue;
        }

        // if the mosquito bites first, then let it bite and then see about dying
        while(biteTime < dieTime && biteTime <= 1.0){
            biteTaken = it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,&disRates,&hospRates,currentDay,numDays,&out,normdev);
	    if(biteTaken){
		it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozSBiteRate));
	    }else{
		//There could be a bug here ! check the number of bites before assigning the restdays
		if(it->second->getNumBites() == 0){
		    it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozFBiteRate));
		}else{
		    it->second->setBiteStartDay(currentDay + rGen.getMozRestDays(mozSBiteRate));
		}
	    }
	    
	    biteTime = it->second->getBiteStartDay() - double(currentDay);
	    
	    if(!biteTaken){
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
            ++it;
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
    double seasFactor = getMosquitoSeasonality(currentDay);

    for(auto& x : locations){
	// The emergence is multiplied by a seasonal factor that could vary everyday
	// The emergence seasonal factor is being included by reading an external file with the entomological parameters
	// In this branch the seasonal factor is simulated in here using trigonometric functions

        mozCount = rGen.getMozEmerge(x.second->getEmergenceRate(), seasFactor * emergeFactor);

        for(int i = 0; i < mozCount; i++){
	    // This is a temporary fix, to be discussed further...
	    // When creating a mosquito there's a lifespan assigned, but it's not being used anymore
            unique_ptr<Mosquito> moz(new Mosquito(
                double(currentDay) + rGen.getMozLifeSpan(), double(currentDay) + rGen.getMozRestDays(mozFBiteRate), x.first));
            mosquitoes.insert(make_pair(x.first, move(moz)));
        }
    }
}

double Simulation::getMosquitoSeasonality(unsigned currDay){
    double PI = 3.14159265;
    double signal_mozz = y_mag * sin((2 * PI + y_phase * PI / 360) * currDay / (365 * y_freq)) + multi_mag * sin((2 * PI + multi_phase * PI / 360) * currDay / (365 * multi_freq)) + signal_offset;
    if(signal_mozz < 0.0){
	signal_mozz = 0.0;
    }
    //    printf("Day: %d, PI %.4f, YM %.4f, MM %.4f YF %.4f, MF %.4f, OFFSET %.2f, SIGNAL %.4f\n", currDay, PI, y_mag, multi_mag, y_freq, multi_freq, signal_offset, signal_mozz);
    return signal_mozz;
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
    vaccineDay = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    routineVaccination = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    vaccineCoverage = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    vaccineAge = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    catchupFlag = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    trialVaccination = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    vaccinationGroupsFile = line;
    getline(infile, line, ',');
    vaccineAdvanceMode = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    vaccineProtection = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    vaccineWaning = strtod(line.c_str(), NULL);
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
    vaccineProfileFile = line;
    getline(infile, line, ',');
    normdev = strtod(line.c_str(), NULL);    
    getline(infile, line, ',');
    propInf = strtod(line.c_str(), NULL);    
    getline(infile, line, ',');
    deathRate = strtod(line.c_str(), NULL);    
    getline(infile, line, ',');
    std::string annualFoiFile = line;
    getline(infile, line, ',');
    std::string foiFile = line;
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
    getline(infile, line, ',');
    y_mag = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    multi_mag = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    y_freq = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    multi_freq = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    y_phase = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    multi_phase = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    signal_offset = strtod(line.c_str(), NULL);

    readInitialFOI(foiFile);
    readAnnualFOI(annualFoiFile);
}

void Simulation::readInitialFOI(std::string fileIn){
    if(fileIn.length() == 0){
	exit(1);
    }
    ifstream infile(fileIn);
    string line;
    if(!infile.good()){
	exit(1);
    }
    getline(infile, line);
    for(int i = 0; i < 4; i++){
	getline(infile, line, ',');
	double foiTemp = strtod(line.c_str(),NULL);
	InitialConditionsFOI.push_back(foiTemp);
    }
    if(InitialConditionsFOI.size() != 4){
	printf("InitialConditionsFOI not set\n");
	exit(1);
    }
}

void Simulation::readAnnualFOI(std::string fileIn){
    if(fileIn.length() == 0){
	exit(1);
    }
    ifstream infile(fileIn);
    string line;
    if(!infile.good()){
	exit(1);
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
	std::map<unsigned,double> map_temp;
	map_temp.clear();
	map_temp.insert(make_pair(1, d1));
	map_temp.insert(make_pair(2, d2));
	map_temp.insert(make_pair(3, d3));
	map_temp.insert(make_pair(4, d4));     
	annualForceOfImportation.push_back(map_temp);
    }

    if(annualForceOfImportation.size() == 0){
	printf("ForceOfImportation not set\n");
	exit(1);
    }
}

void Simulation::readAegyptiFile(string file){
    ifstream infile(file);
    if (!infile.good()) {
        exit(1);
    }
    std::string line;
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
	exit(1);
    }
    infile.close();
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



void Simulation::readVaccineProfileFile(){
    if(vaccineProfileFile.length() == 0){
        exit(1);
    }
    string line;
    unsigned par = 0;
    double parPos;
    double parNeg;

    ifstream infile(vaccineProfileFile);
    if(!infile.good()){
        exit(1);
    }
    while(getline(infile, line, ',')){
        parPos = strtod(line.c_str(), NULL);
        getline(infile, line, '\n');
        parNeg = strtod(line.c_str(), NULL);
        if(par <= 2){
            VE_pos.insert(make_pair(par, parPos));
            VE_neg.insert(make_pair(par, parNeg));
        }
        par++;
    }
    infile.close();
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

        unique_ptr<Human> h(new Human(houseID, hMemID, age, gen, trajectories, rGen, currentDay, InitialConditionsFOI));

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
