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
    unsigned numHu = humans.size();

    cout << "\n" << simName <<": Starting ... " << endl;

    simEngine();
    out.close();
    outpop.close();
}



string Simulation::readInputs() {
    readSimControlFile(configLine);
    printSimulationParams();
    outputPath.erase(remove(outputPath.begin(), outputPath.end(), '\"'), outputPath.end());
    outputFile = outputPath + "/" + simName + ".csv";
    cout << "\n" << simName <<": outputFile:" << outputFile << endl;
    out.open(outputFile);
    if (!out.good()) {
        cout << "\n\n" << simName <<": " << "Can't open output file:" << outputFile << ". Exiting.\n\n";
        exit(1);
    }
    out << "day,infection,disease,age,previous_infections,vaccinated\n";
    outputPopFile = outputPath + "/" + simName + "_pop.csv";
    cout << "\n" << simName <<": outputPopFile:" << outputPopFile << endl;
    outpop.open(outputPopFile);
    if (!outpop.good()) {
        cout << "\n\n" << simName <<": " << "Can't open outputPop file:" << outputPopFile << ". Exiting.\n\n";
        exit(1);
    }
    outpop << "year,noinf_0008,inf_0008,noinf_0918,inf_0918,noinf_1999,inf_1999,nodis_0008,dis_0008,nodis_0918,dis_0918,nodis_1999,dis_1999,nohosp_0008,hosp_0008,nohosp_0918,hosp_0918,nohosp_1999,hosp_1999\n";
    // outpop << "year,age,seropos,population,doses\n";
    cout << "\n\n" << simName << ": Reading vaccine profile file ..." << endl;
    readVaccineProfileFile();
    RandomNumGenerator rgen(rSeed, huImm, emergeFactor, mlife, mrest, halflife);
    rGen = rgen;
    cout << "\n\n" << simName << ": Reading locations file ..." << endl;
    readLocationFile(locationFile);
    cout << "\n\n" << simName << ": Reading mortality file ..." << endl;
    readMortalityFile();
    cout << "\n\n" << simName << ": Reading trajectories file ..." << endl;
    readHumanFile(trajectoryFile);
    return simName;
}



void Simulation::simEngine() {
    resetPop();

    while (currentDay < numDays) {
        cout << currentDay << endl;

        humanDynamics();
        mosquitoDynamics();

        if(ceil((currentDay + 1) / 365) != ceil(currentDay / 365)){
            year++;
            updatePop();
            writePop();
            resetPop();
        }

        currentDay++;
    }
}



void Simulation::updatePop(){
    int count;
    int age;
    int age_09 = 9 * 365;
    int age_19 = 19 * 365;
    int noinf_0008 = 0, inf_0008 = 0, noinf_0918 = 0, inf_0918 = 0, noinf_1999 = 0, inf_1999 = 0;
    int nodis_0008 = 0, dis_0008 = 0, nodis_0918 = 0, dis_0918 = 0, nodis_1999 = 0, dis_1999 = 0;
    int nohosp_0008 = 0, hosp_0008 = 0, nohosp_0918 = 0, hosp_0918 = 0, nohosp_1999 = 0, hosp_1999 = 0;

    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
        age = itHum->second->getAge(currentDay);
        if(age < age_09){
            if(itHum->second->getRecentInf() == 0){
                noinf_0008++;
            } else {
                inf_0008++;
            }
            if(itHum->second->getRecentDis() == 0){
                nodis_0008++;
            } else {
                dis_0008++;
            }
            if(itHum->second->getRecentHosp() == 0){
                nohosp_0008++;
            } else {
                hosp_0008++;
            }
        } else if(age < age_19){
            if(itHum->second->getRecentInf() == 0){
                noinf_0918++;
            } else {
                inf_0918++;
            }
            if(itHum->second->getRecentDis() == 0){
                nodis_0918++;
            } else {
                dis_0918++;
            }
            if(itHum->second->getRecentHosp() == 0){
                nohosp_0918++;
            } else {
                hosp_0918++;
            }
        } else {
            if(itHum->second->getRecentInf() == 0){
                noinf_1999++;
            } else {
                inf_1999++;
            }
            if(itHum->second->getRecentDis() == 0){
                nodis_1999++;
            } else {
                dis_1999++;
            }
            if(itHum->second->getRecentHosp() == 0){
                nohosp_1999++;
            } else {
                hosp_1999++;
            }
        }
        itHum->second->resetRecent();
    }

    outpop << year << "," << 
        noinf_0008 << "," << inf_0008 << "," << noinf_0918 << "," << inf_0918 << "," << noinf_1999 << "," << inf_1999 << "," << 
        nodis_0008 << "," << dis_0008 << "," << nodis_0918 << "," << dis_0918  << "," << nodis_1999 << "," << dis_1999 << "," << 
        nohosp_0008 << "," << hosp_0008 << "," << nohosp_0918 << "," << hosp_0918 << "," << nohosp_1999  << "," << hosp_1999 << "\n";

    // for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
    //     itPop = seroage_pop.find(make_pair(floor(itHum->second->getAge(currentDay)/365),itHum->second->getPreviousInfections()));
    //     pop = seroage_pop.at(make_pair(floor(itHum->second->getAge(currentDay)/365),itHum->second->getPreviousInfections()));
    //     seroage_pop.erase(itPop);
    //     seroage_pop.insert(make_pair(
    //         make_pair(floor(itHum->second->getAge(currentDay)/365),itHum->second->getPreviousInfections()), pop + 1));
    // }
}



