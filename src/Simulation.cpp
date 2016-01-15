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
    outvac.close();
}



string Simulation::readInputs() {
    readSimControlFile(configLine);
    outputPath.erase(remove(outputPath.begin(), outputPath.end(), '\"'), outputPath.end());

    // outputFile = outputPath + "/" + simName + ".csv";
    // out.open(outputFile);
    // if (!out.good()) {
    //     exit(1);
    // }
    // out << "day,infection,disease,age,previous_infections,vaccinated\n";
    outputVacFile = outputPath + "/" + simName + "_vac.csv";
    outputPopFile = outputPath + "/" + simName + "_pop.csv";
    outpop.open(outputPopFile);
    if (!outpop.good()) {
      //      printf("cannot create output file %s\n",outputPopFile.c_str());
        exit(1);
    }
    outpop << "year,seropos,seroneg\n";
    outvac << "year,totalpop,total9,total18,total19+,vac9,vac18,vac19+,disAge9Inf0,disAge9Inf1,disAge9Inf2,disAge9Inf3,disAge9Inf4,disAge18Inf0,disAge18Inf1,disAge18Inf2,disAge18Inf3,disAge18Inf4,disAge19Inf0,disAge19Inf1,disAge19Inf2,disAge19Inf3,disAge19Inf4,"<<
      "nonDisAge9Inf0,nonDisAge9Inf1,nonDisAge9Inf2,nonDisAge9Inf3,nonDisAge9Inf4,nonDisAge18Inf0,nonDisAge18Inf1,nonDisAge18Inf2,nonDisAge18Inf3,nonDisAge18Inf4,nonDisAge19Inf0,nonDisAge19Inf1,nonDisAge19Inf2,nonDisAge19Inf3,nonDisAge19Inf4\n";
    outvac.open(outputVacFile);
    if(!outvac.good()){
      exit(1);
    }

    RandomNumGenerator rgen(rSeed, huImm, emergeFactor, mlife, mrest, halflife);
    rGen = rgen;

    RandomNumGenerator rgen2(rSeedInf, huImm, emergeFactor, mlife, mrest, halflife);
    rGenInf = rgen2;
    readVaccineProfileFile();
    readLocationFile(locationFile);
    readHumanFile(trajectoryFile);

    return simName;
}



void Simulation::simEngine() {
  //  if(vaccinationFlag == true){printf("Vaccination is activated for day %u\n",vaccineDay);}
  while(currentDay < numDays){        
    //    printf("simEngine Entered current day %d, max days %d\n",currentDay, numDays);
    //if(vaccineDay == currentDay && vaccinationFlag == true){printf("vaccination beings at day: %u\n",currentDay);}
    //    double r = rGen.getEventProbability();
    //    printf("day %d randomnumber r = %.4f\n",currentDay,r);
    //    printf("day %d to humdynamics randomnumber r = %.4f\n",currentDay,r);

    humanDynamics();
    //    r = rGen.getEventProbability();
    //    printf("day %d to mosdynamics randomnumber r = %.4f\n",currentDay,r);

    mosquitoDynamics();
    //    r = rGen.getEventProbability();
    //    printf("day %d after mosdynamics randomnumber r = %.4f\n", currentDay,r);
    if(ceil((currentDay + 1) / 365) != ceil(currentDay / 365)){
      year++;
	updatePop();
    }
    
    currentDay++;
  }
}

struct popStats{
  int prevInf[3][5];
  int prevInfVac[3][5];
};

