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

    // outputFile = outputPath + "/" + simName + ".csv";
    // out.open(outputFile);
    // if (!out.good()) {
    //     exit(1);
    // }
    // out << "day,infection,disease,age,previous_infections,vaccinated\n";

    outputPopFile = outputPath + "/" + simName + "_pop.csv";
    outputPrevacFile = outputPath + "/" + simName + "_prevac.csv";
    outpop.open(outputPopFile);
    if (!outpop.good()) {
      //      printf("cannot create output file %s\n",outputPopFile.c_str());
        exit(1);
    }
    outpop << "year,seropos_09,seroneg_09";
    outpop << ",noinf_0008,inf_0008,noinf_0918,inf_0918,noinf_1999,inf_1999"; 
    outpop << ",nodis_0008,dis_0008,nodis_0918,dis_0918,nodis_1999,dis_1999"; 
    outpop << ",nohosp_0008,hosp_0008,nohosp_0918,hosp_0918,nohosp_1999,hosp_1999";
    outpop << ",seropos_vac_coh1,seropos_vac_inf_coh1,seropos_vac_dis_coh1,seropos_vac_hosp_coh1";
    outpop << ",seropos_novac_coh1,seropos_novac_inf_coh1,seropos_novac_dis_coh1,seropos_novac_hosp_coh1";
    outpop << ",seroneg_vac_coh1,seroneg_vac_inf_coh1,seroneg_vac_dis_coh1,seroneg_vac_hosp_coh1";
    outpop << ",seroneg_novac_coh1,seroneg_novac_inf_coh1,seroneg_novac_dis_coh1,seroneg_novac_hosp_coh1\n";

    outprevac.open(outputPrevacFile);
    if(!outprevac.good()){
      printf("cannot create output file %s\n",outputPrevacFile.c_str());
      exit(1);
    }
    outprevac << "age,seropos,seroneg\n";

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
    //    if(vaccineDay == currentDay && vaccinationFlag == true){printf("vaccination beings at day: %u\n",currentDay);}
    double r = rGen.getEventProbability();
    printf("day %d to humdynamics randomnumber r = %.4f\n",currentDay,r);
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



