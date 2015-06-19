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

    // cout << "\n\n" << simName <<": Setting initial infection in humans ..." << endl;
    // if ((dnv1IniProp + dnv1IniProp + dnv1IniProp) > 1) {
    //     cout << "\n" << simName <<": Sum of proportions greater than 1. Exiting" << endl;
    // }
    unsigned numHu = humans.size();
    // unsigned numIniInfectedDnv1 = setInitialInfection(dnv1IniProp, Infection::InfType::DENV1);
    // cout << "\n" << simName <<": " << numIniInfectedDnv1 << " humans infected with DNV1" << endl;
    // //cout<<"\naddnv1:"<<dnv1IniProp*numHu/numHu << " " << dnv1IniProp;
    
    // double adjustedDnv2IniProp = dnv2IniProp * numHu / (numHu - numIniInfectedDnv1);
    // unsigned numIniInfectedDnv2 = setInitialInfection(adjustedDnv2IniProp, Infection::InfType::DENV2);
    // cout << "\n" << simName <<": "<< numIniInfectedDnv2 << " humans infected with DNV2" << endl;
    // //cout<<"\nadjdnv2: "<<adjustedDnv2IniProp;
    
    // double adjustedDnv3IniProp = dnv3IniProp * numHu / (numHu - numIniInfectedDnv1 - numIniInfectedDnv2);
    // unsigned numIniInfectedDnv3 = setInitialInfection(adjustedDnv3IniProp, Infection::InfType::DENV3);
    // cout << "\n" << simName <<": " << numIniInfectedDnv3 << " humans infected with DNV3" << endl;
    // //cout<<"\nadjdnv3: "<<adjustedDnv3IniProp;

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
    cout << "\n\n" << simName << ": Reading trajectories file ..." << endl;
    readHumanFile(trajectoryFile);
    cout << "\n\n" << simName << ": Reading emergence file ..." << endl;
    readMosquitoEmergenceFile(mozEmergenceFile);
    cout << "\n\n" << simName << ": Reading initial infections file ..." << endl;
    readInitialInfectionsFile(initialInfectionsFile);
    return simName;
}

unsigned Simulation::setInitialInfection(double prop, Infection::InfType infType) {
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
        //for (auto itInf = it->second->infections->begin(); itInf != it->second->infections->end();) {
        if (it->second->infection != nullptr) {
            auto& itInf = it->second->infection;
            diff = currentDay - itInf->getStartDay();
            if (diff >= 0 && diff < 10) {
                if (itInf->getInfectionType() == Infection::InfType::DENV1) {
                    itInf->setInfectiousness(dnv[Infection::InfType::DENV1][diff]);
                } else if (itInf->getInfectionType() == Infection::InfType::DENV2) {
                    itInf->setInfectiousness(dnv[Infection::InfType::DENV2][diff]);
                } else if (itInf->getInfectionType() == Infection::InfType::DENV3) {
                    itInf->setInfectiousness(dnv[Infection::InfType::DENV3][diff]);
                } else if (diff == 10) {
                    it->second->infection.reset(nullptr);
                }
            }
        }

        (it->second)->setTrajDay(rGen.getRandomNum(5));
    }
}

void Simulation::humanVisit(std::unique_ptr<Human>& human, string location, double t) {
    auto range = mosquitoes.equal_range(location);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getState() == Mosquito::MozState::BITE) {
            attemptBite(human, it->second, t, location);
        }
    }
}