void Simulation::updatePop(){
  //  printf("update pop year: %u\n",year);
    int count;
    int age;
    int age_09 = 9 * 365;
    int age_10 = 10 * 365;
    int seropos = 0, seroneg = 0;
    int vacpop[3];
    int totalpop[3];
    popStats nonDis;
    popStats dis;
      for(int i = 0;i < 3; i++){
	totalpop[i] = 0;
	vacpop[i] = 0;
	for(int j = 0; j < 5;j++){
	  nonDis.prevInf[i][j] = 0;
	  nonDis.prevInfVac[i][j] = 0;
	  dis.prevInf[i][j] = 0;
	  dis.prevInfVac[i][j] = 0;
	}
      }
    // int age_19 = 19 * 365;
    // int noinf_0008 = 0, inf_0008 = 0, noinf_0918 = 0, inf_0918 = 0, noinf_1999 = 0, inf_1999 = 0;
    // int nodis_0008 = 0, dis_0008 = 0, nodis_0918 = 0, dis_0918 = 0, nodis_1999 = 0, dis_1999 = 0;
    // int nohosp_0008 = 0, hosp_0008 = 0, nohosp_0918 = 0, hosp_0918 = 0, nohosp_1999 = 0, hosp_1999 = 0;

    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
        itHum->second->updateAttractiveness(currentDay);
        age = itHum->second->getAge(currentDay);

        if(age >= age_09 && age < age_10){
            if(itHum->second->getPreviousInfections()){
                seropos++;
            } else {
                seroneg++;
            }
            // if(itHum->second->getRecentInf() == 0){
            //     noinf_0008++;
            // } else {
            //     inf_0008++;
            // }
            // if(itHum->second->getRecentDis() == 0){
            //     nodis_0008++;
            // } else {
            //     dis_0008++;
            // }
            // if(itHum->second->getRecentHosp() == 0){
            //     nohosp_0008++;
            // } else {
            //     hosp_0008++;
            // }
        // } else if(age < age_19){
            // if(itHum->second->getRecentInf() == 0){
            //     noinf_0918++;
            // } else {
            //     inf_0918++;
            // }
            // if(itHum->second->getRecentDis() == 0){
            //     nodis_0918++;
            // } else {
            //     dis_0918++;
            // }
            // if(itHum->second->getRecentHosp() == 0){
            //     nohosp_0918++;
            // } else {
            //     hosp_0918++;
            // }
        // } else {
            // if(itHum->second->getRecentInf() == 0){
            //     noinf_1999++;
            // } else {
            //     inf_1999++;
            // }
            // if(itHum->second->getRecentDis() == 0){
            //     nodis_1999++;
            // } else {
            //     dis_1999++;
            // }
            // if(itHum->second->getRecentHosp() == 0){
            //     nohosp_1999++;
            // } else {
            //     hosp_1999++;
            // }
        }
	/* Three age groups 
	   0 -> 0 - 8
	   1 -> 9 - 18
	   2 -> 19+
	*/
	int ageGroup = 0;
	if(age < 9 * 365){
	  ageGroup = 0;
	}else if (age < 19 * 365){
	  ageGroup = 1;
	}else{
	  ageGroup = 2;
	}

	if(itHum->second->isVaccinated()){
	  vacpop[ageGroup]++;
	  totalpop[ageGroup]++;
	  if(itHum->second->getRecentDis()){
	    dis.prevInfVac[ageGroup][itHum->second->getPreviousInfections()]++;
	  }else{	  
	    nonDis.prevInfVac[ageGroup][itHum->second->getPreviousInfections()]++;
	  }
	}else{
	  totalpop[ageGroup]++;
	  if(itHum->second->getRecentDis()){
	    dis.prevInf[ageGroup][itHum->second->getPreviousInfections()]++;
	  }else{
	    nonDis.prevInf[ageGroup][itHum->second->getPreviousInfections()]++;
	  }
	}
        itHum->second->resetRecent();
    }

    outpop << year << "," << 
        seropos << "," << seroneg << "\n";
        // noinf_0008 << "," << inf_0008 << "," << noinf_0918 << "," << inf_0918 << "," << noinf_1999 << "," << inf_1999 << "," << 
        // nodis_0008 << "," << dis_0008 << "," << nodis_0918 << "," << dis_0918  << "," << nodis_1999 << "," << dis_1999 << "," << 
        // nohosp_0008 << "," << hosp_0008 << "," << nohosp_0918 << "," << hosp_0918 << "," << nohosp_1999  << "," << hosp_1999 << "\n";
    int sumpop = totalpop[0] + totalpop[1] + totalpop[2];
    outvac << year << "," <<
      sumpop<<","<< totalpop[0]<<","<<totalpop[1]<<","<<totalpop[2]<<","<<vacpop[0]<<","<<vacpop[1]<<","<< vacpop[2] << ","<<
      dis.prevInfVac[0][0] << "," << dis.prevInfVac[0][1] << "," << dis.prevInfVac[0][2] << "," << dis.prevInfVac[0][3] << ","  << dis.prevInfVac[0][4] << "," <<
      dis.prevInfVac[1][0] << "," << dis.prevInfVac[1][1] << "," << dis.prevInfVac[1][2] << "," << dis.prevInfVac[1][3] << ","  << dis.prevInfVac[1][4] << "," <<
      dis.prevInfVac[2][0] << "," << dis.prevInfVac[2][1] << "," << dis.prevInfVac[2][2] << "," << dis.prevInfVac[2][3] << ","  << dis.prevInfVac[2][4] << "," <<
      nonDis.prevInfVac[0][0] << "," << nonDis.prevInfVac[0][1] << "," << nonDis.prevInfVac[0][2] << "," << nonDis.prevInfVac[0][3] << ","  << nonDis.prevInfVac[0][4] << "," <<
      nonDis.prevInfVac[1][0] << "," << nonDis.prevInfVac[1][1] << "," << nonDis.prevInfVac[1][2] << "," << nonDis.prevInfVac[1][3] << ","  << nonDis.prevInfVac[1][4] << "," <<
      nonDis.prevInfVac[2][0] << "," << nonDis.prevInfVac[2][1] << "," << nonDis.prevInfVac[2][2] << "," << nonDis.prevInfVac[2][3] << ","  << nonDis.prevInfVac[2][4] << "\n";
}