void Simulation::resetPop(){
    // seroage_pop.clear();
    // seroage_doses.clear();

    // for(unsigned a = 0; a < 100; a++){
    //     for(unsigned s = 0; s < 5; s++){
    //         seroage_pop.insert(make_pair(make_pair(a,s),0));
    //         seroage_doses.insert(make_pair(make_pair(a,s),0));
    //     }
    // }
}



void Simulation::writePop(){
    // outpop << year << "," << 9 << "," <<
    // double(seroage_pop.at(make_pair(9,0))) / 
    // double(
    //     seroage_pop.at(make_pair(9,0)) +
    //     seroage_pop.at(make_pair(9,1)) +
    //     seroage_pop.at(make_pair(9,2)) +
    //     seroage_pop.at(make_pair(9,3)) +
    //     seroage_pop.at(make_pair(9,4))) << "\n";

    // for(unsigned a = 0; a < 100; a++){
    //     for(unsigned s = 0; s < 5; s++){
    //         outpop << year << "," << a << "," << s << "," <<
    //         seroage_pop.at(make_pair(a,s)) << "," <<
    //         seroage_doses.at(make_pair(a,s)) << "\n";
    //     }
    // }
}



void Simulation::humanDynamics() {
    auto itDose = seroage_doses.begin();
    int diff, age, dose;
    bool vaxd = false;

    for (auto it = humans.begin(); it != humans.end(); ++it) {
        // daily mortality for humans by age
        if(rGen.getEventProbability() < mortalityHuman[floor(it->second->getAge(currentDay) / 365.0)])
            it->second->reincarnate(currentDay);

        // update temporary cross-immunity status if necessary
        if(currentDay == it->second->getImmEndDay())
            it->second->setImmunityTemp(false);

        // select movement trajectory for the day
        (it->second)->setTrajDay(rGen.getRandomNum(5));

        // simulate possible imported infection
        if(rGen.getEventProbability() < ForceOfImportation){
            int serotype = rGen.getRandomNum(4) + 1;
            if(!it->second->isImmune(serotype)){
                it->second->infection.reset(new Infection(
                    currentDay + 1, currentDay + 15, 0.0, serotype, it->second->getPreviousInfections() == 0, 0));
                it->second->updateImmunityPerm(serotype,true);
                it->second->setImmunityTemp(true);
                it->second->setImmStartDay(currentDay);
                it->second->setImmEndDay(currentDay + 14 + rGen.getHumanImmunity());
                it->second->updateRecent(1, 0, 0);
            }
        }

        // vaccinate if appropriate according to age
        // if(vaccinationStrategy == "catchup" || vaccinationStrategy == "nocatchup"){
            age = it->second->getAge(currentDay);
            if(rGen.getEventProbability() < .8 || it->second->isVaccinated()){
                if(age == 9 * 365){
                    it->second->vaccinate(&VE_pos, &VE_neg, 1.0, currentDay);
                    vaxd = true;
                } else if(it->second->isVaccinated() && age == 9 * 365 + 183){
                    it->second->vaccinate(&VE_pos, &VE_neg, 1.0, currentDay);
                    vaxd = true;
                } else if(it->second->isVaccinated() && age == 10 * 365){
                    it->second->vaccinate(&VE_pos, &VE_neg, 1.0, currentDay);
                    vaxd = true;
                }
            }            
        // }
        // if(vaccinationStrategy == "catchup"){
        //     if(currentDay <= 365){
        //         if(rGen.getEventProbability() < .8 / 365.0 || it->second->isVaccinated()){
        //             if(!it->second->isVaccinated() && age >= 3 * 365 && age < 8 * 365){
        //                 it->second->vaccinate(&VE_pos, &VE_neg, 1.0/3.0, currentDay);
        //                 vaxd = true;
        //             }
        //         }
        //     }
        //     if(currentDay <= 365 + 183 && currentDay > 182){
        //         if(it->second->isVaccinated() && age >= 3 * 365 + 183 && age < 8 * 365 + 183){
        //                 it->second->vaccinate(&VE_pos, &VE_neg, 2.0/3.0, currentDay);
        //                 vaxd = true;
        //         }
        //     }
        //     if(currentDay <= 365 * 2 && currentDay > 365){
        //         if(it->second->isVaccinated() && age >= 4 * 365 && age < 9 * 365){
        //                 it->second->vaccinate(&VE_pos, &VE_neg, 1.0, currentDay);
        //                 vaxd = true;
        //         }
        //     }
        // }

        // if(vaxd){
        //     itDose = seroage_doses.find(make_pair(floor(it->second->getAge(currentDay)/365.0),it->second->getPreviousInfections()));
        //     dose = seroage_pop.at(make_pair(floor(it->second->getAge(currentDay)/365.0),it->second->getPreviousInfections()));
        //     seroage_doses.erase(itDose);
        //     seroage_doses.insert(make_pair(
        //         make_pair(floor(it->second->getAge(currentDay)/365.0),it->second->getPreviousInfections()), dose + 1));
        //     vaxd = false;
        // }
    }
}