void Simulation::attemptBite(std::unique_ptr<Human>& human, std::unique_ptr<Mosquito>& mosquito, double t, string location) {
    //cout<<"\n\nattempt bite:\n";
    //cout << human->toString() <<"\n";
    //cout << mosquito->toString() <<"\n";

    //cout<<"\n\nhuman not immune:\n";
    if (rGen.getEventProbability() < t * biteProbablity) { // check

        mosquito->setState(Mosquito::MozState::REST);
        mosquito->setBiteStartDay(currentDay + rGen.getMozRestDays());

        if (human->infection != nullptr && mosquito->infection == nullptr) {
            auto& inf = human->infection;
            if (rGen.getEventProbability() < inf->getInfectiousness()) {
                int sday = currentDay + rGen.getMozLatencyDays();
                int eday = numDays + 1;
                mosquito->infection.reset(new Infection(sday, eday, 0, inf->getInfectionType()));
            }

        }
        if (mosquito->infection != nullptr && human->infection == nullptr && !human->isImmune()) {
            auto& inf = mosquito->infection;

            if (rGen.getEventProbability() < inf->getInfectiousness()) {
                int sday = currentDay + rGen.getHuLatencyDays();
                int eday = sday + 9;
                human->infection.reset(new Infection(sday, eday, 0, inf->getInfectionType()));
                human->setImmunity(true);
                human->setImmStartDay(currentDay);
                human->setImmEndDay(currentDay + 9 + rGen.getHumanImmunity());
                out << currentDay << "," << inf->getInfectionType() << "," << human->getHouseID() << "," << human->getHouseMemNum();                        
                out << "," << human->getAge(currentDay) << "," << human->getGender() << "," << sday << "," << location << "\n";
                //cout << currentDay << "," << inf->getInfectionType() << "," << human->getHouseID() << "," << human->getHouseMemNum();                        
                //cout << "," << human->getAge(currentDay) << "," << human->getGender() << "," << sday << "," << location << "\n";
            }
        }
    }
    //cout<<"\nafter:";
    //cout << human->toString()<<"\n";
    //cout << mosquito->toString()<<"\n";
    //if (mosquito.)
    //cout << "\n bite occurs";
}