void Simulation::humanDynamics() {
    int diff, age, dose;
    bool vaxd = false;
    //    printf("human dynamics entered\n");
    for (auto it = humans.begin(); it != humans.end(); ++it) {
        // daily mortality for humans by age
        if(rGen.getEventProbability() < (deathRate * it->second->getAge(currentDay)))
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
        if(rGen.getEventProbability() < ForceOfImportation){
            int serotype = rGen.getRandomNum(4) + 1;
            if(!it->second->isImmune(serotype)){
                it->second->infection.reset(new Infection(
                    currentDay + 1, currentDay + 15, 0.0, serotype, it->second->getPreviousInfections() == 0, 0));
                it->second->updateImmunityPerm(serotype,true);
                it->second->setImmunityTemp(true);
                it->second->setImmStartDay(currentDay);
                it->second->setImmEndDay(currentDay + 14 + rGenInf.getHumanImmunity());
                it->second->updateRecent(1, 0, 0);
		//		printf("successful importation to human: %s - %d with serotype %d\n", it->second->getHouseID().c_str(),it->second->getHouseMemNum(),serotype);
            }
        }
	if(vaccineDay <= currentDay && vaccinationFlag == true){
	  // vaccinate if appropriate according to age
	  // if(vaccinationStrategy == "catchup" || vaccinationStrategy == "nocatchup"){
	  age = it->second->getAge(currentDay);
	  if(rGenInf.getEventProbability() < .8 || it->second->isVaccinated()){
	    if(age == 9 * 365){
	      it->second->vaccinate(&VE_pos, &VE_neg,rGenInf, 1.0, currentDay);
	      vaxd = true;
	    } else if(it->second->isVaccinated() && age == 9 * 365 + 183){
	      it->second->vaccinate(&VE_pos, &VE_neg,rGenInf, 1.0, currentDay);
	      vaxd = true;
	    } else if(it->second->isVaccinated() && age == 10 * 365){
	      it->second->vaccinate(&VE_pos, &VE_neg,rGenInf, 1.0, currentDay);
	      vaxd = true;
	    }
	  } 
	}           
	// }
	/* if(vaccinationStrategy == "catchup"){
            if(currentDay <= 365){
                 if(rGen.getEventProbability() < .8 / 365.0 || it->second->isVaccinated()){
                     if(!it->second->isVaccinated() && age >= 3 * 365 && age < 8 * 365){
                         it->second->vaccinate(&VE_pos, &VE_neg, 1.0/3.0, currentDay);
                         vaxd = true;
                     }
                 }
             }
             if(currentDay <= 365 + 183 && currentDay > 182){
	if(it->second->isVaccinated() && age >= 3 * 365 + 183 && age < 8 * 365 + 183){
                         it->second->vaccinate(&VE_pos, &VE_neg, 2.0/3.0, currentDay);
                         vaxd = true;
                 }
             }
             if(currentDay <= 365 * 2 && currentDay > 365){
                 if(it->second->isVaccinated() && age >= 4 * 365 && age < 9 * 365){
                         it->second->vaccinate(&VE_pos, &VE_neg, 1.0, currentDay);
                         vaxd = true;
                 }
             }
	*/
    }
   //    printf("human dynamics finished\n");
}