void Simulation::mosquitoDynamics() {
    generateMosquitoes();

    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
        if(it->second->infection != nullptr){
            if(currentDay == it->second->infection->getStartDay())
                it->second->infection->setInfectiousnessMosquito(mozInfectiousness);
        }
 
        // determine if the mosquito will bite and/or die today, and if so at what time
        double biteTime = double(numDays + 1), dieTime = double(numDays + 1);
 
        if(it->second->getBiteStartDay() <= double(currentDay + 1)){
            it->second->setState(Mosquito::MozState::BITE);
            biteTime = it->second->getBiteStartDay() - double(currentDay);
            if(biteTime < 0){
                biteTime = rGen.getEventProbability();
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
        if(biteTime < dieTime && biteTime <= 1.0){
            it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,currentDay,numDays,&out);

            if(dieTime < 1.0){
                auto it_temp = it;
                it++;
                mosquitoes.erase(it_temp);
                continue;
            }
        }
 
        // let the mosquito move if that happens today
        if(rGen.getEventProbability() < mozMoveProbability) {
            string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
            if(newLoc != "TOO_FAR_FROM_ANYWHERE") {
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
        mozCount = rGen.getMozEmerge(x.second->getMozzes());

        for(int i = 0; i < mozCount; i++){
            unique_ptr<Mosquito> moz(new Mosquito(
                mozID++, currentDay, double(currentDay) + rGen.getMozLifeSpan(), double(currentDay) + rGen.getMozRestDays(), x.first));
            mosquitoes.insert(make_pair(x.first, move(moz)));
        }
    }
}



void Simulation::setLocNeighborhood(double dist) {
    for (auto it1 = locations.begin(); it1 != locations.end(); ++it1) {
        auto it2 = it1;
        it2++;
        for (; it2 != locations.end(); ++it2) {
            if (it1->second->getDistanceFromLoc(*it2->second) <= dist) {
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
    numDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    outputPath = line;
    getline(infile, line, ',');
    locationFile = line;
    getline(infile, line, ',');
    mortalityFile = line;
    getline(infile, line, ',');
    trajectoryFile = line;
    getline(infile, line, ',');
    vaccineProfileFile = line;
    getline(infile, line, ',');
    FOI = strtod(line.c_str(),NULL);
    getline(infile, line, ',');
    ForceOfImportation = strtod(line.c_str(),NULL);
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
    mrest = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccinationStrategy = line;
}



void Simulation::readLocationFile(string locFile) {
    if (locFile.length() == 0) {
        cout << "\n" << simName <<": Locations file not specified! Exiting." << endl; 
        exit(1);
    }
    string line, locID, locType;
    double x, y, mozzes;

    ifstream infile(locFile);
    if (!infile.good()) {
        cout << "\n\n" << simName << ": Can't open file:" << locFile << ". Exiting.\n" << endl;
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

        unique_ptr<Location> location(new Location(locID, locType, x, y, mozzes));
        locations.insert(make_pair(locID, move(location)));

    }
    infile.close();
    cout << "\n" << simName << ": Done reading locations!" << endl;

}



void Simulation::readMortalityFile() {
    if (mortalityFile.length() == 0) {
        cout << "\n" << simName << ": Mortality file not specified! Exiting." << endl;
        exit(1);
    }
    string line;
    int age;
    double mort;

    ifstream infile(mortalityFile);
    if(!infile.good()){
        cout << "\n\n" << simName << ": Can't open file:" << mortalityFile << ". Exiting.\n" << endl;
        exit(1);
    }
    while(getline(infile, line, ',')){
        age = strtol(line.c_str(), NULL, 10);
        getline(infile, line, ',');
        mort = strtod(line.c_str(), NULL);

        mortalityHuman.insert(make_pair(age,mort));

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');
    }
    infile.close();
    cout << "\n" << simName << ": Done reading mortalities!" << endl;
}



void Simulation::readVaccineProfileFile() {
    if (vaccineProfileFile.length() == 0) {
        cout << "\n" << simName << ": Vaccine profile file not specified! Exiting." << endl;
        exit(1);
    }
    string line;
    unsigned sero;
    double vep;
    double ven;
    double hl;

    ifstream infile(vaccineProfileFile);
    if(!infile.good()){
        cout << "\n\n" << simName << ": Can't open file:" << vaccineProfileFile << ". Exiting.\n" << endl;
        exit(1);
    }
    while(getline(infile, line, ',')){
        sero = strtol(line.c_str(), NULL, 10);
        getline(infile, line, ',');
        vep = strtod(line.c_str(), NULL);
        getline(infile, line, ',');
        ven = strtod(line.c_str(), NULL);
        getline(infile, line, '\n');
        hl = strtod(line.c_str(), NULL);

        VE_pos.insert(make_pair(sero,vep));
        VE_neg.insert(make_pair(sero,ven));
        halflife.insert(make_pair(sero,hl * 365.0));
    }
    infile.close();
    cout << "\n" << simName << ": Done reading the vaccine profile!" << endl;
}



void Simulation::readHumanFile(string humanFile) {
    if (humanFile.length() == 0) {
        cout << "\n" << simName << ": Trajectories file not specified! Exiting." << endl;
        exit(1);
    }
    string line, houseID;
    int age;
    unsigned hMemID;
    double bodySize;
    char gen;

    ifstream infile(humanFile);
    if (!infile.good()) {
        cout << "\n\n" << simName << ": Can't open file:" << humanFile << ". Exiting.\n" << endl;
        exit(1);
    }
    while (getline(infile, line, ',')) {
        unique_ptr < vector < vector < pair<string, double >> >> trajectories(new vector < vector < pair<string, double >> >());

        for (int i = 0; i < 5; i++) {
            houseID = line;
            getline(infile, line, ',');
            hMemID = strtol(line.c_str(), NULL, 10);
            getline(infile, line, ',');
            gen = line[0];
            getline(infile, line, ',');
            age = strtol(line.c_str(), NULL, 10);
            getline(infile, line, ',');
            bodySize = strtod(line.c_str(), NULL);
            vector < pair<string, double >> path;
            getline(infile, line);
            stringstream ss;
            ss << line;
            while (getline(ss, line, ',')) {
                string hID = line;
                getline(ss, line, ',');
                double timeSpent = strtod(line.c_str(), NULL);
                path.push_back(make_pair(hID, timeSpent));
            }
            trajectories->push_back(move(path));
            if (i < 4)
                getline(infile, line, ',');
        }

        unique_ptr<Human> h(new Human(houseID, hMemID, age, bodySize, gen, trajectories, rGen, currentDay, FOI));

        std::set<std::string> locsVisited = h->getLocsVisited();
        for(std::set<std::string>::iterator itrSet = locsVisited.begin(); itrSet != locsVisited.end(); itrSet++)
            if(locations.find(*itrSet) != locations.end()){
                locations.find(*itrSet)->second->addHuman(h.get());
            }

        humans.insert(make_pair(houseID, move(h)));

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');
    }
    infile.close();
    cout << "\n" << simName << ": Done reading trajectories!" << endl;
}



void Simulation::printLocations() const {
    cout << "\n" << simName << ": Locations:" << endl;
    for (auto& x : locations) {
        cout << x.second->toString() << endl;
    }
}



void Simulation::printHumans() const {
    cout << "\n" << simName << ": Human:" << endl;
    for (auto& x : humans) {
        cout << x.second->toString() << endl;
    }
}



void Simulation::printMosquitoes() const {
    cout << "\n" << simName << ": Mosquitoes:" << endl;
    for (auto& x : mosquitoes) {
        cout << x.first << " " << x.second->toString() << endl;
    }
}



void Simulation::printSimulationParams() const {
    cout << "\n\n"<< simName <<": currentDay:" << currentDay << endl;
    cout << "\n"<< simName <<": numDays:" << numDays << endl; 
    cout << "\n"<< simName <<": trajectoryFile:" << trajectoryFile << endl;
    cout << "\n"<< simName <<": locationFile:" << locationFile << endl;
    cout << "\n"<< simName <<": seed:" << rSeed << endl;
    cout << "\n"<< simName <<": outputPath:" << outputPath << endl;
    cout << "\n"<< simName <<": mozInfectiousness:" << mozInfectiousness << endl;
    cout << "\n"<< simName <<": mozMoveProbability:" << mozMoveProbability << endl;
    cout << "\n"<< simName <<": " << rGen.toString() << endl;
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