void Simulation::mosquitoDynamics() {
    generateMosquitoes();
    //printMosquitoes();
    for (auto it = mosquitoes.begin(); it != mosquitoes.end();) {
        //cout << "\n" << it->second->toString() << endl;

        // if this is the first day of a mosquito's infectiousness,
        // set the value of its infectiousness, which will remain until death
        if (it->second->infection != nullptr) {
            if (currentDay == it->second->infection->getStartDay())
                it->second->infection->setInfectiousness(mozInfectiousness);
        }

        // if the mosquito is supposed to enter bite mode today, make that happen
        if (currentDay >= it->second->getBiteStartDay())
            it->second->setState(Mosquito::MozState::BITE);


// SWITCH STATEMENT HERE TO HANDLE VARIABLE ORDER OF EVENTS


        double biteTime = rGen.getEventProbability();

        // get rid of mosquitoes that die today
        if(currentDay == it->second->getDDay()) {
            auto it_temp = it;
            it++;
            mosquitoes.erase(it_temp);
            continue;
        }

        // take a bite if that happens today
        if (it->second->getState() == Mosquito::MozState::BITE)
            it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,currentDay,numDays,&out);

        // let the mosquito move if that happens today
        if (rGen.getEventProbability() < mozMoveProbability) {
            string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
            if (newLoc != "TOO_FAR_FROM_ANYWHERE") {
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
    // for (auto it = mosquitoes.begin(); it != mosquitoes.end();) {
    //     if (it->second->getFly()) {
    //         //cout << "\n" << it->second->getMID() << " moving";
    //         it->second->setFly(false);
    //         string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
    //         if (newLoc != "TOO_FAR_FROM_ANYWHERE") {
    //             mosquitoes.insert(make_pair(newLoc, move(it->second)));
    //             it = mosquitoes.erase(it);
    //         } else ++it;
    //     } else ++it;
    // }
}

void Simulation::generateMosquitoes() {
    for (auto& x : locations) {
        auto loc = mEmergence.find(x.first);
        if (loc != mEmergence.end()) {
            if (currentDay > loc->second.size()) {
                cout << "\n\n" << simName <<": " << x.first << ": Not enough mosquito emergence data. Exiting...\n";
                exit(1);
            }
            for (int i = 0; i < loc->second[currentDay]; i++) {
                unique_ptr<Mosquito> moz(new Mosquito(mozID++, currentDay, currentDay + rGen.getMozLifeSpan(), x.first));
                mosquitoes.insert(make_pair(x.first, move(moz)));
                //mosquitoes.push_back(move(moz));
            }
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

        switch(serotype){
            case 1:
                resident->second->infection.reset(new Infection(0, rGen.intialInfDaysLeft(), 0, Infection::InfType::DENV1));
                break;
            case 2:
                resident->second->infection.reset(new Infection(0, rGen.intialInfDaysLeft(), 0, Infection::InfType::DENV2));
                break;
            case 3:
                resident->second->infection.reset(new Infection(0, rGen.intialInfDaysLeft(), 0, Infection::InfType::DENV3));
                break;
        }

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');

    }
    infile.close();
    cout << "\n" << simName << ": Done reading initial infections" <<endl;

}

void Simulation::readMosquitoEmergenceFile(string mozFile) {
    if (mozFile.length() == 0) {
        cout << "\n" << simName <<": Mosquito emergence file not specified! Exiting." << endl;
        exit(1);
    }

    string line, locID;
    unsigned char num;

    ifstream infile(mozFile);
    if (!infile.good()) {
        cout << "\n\n"  << simName << ": Can't open file:" << mozFile << ". Exiting.\n" << endl;
        exit(1);
    }
    getline(infile, line);
    while (getline(infile, line, ',')) {
        line.erase(remove(line.begin(), line.end(), '\"'), line.end());
        locID = line;
        vector<unsigned short> em;
        getline(infile, line);
        stringstream ss;
        ss << line;
        while (getline(ss, line, ',')) {
            num = strtol(line.c_str(), NULL, 10);
            em.push_back(num);
            //cout << num;
        }
        mEmergence.insert(make_pair(locID, em));
        while (infile.peek() == '\n')
            infile.ignore(1, '\n');

    }
    infile.close();
    cout << "\n" << simName << ": Done reading mosquito emergence" <<endl;

}

void Simulation::printMEmergence() const {
    cout << "\n" << simName << ": Mosquito Emergence:" <<endl;
    for (auto& x : mEmergence) {
        cout << "\n" << x.first << " ";
        for (auto& em : x.second) {
            cout << em << " ";
        }
    }
    cout << endl;
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
    mozEmergenceFile = line;
    getline(infile, line, ',');
    locationFile = line;
    getline(infile, line, ',');
    neighborhoodFile = line;
    getline(infile, line, ',');
    trajectoryFile = line;
    getline(infile, line, ',');
    initialInfectionsFile = line;
    getline(infile, line, ',');
    unsigned hllo = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned hlhi = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    humanInfectionDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned huImm = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    biteProbablity = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    unsigned mlifelo = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    unsigned mlifehi = strtol(line.c_str(), NULL, 10);
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
    RandomNumGenerator rgen(rSeed, hllo, hlhi, huImm, mlifelo, mlifehi, mlho, mlhi, mrestlo, mresthi);
    rGen = rgen;
}

void Simulation::readLocationFile(string locFile) {
    if (locFile.length() == 0) {
        cout << "\n" << simName <<": Locations file not specified! Exiting." << endl; 
        exit(1);
    }
    string line, locID, locType;
    double x, y;

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
        getline(infile, line);
        locID = line;

        while (infile.peek() == '\n')
            infile.ignore(1, '\n');

        unique_ptr<Location> location(new Location(locID, locType, x, y));
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
            //unique_ptr<vector<pair<string,double>>> path(new vector<pair<string,double>>());
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
            //cout << "\n" << "housID:" << houseID << " hMemID:" << hMemID << " gen:" << gen << " age:" << age << " bSize:" << bodySize;
            if (i < 4)
                getline(infile, line, ',');
        }

        unique_ptr<Human> h(new Human(houseID, hMemID, age, bodySize, gen, trajectories));

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
    cout << "\n"<< simName <<": mozEmergenceFile:" << mozEmergenceFile << endl;
    cout << "\n"<< simName <<": locationFile:" << locationFile << endl;
    cout << "\n"<< simName <<": neighborhoodFile:" << neighborhoodFile << endl;
    //cout << "\n"<< simName <<": simName:" << simName << endl;
    cout << "\n"<< simName <<": seed:" << rSeed << endl;
    cout << "\n"<< simName <<": outputPath:" << outputPath << endl;
    cout << "\n"<< simName <<": humanInfectionDays:" << humanInfectionDays << endl;
    cout << "\n"<< simName <<": biteProbablity:" << biteProbablity << endl;
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