void Simulation::updatePop(){
  //  printf("update pop year: %u\n",year);
    int count;
    int age;
    int age_09 = 9 * 365;
    int age_10 = 10 * 365;
    int age_19 = 19 * 365;
    int seropos_09 = 0, seroneg_09 = 0;
    int noinf_0008 = 0, inf_0008 = 0, noinf_0918 = 0, inf_0918 = 0, noinf_1999 = 0, inf_1999 = 0;
    int nodis_0008 = 0, dis_0008 = 0, nodis_0918 = 0, dis_0918 = 0, nodis_1999 = 0, dis_1999 = 0;
    int nohosp_0008 = 0, hosp_0008 = 0, nohosp_0918 = 0, hosp_0918 = 0, nohosp_1999 = 0, hosp_1999 = 0;
    int seropos_vac_coh1 = 0, seropos_novac_coh1 = 0, seroneg_vac_coh1 = 0, seroneg_novac_coh1 = 0;
    int seropos_vac_inf_coh1 = 0, seropos_vac_dis_coh1 = 0, seropos_vac_hosp_coh1 = 0;
    int seropos_novac_inf_coh1 = 0, seropos_novac_dis_coh1 = 0, seropos_novac_hosp_coh1 = 0;
    int seroneg_vac_inf_coh1 = 0, seroneg_vac_dis_coh1 = 0, seroneg_vac_hosp_coh1 = 0;
    int seroneg_novac_inf_coh1 = 0, seroneg_novac_dis_coh1 = 0, seroneg_novac_hosp_coh1 = 0;
    /*int inf[100], dis[100],hosp[100];
      int popByAge[100];
      for(int i = 0;i < 100;i++){
      inf[i] = 0;
      dis[i] = 0;
      hosp[i] = 0;
      popByAge[i] = 0;
    }*/
    for(auto itHum = humans.begin(); itHum != humans.end(); itHum++){
        itHum->second->updateAttractiveness(currentDay);
        age = itHum->second->getAge(currentDay);

        if(age >= age_09 && age < age_10){
            if(itHum->second->getPreviousInfections() > 0){
                seropos_09++;
            } else {
                seroneg_09++;
            }
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
	if(itHum->second->getCohort() == 1){
	  if(itHum->second->getSeroStatusAtVaccination()){
	    if(itHum->second->isVaccinated()){
	      seropos_vac_coh1++;
	      if(itHum->second->getRecentInf() > 0){
		seropos_vac_inf_coh1++;
	      }
	      if(itHum->second->getRecentDis() > 0){
		seropos_vac_dis_coh1++;
		if(itHum->second->getRecentHosp() > 0){
		  seropos_vac_hosp_coh1++;
		}
	      }
	    }else{
	      seropos_novac_coh1++;
	      if(itHum->second->getRecentInf() > 0){
		seropos_novac_inf_coh1++;
	      }
	      if(itHum->second->getRecentDis() > 0){
		seropos_novac_dis_coh1++;
		if(itHum->second->getRecentHosp() > 0){
		  seropos_novac_hosp_coh1++;
		}
	      }
	    }
	  }else{
	    if(itHum->second->isVaccinated()){
	      seroneg_vac_coh1++;
	      if(itHum->second->getRecentInf() > 0){
		seroneg_vac_inf_coh1++;
	      }
	      if(itHum->second->getRecentDis() > 0){
		seroneg_vac_dis_coh1++;
		if(itHum->second->getRecentHosp() > 0){
		  seroneg_vac_hosp_coh1++;
		}
	      }
	    }else{
	      seroneg_novac_coh1++;
	      if(itHum->second->getRecentInf() > 0){
		seroneg_novac_inf_coh1++;
	      }
	      if(itHum->second->getRecentDis() > 0){
		seroneg_novac_dis_coh1++;
		if(itHum->second->getRecentHosp() > 0){
		  seroneg_novac_hosp_coh1++;
		}
	      }
	    }
	  }
	}

        itHum->second->resetRecent();
    }

    outpop << year << "," << 
      seropos_09 << "," << seroneg_09 << "," <<
      noinf_0008 << "," << inf_0008 << "," << noinf_0918 << "," << inf_0918 << "," << noinf_1999 << "," << inf_1999 << "," << 
      nodis_0008 << "," << dis_0008 << "," << nodis_0918 << "," << dis_0918  << "," << nodis_1999 << "," << dis_1999 << "," << 
      nohosp_0008 << "," << hosp_0008 << "," << nohosp_0918 << "," << hosp_0918 << "," << nohosp_1999  << "," << hosp_1999 << "," <<
      seropos_vac_coh1 <<  "," << seropos_vac_inf_coh1 << "," << seropos_vac_dis_coh1 << "," << seropos_vac_hosp_coh1 << "," <<
      seropos_novac_coh1 <<  "," << seropos_novac_inf_coh1 << "," << seropos_novac_dis_coh1 << "," << seropos_novac_hosp_coh1 << "," <<
      seroneg_vac_coh1 << ","  << seroneg_vac_inf_coh1 << "," << seroneg_vac_dis_coh1 << "," << seroneg_vac_hosp_coh1 << "," <<
      seroneg_novac_coh1 << ","  << seroneg_novac_inf_coh1 << "," << seroneg_novac_dis_coh1 << "," << seroneg_novac_hosp_coh1 << "\n";
    /*    
    if(vaccinationFlag == true){
      if(currentDay >= vaccineDay && currentDay < vaccineDay + 30){
	for(int i = 0;i < 100;i++){
	  outprevac << i << "," << popByAge[i] << "," << inf[i] << "," << dis[i] << "," << hosp[i] << "\n";
	}
      }
      }*/
}



void Simulation::humanDynamics() {
    int diff, age, dose;
    bool vaxd = false;
    int cohort = 0;
    if (currentDay >= vaccineDay){
      cohort = floor((currentDay - vaccineDay) / 365) + 1;
    }

    int seroposAtVax[100], seronegAtVax[100];
    if(currentDay == vaccineDay){
      for(int i = 0; i < 100; i++){
       seroposAtVax[i] = 0;
       seronegAtVax[i] = 0;
      }
    }

    //    printf("human dynamics entered cohort %d\n",cohort);
    for(auto it = humans.begin(); it != humans.end(); ++it){
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
	
	//Assign a cohort
	if(currentDay >= vaccineDay){
	  age = it->second->getAge(currentDay);
	  if(age == vaccineAge * 365){
	    it->second->setCohort(cohort);
	  }
	}

        // vaccinate, if applicable

	if(currentDay >= vaccineDay){
	  age = it->second->getAge(currentDay);
	    
	  if(currentDay == vaccineDay){
	    it->second->setSeroStatusAtVaccination();
	      
	    // record serostatus by age groups at the day of vaccination
	    int ageTemp = floor(age / 365);
	    if(ageTemp > 99){
	      ageTemp = 99;
	    }
	    if(it->second->getSeroStatusAtVaccination() == true){
	      seroposAtVax[ageTemp]++;
	    }else{
	      seronegAtVax[ageTemp]++;
	    }
	  }
	    
	    // routine vaccination by age
	  if(age == vaccineAge * 365){
	    it->second->setCohort(cohort);
	    if(vaccinationFlag == true){
	      if(rGenInf.getEventProbability() < vaccineCoverage)
		it->second->vaccinate(&VE_pos, &VE_neg,rGenInf, 1.0, currentDay);
	    }
	  }
	    
	    // catchup vaccination by age
	  if(vaccinationFlag == true){
	    if(catchupFlag == true && vaccineDay == currentDay){
	      if(age > vaccineAge * 365 && age < 18 * 365){
		if(rGenInf.getEventProbability() < vaccineCoverage)
		  it->second->vaccinate(&VE_pos, &VE_neg,rGenInf, 1.0, currentDay);
	      }
	    }
	  }
	}
	
        // simulate possible imported infection
        if(rGen.getEventProbability() < ForceOfImportation){
            int serotype = rGen.getRandomNum(4) + 1;
            if(!it->second->isImmune(serotype)){
                it->second->infect(currentDay, serotype, &rGenInf);
            }
        }
    }
    if(vaccinationFlag == true){
      if(currentDay == vaccineDay){
	for(int i = 0;i < 100;i++){
	  outprevac << i << "," << seroposAtVax[i] << "," << seronegAtVax[i] <<  "\n";
	}
      }
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
        if(biteTime < dieTime && biteTime <= 1.0){
    	  it->second->takeBite(biteTime,locations[it->second->getLocationID()].get(),&rGen,&rGenInf,currentDay,numDays,&out);
            if(dieTime < 1.0){
                auto it_temp = it;
                it++;
                mosquitoes.erase(it_temp);
                continue;
            }
        }
 
        // let the mosquito move if that happens today 
    	double moveProb = rGen.getEventProbability();
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
    rSeedInf = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    numDays = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccineDay = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    vaccinationFlag = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    getline(infile, line, ',');
    vaccineCoverage = strtod(line.c_str(), NULL);
    getline(infile, line, ',');
    vaccineAge = strtol(line.c_str(), NULL, 10);
    getline(infile, line, ',');
    catchupFlag = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
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

        unique_ptr<Human> h(new Human(houseID, hMemID, age, gen, trajectories, rGen, currentDay, ForceOfImportation));

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
