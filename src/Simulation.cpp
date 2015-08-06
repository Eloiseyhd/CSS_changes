/* 
 * File:   Simulation.cpp
 * Author: amit
 * 
 * Created on August 25, 2014, 12:52 AM
 */

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
    //out.open("test1.csv");
    out << "day,type,house,member_no,age,sex,startday,bite_location\n";
    cout << "\n\n" << simName << ": Reading locations file ..." << endl;
    readLocationFile(locationFile);
    cout << "\n\n" << simName << ": Reading neighborhoods file ..." << endl;
    readNeighborhoodFile(neighborhoodFile);
    cout << "\n\n" << simName << ": Reading mortality file ..." << endl;
    readMortalityFile(mortalityFile);
    cout << "\n\n" << simName << ": Reading trajectories file ..." << endl;
    readHumanFile(trajectoryFile);
    cout << "\n\n" << simName << ": Reading initial infections file ..." << endl;
    readInitialInfectionsFile(initialInfectionsFile);
    return simName;
}

unsigned Simulation::setInitialInfection(double prop, unsigned infType) {
    unsigned i = 0;
    for (auto& h : humans) {
        if (rGen.getEventProbability() < prop) {
            if (h.second->infection == nullptr) {
                h.second->infection.reset(new Infection(0, rGen.intialInfDaysLeft(), 0, infType));
                i++;
            }
        }
    }
    return i;
}

void Simulation::simEngine() {
    while (currentDay < numDays) {
        //cout << "\n" << simName <<": Day:" << currentDay << endl;
        cout << currentDay << endl;
        humanDynamics();
        mosquitoDynamics();
        currentDay++;
    }
}



void Simulation::humanDynamics() {
    int diff;

    for (auto it = humans.begin(); it != humans.end(); ++it) {
        // daily mortality for humans by age
        if(rGen.getEventProbability() < mortalityHuman[floor(it->second->getAge(currentDay) / 365)])
            it->second->reincarnate(currentDay);

        // update infectiousness for the day if infected
        if (it->second->infection != nullptr) {
            auto& itInf = it->second->infection;
            diff = currentDay - itInf->getStartDay();
            if (diff >= 0 && diff < 10) {
                if (itInf->getInfectionType() == 1) {
                    itInf->setInfectiousness(dnv[1-1][diff]);
                } else if (itInf->getInfectionType() == 2) {
                    itInf->setInfectiousness(dnv[2-1][diff]);
                } else if (itInf->getInfectionType() == 3) {
                    itInf->setInfectiousness(dnv[3-1][diff]);
                } else if (itInf->getInfectionType() == 4) {
                    itInf->setInfectiousness(dnv[4-1][diff]);
                }
            } else if (diff == 10) {
                it->second->infection.reset(nullptr);
            }
        }

        // update temporary cross-immunity status if necessary
        if(it->second->isImmuneTemp() && currentDay == it->second->getImmEndDay())
            it->second->setImmunityTemp(false);

        // select movement trajectory for the day
        (it->second)->setTrajDay(rGen.getRandomNum(5));
    }
}



