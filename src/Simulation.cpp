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
    /*
    int dayVax0 = vaccineDay - 365;
    if(dayVax0 < 0){
        dayVax0 = 0;
    }

    if(currentDay == dayVax0){
	        for(int i = 0; i < 101; i++){
            seroposAtVax[i] = 0;
            seronegAtVax[i] = 0;
            disAtVax[i] = 0;
            hospAtVax[i] = 0;
	    }
     }
    */

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
        if(rGen.getEventProbability() < ForceOfImportationTrial / 365.0){
            for(int serotype = 1; serotype <= 4; serotype++){
                if(!it->second->isImmune(serotype)){
                    it->second->infect(currentDay, serotype, &rGenInf, &disRates, &hospRates, normdev);                
                }
            }
        }

    	// vaccination
    	age = it->second->getAgeDays(currentDay);
    	if(currentDay == vaccineDay){
    	    it->second->setAgeTrialEnrollment(age);
    	    it->second->setSeroStatusAtVaccination();
    	    it->second->setCohort(cohort);
    	    //one-time vaccination by age groups in the trial
    	    if(trialVaccination == true){
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
    	}
    	if(routineVaccination == true){
    	    if(currentDay >= vaccineDay){
    		// routine vaccination by age
    		if(age == vaccineAge * 365){
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
            biteTaken = it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,&disRates,&hospRates,currentDay,numDays,&out,normdev);
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