void Simulation::mosquitoDynamics() {

    generateMosquitoes();
    int totalmosquitoes =0;
    int biterng = 0;
    int moverng = 0;
    for(auto it = mosquitoes.begin(); it != mosquitoes.end();){
      totalmosquitoes++;
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
	      biterng++;
	      //	      printf("day %u %.4f bitetime mosquito %u\n", currentDay,rGen.getEventProbability(),it-second->getMosquitoID());
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
	//double biter = rGen.getEventProbability();
	//	printf("day %u before takebite rng %.4f for mosquito %u dieTime %.4f biteTime %.4f\n",currentDay,biter,it->second->getMosquitoID(),dieTime,biteTime);
        // if the mosquito bites first, then let it bite and then see about dying
        if(biteTime < dieTime && biteTime <= 1.0){
	  //	  printf("day %u  mosquito %u takes bite\n",currentDay,it->second->getMosquitoID());
	  it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,currentDay,numDays,&out);
            if(dieTime < 1.0){
                auto it_temp = it;
                it++;
                mosquitoes.erase(it_temp);
                continue;
            }
        }
	//	biter = rGen.getEventProbability();
	//	printf("day %u after takebite rng %.4f for mosquito %u\n",currentDay,biter,it->second->getMosquitoID());
        // let the mosquito move if that happens today 
	double moveProb = rGen.getEventProbability();
	moverng++;
	//	printf("Mosquito: %u in location %s move prob %.4f vs %.4f\n", it->second->getMosquitoID(),it->first.c_str(),moveProb,mozMoveProbability);
        if(moveProb < mozMoveProbability) {
            string newLoc = locations.find(it->first)->second->getRandomCloseLoc(rGen);
	    //	    printf("Mosquito: %u possibly moves to new location: %s\n",it->second->getMosquitoID(),newLoc.c_str());
            if(newLoc != "TOO_FAR_FROM_ANYWHERE") {
                it->second->setLocation(newLoc);
		//		printf("Mosquito: %u move to new location: %s\n",it->second->getMosquitoID(),newLoc.c_str());
                mosquitoes.insert(make_pair(newLoc, move(it->second)));
                auto it_temp = it;
                it++;
                mosquitoes.erase(it_temp);
		moverng++;
            }
            else{
                it++;
            }
        }
        else{
            it++;
        }
    }
    //    printf("day %u total mosquitoes %d biterng %d moverng %d\n",currentDay,totalmosquitoes,biterng,moverng);
}



void Simulation::generateMosquitoes(){
    int mozCount = 0;
    int rngcount = 0;
    for(auto& x : locations){
        mozCount = rGen.getMozEmerge(x.second->getMozzes());
	rngcount++;
        for(int i = 0; i < mozCount; i++){
            unique_ptr<Mosquito> moz(new Mosquito(
                mozID++, currentDay, double(currentDay) + rGen.getMozLifeSpan(), double(currentDay) + rGen.getMozRestDays(), x.first));
	    rngcount+=2;
            mosquitoes.insert(make_pair(x.first, move(moz)));
        }
    }
    //    printf("day %u rng %d times\n",currentDay,rngcount);
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
    rSeedInf = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    numDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccineDay = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccinationFlag = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    outputPath = line;
    getline(infile, line, ',');
    locationFile = line;
    getline(infile, line, ',');
    trajectoryFile = line;
    getline(infile, line, ',');
    vaccineProfileFile = line;
    getline(infile, line, ',');
    deathRate = strtod(line.c_str(), NULL);    
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
    mrest = strtod(line.c_str(), NULL);
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

        unique_ptr<Location> location(new Location(locID, locType, x, y, mozzes));
        locations.insert(make_pair(locID, move(location)));

    }
    infile.close();
}



void Simulation::readVaccineProfileFile() {
    if (vaccineProfileFile.length() == 0) {
        exit(1);
    }
    string line;
    unsigned sero;
    double vep;
    double ven;
    double hl;

    ifstream infile(vaccineProfileFile);
    if(!infile.good()){
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
}



void Simulation::readHumanFile(string humanFile) {
    if (humanFile.length() == 0) {
        exit(1);
    }
    string line, houseID;
    int age;
    unsigned hMemID;
    char gen;

    ifstream infile(humanFile);
    if (!infile.good()) {
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

        unique_ptr<Human> h(new Human(houseID, hMemID, age, gen, trajectories, rGen, currentDay));

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