void Simulation::mosquitoDynamics() {
    generateMosquitoes();

    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
        if(it->second->infection != nullptr){
            if(currentDay == it->second->infection->getStartDay())
                it->second->infection->setInfectiousness(mozInfectiousness);
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



void Simulation::readInitialInfectionsFile(string infectionsFile) {
    if (infectionsFile.length() == 0) {
        cout << "\n" << simName <<": Initial infections file not specified! Exiting." << endl;
        exit(1);
    }
    string line, houseID;
    unsigned hMemID, serotype;
    std::multimap<std::string,std::unique_ptr<Human>>::iterator resident;

    ifstream infile(infectionsFile);
    if (!infile.good()) {
        cout << "\n\n"  << simName << ": Can't open file:" << infectionsFile << ". Exiting.\n" << endl;
        exit(1);
    }
    while (getline(infile, line, ',')) {
        houseID = line;
        getline(infile, line, ',');
        hMemID = strtol(line.c_str(), NULL, 10);
        getline(infile, line, '\n');
        serotype = strtol(line.c_str(), NULL, 10);

        resident = humans.equal_range(houseID).first;
        for(int i = 1; i < hMemID; i++)
            resident++;

        resident->second->infection.reset(new Infection(0, rGen.intialInfDaysLeft(), 0, serotype));
        resident->second->setImmunityPerm(serotype,true);
        resident->second->setImmunityTemp(true);
        resident->second->setImmStartDay(currentDay);
        resident->second->setImmEndDay(currentDay + 9 + rGen.getHumanImmunity());

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');

    }
    infile.close();
    cout << "\n" << simName << ": Done reading initial infections" <<endl;

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
    if (line == "") {
        random_device rd;
        rSeed = rd();
        //cout << "random seed: " << rSeed;
    } else {
        rSeed = strtol(line.c_str(), NULL, 10);
    }
    getline(infile, line, ',');
    numDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    outputPath = line;
    getline(infile, line, ',');
    locationFile = line;
    getline(infile, line, ',');
    neighborhoodFile = line;
    getline(infile, line, ',');
    mortalityFile = line;
    getline(infile, line, ',');
    trajectoryFile = line;
    getline(infile, line, ',');
    initialInfectionsFile = line;
    getline(infile, line, ',');
    FOI = strtod(line.c_str(),NULL);
    getline(infile, line, ',');
    unsigned hllo = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned hlhi = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    humanInfectionDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned huImm = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    emergeFactor = strtod(line.c_str(), NULL);    
    getline(infile, line, ',');
    double mlife = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    mozInfectiousness = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    unsigned mlho = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned mlhi = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    mozMoveProbability = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    unsigned mrestlo = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned mresthi = strtol(line.c_str(), NULL, 10);
    RandomNumGenerator rgen(rSeed, hllo, hlhi, huImm, emergeFactor, mlife, mlho, mlhi, mrestlo, mresthi);
    rGen = rgen;
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

void Simulation::readNeighborhoodFile(std::string nFile) {
    if (nFile.length() == 0) {
        cout << "\n" << simName << ": Neighborhood file not specified! Exiting." << endl;
        exit(1);
    }
    ifstream infile(nFile);
    if (!infile.good()) {
        cout << "\n\n" << simName << ": Can't open file:" << nFile << ". Exiting.\n" << endl;
        exit(1);
    }
    string line;
    getline(infile, line);
    while (getline(infile, line)) {
        stringstream ss;
        ss << line;
        getline(ss, line, ',');
        auto loc = locations.find(line);
        while (getline(ss, line, ',')) {
            loc->second->addCloseLoc(line);
        }
        while (infile.peek() == '\n')
            infile.ignore(1, '\n');
    }
    infile.close();
    cout << "\n" << simName << ": Done reading neighborhoods!" << endl;
}

void Simulation::readMortalityFile(std::string nFile) {
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

void Simulation::createNeighborhoodFile(std::string s1, std::string s2, std::string s3) {
    if (s2 == s3) {
        cout << "\n[LocationsFile] and [NeighborhoodFile] can't be same. Exiting.\n";
        exit(1);
    }
    double dist = strtod(s1.c_str(), NULL);
    cout << "\nReading locations file ... " << endl;
    readLocationFile(s2);
    cout << endl;
    cout << "\nCalculating neighborhoods. This may take few minutes ... " << endl;
    setLocNeighborhood(dist);
    cout << "Done" << endl;
    cout << "\nWriting neighborhood file ... " << endl;
    writeNeighborhoodFile(s3);
    cout << "Done" << endl;
}

void Simulation::writeNeighborhoodFile(string file) const {
    ofstream nOut;
    nOut.open(file);
    if (!nOut.good()) {
        cout << "\n" << simName << ": Can't open file:" << file << " . Exiting." << endl;
        exit(1);
    }
    nOut << "locationID,NeighborsID\n";
    for (auto& x : locations) {
        nOut << x.second->getLocID();
        x.second->writeNeighToFile(nOut);
        nOut << "\n";
    }
    nOut.close();
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
    cout << "\n"<< simName <<": neighborhoodFile:" << neighborhoodFile << endl;
    cout << "\n"<< simName <<": seed:" << rSeed << endl;
    cout << "\n"<< simName <<": outputPath:" << outputPath << endl;
    cout << "\n"<< simName <<": humanInfectionDays:" << humanInfectionDays << endl;
    cout << "\n"<< simName <<": mozInfectiousness:" << mozInfectiousness << endl;
    cout << "\n"<< simName <<": mozMoveProbability:" << mozMoveProbability << endl;
    cout << "\n"<< simName <<": " << rGen.toString() << endl;
}

Simulation::Simulation(string line) {
    currentDay = 0;
    configLine = line;
}

Simulation::Simulation() {
}

Simulation::Simulation(const Simulation & orig) {
}

Simulation::~Simulation() {
}