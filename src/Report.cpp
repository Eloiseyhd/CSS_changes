#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Report.h"

using std::stringstream;

Report::Report(){
    reportCohort = false;
    reportAges = false;
    reportGroups = false;
    reportFOI = false;
    reportSpatial = false;

    printCohortPop = false;
    printAgesPop = false;
    printGroupsPop = false;

    printGroupsAgeFirst = false;
    printGroupsTotalAges = false;

    spatialMosquitoes = false;
    
    groupsMaxIndex = 0;
    cohortMaxIndex = 0;
    ageMaxIndex = 0;

    groupsAges.clear();
    groupsStats.clear();
    groupsAvgAgeFirst = 0;

    cohortAges.clear();
    cohortStats.clear();

    discreteAges.min = 0;
    discreteAges.max = 0;
    ageStats.clear();

    parameters.clear();
    spatialData.clear();
    
    for(int i = 0;i < 5;i++){
	cohortEvents[i] = 0;
	groupsEvents[i] = 0;
	ageEvents[i] = 0;
    }
    
    for(int i = 0; i < 2; i++){
	cohortStatus[i] = 0;
	groupsStatus[i] = 0;
	ageStatus[i] = 0;
    }
    
    for(int i = 0; i < 3; i++){
	groupsReportPeriod[i] = 0;
	cohortReportPeriod[i] = 0;
	ageReportPeriod[i] = 0;
	foiReportPeriod[i] = 0;
	spatialReportPeriod[i] = 0;
    }
    
    for(int i = 0; i < 4; i++){
	foiTypes[i] = 0;
	newInfections[i] = 0;
	susceptibles[i] = 0;
	mozSusceptibles[i] = 0;
	mozExposed[i] = 0;
	mozInfectious[i] = 0;
    }

    for(int i = 0; i < 5; i++){
	// There are four status VacSero+ VacSero- PlacSero+ PlacSero-
	for(int j = 0; j < 4; j++){
	    groupsTotalAgeStats.status[j].events[i] = 0;
	    groupsTotalAgeStats.status[j].nonevents[i] = 0;
	}
	groupsTotalAgeStats.total.events[i] = 0;
	groupsTotalAgeStats.total.nonevents[i] = 0;
    }
    
    outCohort.close();
    outAges.close();
    outGroups.close();

    events = {"inf", "dis", "hosp", "serop", "vac"};
    status = {"vac", "plac", "serop", "seron"};
}

void Report::setupReport(string file, string outputPath_, string simName_) {
    //       Read the reporting file and assign variable values
    if (file.length() == 0) {
	exit(1);
    }
    string line;
    std::ifstream infile(file);
    if(!infile.good()){
	exit(1);
    }
    while(getline(infile,line,'\n')){
	addParameter(line);
    }
    infile.close();

    this->readParameter("groups_events", "events",groupsEvents);
    this->readParameter("cohort_events","events",cohortEvents);
    this->readParameter("age_events","events", ageEvents);
    this->readParameter("groups_status","status",groupsStatus);
    this->readParameter("cohort_status","status",cohortStatus);
    this->readParameter("age_status","status",ageStatus);
    this->readParameter("groups_report_period","period",groupsReportPeriod);
    this->readParameter("cohort_report_period","period",cohortReportPeriod);
    this->readParameter("age_report_period","period",ageReportPeriod);
    this->readParameter("foi_report_period","period",foiReportPeriod);
    this->readParameter("foi_serotypes","serotypes",foiTypes);
    this->readParameter("spatial_report_period", "period", spatialReportPeriod);

    this->readParameter("groups_ages","ages",&groupsAges);
    this->readParameter("cohort_ages","ages",&cohortAges);
    discreteAges = this->readParameter("age_ages","ages", discreteAges);
    printGroupsPop = this->readParameter("groups_complement", printGroupsPop);
    printCohortPop = this->readParameter("cohort_complement", printCohortPop);
    printAgesPop = this->readParameter("age_complement", printAgesPop);
    printGroupsAgeFirst = this->readParameter("groups_avg_first", printGroupsAgeFirst);
    printGroupsTotalAges = this->readParameter("groups_print_total_ages", printGroupsTotalAges);
    reportGroups = this->readParameter("groups_print", reportGroups);
    reportCohort = this->readParameter("cohort_print", reportCohort);
    reportAges = this->readParameter("age_print", reportAges);
    reportFOI = this->readParameter("foi_print", reportFOI);
    reportSpatial = this->readParameter("spatial_print", reportSpatial);
    spatialMosquitoes = this->readParameter("spatial_mosquitoes", spatialMosquitoes);

    if(reportSpatial == true){
	string outputSpatialFile = outputPath_ + "/" + simName_ + "_spatial.csv";
	outSpatial.open(outputSpatialFile);
	if (!outSpatial.good()) {
	    exit(1);
	}
    }

    if(reportFOI == true){
	string outputFOIFile = outputPath_ + "/" + simName_ + "_foi.csv";
	outFOI.open(outputFOIFile);
	if (!outFOI.good()) {
	    exit(1);
	}
    }
    if(reportGroups == true){
	outputGroupsFile = outputPath_ + "/" + simName_ + "_pop.csv";
	outGroups.open(outputGroupsFile);
	if (!outGroups.good()) {
	    exit(1);
	}
    }

    if(reportCohort == true){
	outputCohortFile = outputPath_ + "/" + simName_ + "_cohort.csv";
	outCohort.open(outputCohortFile);
	if (!outCohort.good()) {
	    exit(1);
	}
    }

    if(reportAges == true){
	outputAgesFile = outputPath_ + "/" + simName_ + "_ages.csv";
	outAges.open(outputAgesFile);
	if (!outAges.good()) {
	    exit(1);
	}
    }
    resetReports();
    printHeaders();
}

void Report::readParameter(string param_name,string param_type, vector<rangeStruct> * values_){
   map<string, string>::iterator it;
    it = parameters.find(param_name);
    if(it != parameters.end()){
	if(param_type == "ages"){
	    if(param_name != "age_ages"){
		parseGroupsAges(it->second,values_);
	    }
	}
    }
}

Report::rangeStruct Report::readParameter(string param_name,string param_type, rangeStruct vtemp){
   map<string, string>::iterator it;
   rangeStruct values_ = vtemp;
    it = parameters.find(param_name);
    if(it != parameters.end()){
	if(param_type == "ages" && param_name == "age_ages"){
	    values_ = parseDiscreteAges(it->second);
	}
    }
    return values_;
}

bool Report::readParameter(string param_name, bool vtemp){
    map<string, string>::iterator it;
    bool values_ = vtemp;
    it = parameters.find(param_name);
    if(it != parameters.end()){
	values_ = parseBoolean(it->second);
    }
    return values_;
}

void Report::readParameter(string param_name, string param_type, int * values_){
    map<string, string>::iterator it;
    it = parameters.find(param_name);
    if(it != parameters.end()){
	if(param_type == "events"){
	    parseEvents(it->second,values_,5);
	}
	if(param_type == "status"){
	    parseEvents(it->second,values_,2);
	}
	if(param_type == "period"){
	    parsePeriod(it->second,values_);
	}
	if(param_type == "serotypes"){
	    parseEvents(it->second,values_,4);
	}
    }
}

void Report::addParameter(string line){
    if(line.size() > 0 && line[0] != '#' && line[0] != ' '){
	string param_name, param_value;
	size_t pos_equal = line.find_first_of('=');
	if(pos_equal != string::npos){
	    param_name = line.substr(0,pos_equal);
	    param_value = line.substr(pos_equal + 1);	    
	    // trim trailing spaces and weird stuff for param_name
	    pos_equal = param_name.find_first_of(" \t");
	    if(pos_equal != string::npos){
		param_name = param_name.substr(0,pos_equal);
	    }
	    // trim trailing and leading spaces and weird stuff from param_value
	    pos_equal = param_value.find_first_not_of(" \t");
	    if(pos_equal != string::npos){
		param_value = param_value.substr(pos_equal);
	    }
	    pos_equal = param_value.find_first_of("#");
	    if(pos_equal != string::npos){
		param_value = param_value.substr(0,pos_equal);
	    }
	    // Add the parameter name and value to the map
	    parameters.insert(make_pair(param_name,param_value));
	}
    }
}

bool Report::parseBoolean(string line){
    bool flag_temp = (std::stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    return flag_temp;
}

void Report::parsePeriod(string line, int * period_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    while(getline(linetemp,line2,',')){
		if(count > 2){
		    break;
		}
		period_temp[count] = strtol(line2.c_str(), NULL, 10);
		if(period_temp[count] < 0){
		    period_temp[count] = 0;
		}
		count++;
    }
    // If there are less than 3 values in the period, or the the start:increase:end don't make sense
    if(count < 3 || !(period_temp[0] < period_temp[1] + period_temp[2] && period_temp[1] < period_temp[2])){
		exit(1);
    }
}

void Report::parseGroupsAges(string line, vector<rangeStruct> * ages_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    linetemp << line;
    ages_temp->clear();
    while(getline(linetemp,line2,';')){
		stringstream lTemp; lTemp << line2;
		string line3;
		rangeStruct rangeTemp;
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

Report::rangeStruct Report::parseDiscreteAges(string line){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    linetemp << line;
    rangeStruct rangeTemp;
    getline(linetemp,line2,',');
    rangeTemp.min = strtol(line2.c_str(), NULL, 10);
    getline(linetemp,line2,',');
    rangeTemp.max = strtol(line2.c_str(), NULL, 10);
    if(rangeTemp.min + rangeTemp.max > 0 && rangeTemp.min < rangeTemp.max && rangeTemp.min >= 0){
	return rangeTemp;
    }else{
	exit(1);
    }
}

void Report::parseEvents(string line, int * Events_, int len){

    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    while(getline(linetemp,line2,',')){
	if(count >= len){
	    break;
	}
	Events_[count] = strtol(line2.c_str(), NULL, 10);
	if(Events_[count] > 1){
	    Events_[count] = 1;
	}
	if(Events_[count] < 0){
	    Events_[count] = 0;
	}
	count++;
    }
}

void Report::updateMosquitoReport(int currDay, Mosquito * m, Location * locNow){
    if(reportFOI == true){
	if(currDay >= foiReportPeriod[0] && currDay <= foiReportPeriod[2] && (currDay - foiReportPeriod[0]) % foiReportPeriod[1] == 0){
	    if(m->infection == nullptr){
		for(unsigned i = 0; i < 4; i++){
		    mozSusceptibles[i]++;
		}
	    }else{
		// get type
		unsigned sero = m->infection->getInfectionType();
		if(m->infection->getInfectiousness() > 0.0){
		    mozInfectious[sero - 1]++;
		}else{
		    mozExposed[sero - 1]++;
		}
	    }
	}
    }
    if(reportSpatial == true && spatialMosquitoes == true){
	if(currDay >= spatialReportPeriod[0] && currDay <= spatialReportPeriod[2] && (currDay - spatialReportPeriod[0]) % spatialReportPeriod[1] == 0){
	    if(m->infection != nullptr){
		unsigned sero = m->infection->getInfectionType();
		if(m->infection->getInfectiousness() > 0.0){
		    if(currDay == m->infection->getStartDay()){
			string tmp_str = std::to_string(locNow->getLocX()) + "," + std::to_string(locNow->getLocY()) + "," + std::to_string(currDay) + "," + std::to_string(sero);
			spatialData.push_back(tmp_str);
		    }
		}
	    }
	}
    }
}

void Report::updateReport(int currDay, Human * h, Location * locNow){
    int reportNum = 0;
    if(reportGroups == true){
       	if(currDay >= groupsReportPeriod[0] && currDay <= groupsReportPeriod[2] && (currDay - groupsReportPeriod[0]) % groupsReportPeriod[1] == 0){
	    updateGroupsReport(currDay, h);
	    reportNum++;
	}
    }
    if(reportCohort == true){
	if(currDay >= cohortReportPeriod[0] && currDay <= cohortReportPeriod[2] && (currDay - cohortReportPeriod[0]) % cohortReportPeriod[1] == 0){
	    updateCohortReport(currDay, h);
	    reportNum++;
	}
    }
    if(reportAges == true){
	if(currDay >= ageReportPeriod[0] && currDay <= ageReportPeriod[2] && (currDay - ageReportPeriod[0]) % ageReportPeriod[1] == 0){
	    updateAgesReport(currDay, h);
	    reportNum++;
	}
    }
    if(reportFOI == true){
	if(currDay >= foiReportPeriod[0] && currDay <= foiReportPeriod[2] && (currDay - foiReportPeriod[0]) % foiReportPeriod[1] == 0){
	    updateFOIReport(currDay, h);
	    reportNum++;
	}
    }
    if(reportSpatial == true && spatialMosquitoes == false){
	if(currDay >= spatialReportPeriod[0] && currDay <= spatialReportPeriod[2] && (currDay - spatialReportPeriod[0]) % spatialReportPeriod[1] == 0){
	    updateSpatialReport(currDay, h, locNow);
	}
    }
    if(reportNum > 0){
	h->resetRecent();
    }
}

void Report::printReport(int currDay){
    if(reportGroups == true){
	if(currDay >= groupsReportPeriod[0] && currDay <= groupsReportPeriod[2] && (currDay - groupsReportPeriod[0]) % groupsReportPeriod[1] == 0){
	    printGroupsReport(currDay);
	    resetGroupStats();
	}
    }
    if(reportCohort == true){
	if(currDay >= cohortReportPeriod[0] && currDay <= cohortReportPeriod[2] && (currDay - cohortReportPeriod[0]) % cohortReportPeriod[1] == 0){
	    printCohortReport(currDay);
	    resetCohortStats();
	}
    }
    if(reportAges == true){
	if(currDay >= ageReportPeriod[0] && currDay <= ageReportPeriod[2] && (currDay - ageReportPeriod[0]) % ageReportPeriod[1] == 0){
	    printAgesReport(currDay);
	    resetAgeStats();
	}
    }
    if(reportFOI == true){
	if(currDay >= foiReportPeriod[0] && currDay <= foiReportPeriod[2] && (currDay - foiReportPeriod[0]) % foiReportPeriod[1] == 0){
	    printFOIReport(currDay);
	    resetFOIStats();
	}
    }
    if(reportSpatial == true){
	if(currDay >= spatialReportPeriod[0] && currDay <= spatialReportPeriod[2] && (currDay - spatialReportPeriod[0]) % spatialReportPeriod[1] == 0){
	    printSpatialReport(currDay);
	    resetSpatialStats();
	}
    }
}

void Report::updateFOIReport(int currDay, Human * h){
    // Collect the number of susceptibles and infections per serotype
    // If h is infectious, then get the serotype
    if(h->getRecentInf() > 0){
	int sero = h->getRecentType();
	if(sero > 0){
	    newInfections[sero - 1]++; //The types from Human go from 1 - 4
	}
    }else{
	// If h isn't infected then check for immunity to all the serotypes
	for(unsigned i = 0; i < 4; i++){
	    susceptibles[i] +=  h->isImmune(i + 1) ? 0 : 1;
	}
    }
}

void Report::updateSpatialReport(int currDay, Human * h, Location * locNow){
    if(h->infection != nullptr){
	unsigned sero = h->infection->getInfectionType();
	if(currDay == h->infection->getStartDay()){
	    string tmp_str = std::to_string(locNow->getLocX()) + "," + std::to_string(locNow->getLocY()) + "," + std::to_string(currDay) + "," + std::to_string(sero);
	    spatialData.push_back(tmp_str);
	}
    }
}

void Report::updateAgesReport(int currDay, Human * h){

    int age_temp = floor((double) h->getAgeDays(currDay) / 365.0);
    if(age_temp > discreteAges.max || age_temp < discreteAges.min){
	return;
    }
    int group = age_temp - discreteAges.min;

    if(h->getRecentInf() > 0){
	ageStats[group].total.events[0]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].events[0]++;
	    }else{
		ageStats[group].status[1].events[0]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].events[0]++;
	    }else{
		ageStats[group].status[3].events[0]++;
	    }
	}
	if(h->getRecentDis() > 0){
	    ageStats[group].total.events[1]++;
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    ageStats[group].status[0].events[1]++;
		}else{
		    ageStats[group].status[1].events[1]++;
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    ageStats[group].status[2].events[1]++;
		}else{
		    ageStats[group].status[3].events[1]++;
		}
	    }
	    if(h->getRecentHosp() > 0){
		ageStats[group].total.events[2]++;
		if(h->isVaccinated() == true){
		    if(h->getSeroStatusAtVaccination() == true){
			ageStats[group].status[0].events[2]++;
		    }else{
			ageStats[group].status[1].events[2]++;
		    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			ageStats[group].status[2].events[2]++;
		    }else{
			ageStats[group].status[3].events[2]++;
		    }
		}
	    } else {
		ageStats[group].total.nonevents[2]++;
		if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].nonevents[2]++;
	    }else{
		ageStats[group].status[1].nonevents[2]++;
	    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			ageStats[group].status[2].nonevents[2]++;
		    }else{
			ageStats[group].status[3].nonevents[2]++;
		    }
		}
	    }
	} else {
	    ageStats[group].total.nonevents[1]++;
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    ageStats[group].status[0].nonevents[1]++;
		}else{
		    ageStats[group].status[1].nonevents[1]++;
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    ageStats[group].status[2].nonevents[1]++;
		}else{
		    ageStats[group].status[3].nonevents[1]++;
		}
	    }
	}
    } else {
	ageStats[group].total.nonevents[0]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].nonevents[0]++;
	    }else{
		ageStats[group].status[1].nonevents[0]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].nonevents[0]++;
	    }else{
		ageStats[group].status[3].nonevents[0]++;
	    }
	}
    }

    if(h->getPreviousInfections() > 0){
	ageStats[group].total.events[3]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].events[3]++;
	    }else{
		ageStats[group].status[1].events[3]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].events[3]++;
	    }else{
		ageStats[group].status[3].events[3]++;
	    }
	}
    } else {
	ageStats[group].total.nonevents[3]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].nonevents[3]++;
	    }else{
		ageStats[group].status[1].nonevents[3]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].nonevents[3]++;
	    }else{
		ageStats[group].status[3].nonevents[3]++;
	    }
	}
    }
    if(h->isVaccinated() == true){
	ageStats[group].total.events[4]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].events[4]++;
	    }else{
		ageStats[group].status[1].events[4]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].events[4]++;
	    }else{
		ageStats[group].status[3].events[4]++;
	    }
	}
    } else {
	ageStats[group].total.nonevents[4]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[0].nonevents[4]++;
	    }else{
		ageStats[group].status[1].nonevents[4]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		ageStats[group].status[2].nonevents[4]++;
	    }else{
		ageStats[group].status[3].nonevents[4]++;
	    }
	}
    }
}


void Report::updateGroupsReport(int currDay, Human * h){

    if(h->getRecentInf() && h->getPreviousInfections() == 1){
	groupsAvgAgeFirst += h->getAgeDays(currDay);
	groupsTotalFirstInf++;
    }

    int group = getGroup(h->getAgeDays(currDay), groupsAges);

    if(h->getRecentInf() > 0){
	groupsTotalAgeStats.total.events[0]++;
	if(group >= 0){
	    groupsStats[group].total.events[0]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].events[0]++;
		if(group >= 0){
		    groupsStats[group].status[0].events[0]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].events[0]++;
		if(group >= 0){
		    groupsStats[group].status[1].events[0]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].events[0]++;
		if(group >= 0){
		    groupsStats[group].status[2].events[0]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].events[0]++;
		if(group >= 0){
		    groupsStats[group].status[3].events[0]++;
		}
	    }
	}
	if(h->getRecentDis() > 0){
	    groupsTotalAgeStats.total.events[1]++;
	    if(group >= 0){
		groupsStats[group].total.events[1]++;
	    }
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    groupsTotalAgeStats.status[0].events[1]++;
		    if(group >= 0){
			groupsStats[group].status[0].events[1]++;
		    }
		}else{
		    groupsTotalAgeStats.status[1].events[1]++;
		    if(group >= 0){
			groupsStats[group].status[1].events[1]++;
		    }
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    groupsTotalAgeStats.status[2].events[1]++;
		    if(group >= 0){
			groupsStats[group].status[2].events[1]++;
		    }
		}else{
		    groupsTotalAgeStats.status[3].events[1]++;
		    if(group >= 0){
			groupsStats[group].status[3].events[1]++;
		    }
		}
	    }
	    if(h->getRecentHosp() > 0){
		groupsTotalAgeStats.total.events[2]++;
		if(group >= 0){
		    groupsStats[group].total.events[2]++;
		}
		if(h->isVaccinated() == true){
		    if(h->getSeroStatusAtVaccination() == true){
			groupsTotalAgeStats.status[0].events[2]++;
			if(group >= 0){
			    groupsStats[group].status[0].events[2]++;
			}
		    }else{
			groupsTotalAgeStats.status[1].events[2]++;
			if(group >= 0){
			    groupsStats[group].status[1].events[2]++;
			}
		    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			groupsTotalAgeStats.status[2].events[2]++;
			if(group >= 0){
			    groupsStats[group].status[2].events[2]++;
			}
		    }else{
			groupsTotalAgeStats.status[3].events[2]++;
			if(group >= 0){
			    groupsStats[group].status[3].events[2]++;
			}
		    }
		}
	    } else {
		groupsTotalAgeStats.total.nonevents[2]++;
		if(group >= 0){
		    groupsStats[group].total.nonevents[2]++;
		}
		if(h->isVaccinated() == true){
		    if(h->getSeroStatusAtVaccination() == true){
			groupsTotalAgeStats.status[0].nonevents[2]++;
			if(group >= 0){
			    groupsStats[group].status[0].nonevents[2]++;
			}
		    }else{
			groupsTotalAgeStats.status[1].nonevents[2]++;
			if(group >= 0){
			    groupsStats[group].status[1].nonevents[2]++;
			}
		    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			groupsTotalAgeStats.status[2].nonevents[2]++;
			if(group >= 0){
			    groupsStats[group].status[2].nonevents[2]++;
			}
		    }else{
			groupsTotalAgeStats.status[3].nonevents[2]++;
			if(group >= 0){
			    groupsStats[group].status[3].nonevents[2]++;
			}
		    }
		}
	    }
	} else {
	    groupsTotalAgeStats.total.nonevents[1]++;
	    if(group >= 0){
		groupsStats[group].total.nonevents[1]++;
	    }
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    groupsTotalAgeStats.status[0].nonevents[1]++;
		    if(group >= 0){
			groupsStats[group].status[0].nonevents[1]++;
		    }
		}else{
		    groupsTotalAgeStats.status[1].nonevents[1]++;
		    if(group >= 0){
			groupsStats[group].status[1].nonevents[1]++;
		    }
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    groupsTotalAgeStats.status[2].nonevents[1]++;
		    if(group >= 0){
			groupsStats[group].status[2].nonevents[1]++;
		    }
		}else{
		    groupsTotalAgeStats.status[3].nonevents[1]++;
		    if(group >= 0){
			groupsStats[group].status[3].nonevents[1]++;
		    }
		}
	    }
	}
    } else {
	groupsTotalAgeStats.total.nonevents[0]++;
	if(group >= 0){
	    groupsStats[group].total.nonevents[0]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].nonevents[0]++;
		if(group >= 0){
		    groupsStats[group].status[0].nonevents[0]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].nonevents[0]++;
		if(group >= 0){
		    groupsStats[group].status[1].nonevents[0]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].nonevents[0]++;
		if(group >= 0){
		    groupsStats[group].status[2].nonevents[0]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].nonevents[0]++;
		if(group >= 0){
		    groupsStats[group].status[3].nonevents[0]++;
		}
	    }
	}
    }


    if(h->getPreviousInfections() > 0){
	groupsTotalAgeStats.total.events[3]++;
	if(group >= 0){
	    groupsStats[group].total.events[3]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].events[3]++;
		if(group >= 0){
		    groupsStats[group].status[0].events[3]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].events[3]++;
		if(group >= 0){
		    groupsStats[group].status[1].events[3]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].events[3]++;
		if(group >= 0){
		    groupsStats[group].status[2].events[3]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].events[3]++;
		if(group >= 0){
		    groupsStats[group].status[3].events[3]++;
		}
	    }
	}
    } else {
	groupsTotalAgeStats.total.nonevents[3]++;
	if(group >= 0){
	    groupsStats[group].total.nonevents[3]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].nonevents[3]++;
		if(group >= 0){
		    groupsStats[group].status[0].nonevents[3]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].nonevents[3]++;
		if(group >= 0){
		    groupsStats[group].status[1].nonevents[3]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].nonevents[3]++;
		if(group >= 0){
		    groupsStats[group].status[2].nonevents[3]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].nonevents[3]++;
		if(group >= 0){
		    groupsStats[group].status[3].nonevents[3]++;
		}
	    }
	}
    }
    if(h->isVaccinated() == true){
	groupsTotalAgeStats.total.events[4]++;
	if(group >= 0){
	    groupsStats[group].total.events[4]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].events[4]++;
		if(group >= 0){
		    groupsStats[group].status[0].events[4]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].events[4]++;
		if(group >= 0){
		    groupsStats[group].status[1].events[4]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].events[4]++;
		if(group >= 0){
		    groupsStats[group].status[2].events[4]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].events[4]++;
		if(group >= 0){
		    groupsStats[group].status[3].events[4]++;
		}
	    }
	}
    } else {
	groupsTotalAgeStats.total.nonevents[4]++;
	if(group >= 0){
	    groupsStats[group].total.nonevents[4]++;
	}
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[0].nonevents[4]++;
		if(group >= 0){
		    groupsStats[group].status[0].nonevents[4]++;
		}
	    }else{
		groupsTotalAgeStats.status[1].nonevents[4]++;
		if(group >= 0){
		    groupsStats[group].status[1].nonevents[4]++;
		}
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		groupsTotalAgeStats.status[2].nonevents[4]++;
		if(group >= 0){
		    groupsStats[group].status[2].nonevents[4]++;
		}
	    }else{
		groupsTotalAgeStats.status[3].nonevents[4]++;
		if(group >= 0){
		    groupsStats[group].status[3].nonevents[4]++;
		}
	    }
	}
    }
}



void Report::updateCohortReport(int currDay, Human * h){
    int cohortNum = h->getCohort();
    if(cohortNum != 1){
	return;
    }
    // get group based on age at trial enrollment
    int cohortAgeGroup = getGroup(h->getAgeTrialEnrollment(),cohortAges); 
    if(cohortAgeGroup < 0){
	return;
    }
    if(h->getRecentInf() > 0){
	cohortStats[cohortAgeGroup].total.events[0]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].events[0]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].events[0]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].events[0]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].events[0]++;
	    }
	}
	if(h->getRecentDis() > 0){
	    cohortStats[cohortAgeGroup].total.events[1]++;
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    cohortStats[cohortAgeGroup].status[0].events[1]++;
		}else{
		    cohortStats[cohortAgeGroup].status[1].events[1]++;
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    cohortStats[cohortAgeGroup].status[2].events[1]++;
		}else{
		    cohortStats[cohortAgeGroup].status[3].events[1]++;
		}
	    }
	    if(h->getRecentHosp() > 0){
		cohortStats[cohortAgeGroup].total.events[2]++;
		if(h->isVaccinated() == true){
		    if(h->getSeroStatusAtVaccination() == true){
			cohortStats[cohortAgeGroup].status[0].events[2]++;
		    }else{
			cohortStats[cohortAgeGroup].status[1].events[2]++;
		    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			cohortStats[cohortAgeGroup].status[2].events[2]++;
		    }else{
			cohortStats[cohortAgeGroup].status[3].events[2]++;
		    }
		}
	    } else {
		cohortStats[cohortAgeGroup].total.nonevents[2]++;
		if(h->isVaccinated() == true){
		    if(h->getSeroStatusAtVaccination() == true){
			cohortStats[cohortAgeGroup].status[0].nonevents[2]++;
		    }else{
			cohortStats[cohortAgeGroup].status[1].nonevents[2]++;
		    }
		}else{
		    if(h->getSeroStatusAtVaccination() == true){
			cohortStats[cohortAgeGroup].status[2].nonevents[2]++;
		    }else{
			cohortStats[cohortAgeGroup].status[3].nonevents[2]++;
		    }
		}
	    }
	} else {
	    cohortStats[cohortAgeGroup].total.nonevents[1]++;
	    if(h->isVaccinated() == true){
		if(h->getSeroStatusAtVaccination() == true){
		    cohortStats[cohortAgeGroup].status[0].nonevents[1]++;
		}else{
		    cohortStats[cohortAgeGroup].status[1].nonevents[1]++;
		}
	    }else{
		if(h->getSeroStatusAtVaccination() == true){
		    cohortStats[cohortAgeGroup].status[2].nonevents[1]++;
		}else{
		    cohortStats[cohortAgeGroup].status[3].nonevents[1]++;
		}
	    }
	}
    } else {
	cohortStats[cohortAgeGroup].total.nonevents[0]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].nonevents[0]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].nonevents[0]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].nonevents[0]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].nonevents[0]++;
	    }
	}
    }


    if(h->getPreviousInfections() > 0){
	cohortStats[cohortAgeGroup].total.events[3]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].events[3]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].events[3]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].events[3]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].events[3]++;
	    }
	}
    } else {
	cohortStats[cohortAgeGroup].total.nonevents[3]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].nonevents[3]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].nonevents[3]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].nonevents[3]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].nonevents[3]++;
	    }
	}
    }
    if(h->isVaccinated() == true){
	cohortStats[cohortAgeGroup].total.events[4]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].events[4]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].events[4]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].events[4]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].events[4]++;
	    }
	}
    } else {
	cohortStats[cohortAgeGroup].total.nonevents[4]++;
	if(h->isVaccinated() == true){
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[0].nonevents[4]++;
	    }else{
		cohortStats[cohortAgeGroup].status[1].nonevents[4]++;
	    }
	}else{
	    if(h->getSeroStatusAtVaccination() == true){
		cohortStats[cohortAgeGroup].status[2].nonevents[4]++;
	    }else{
		cohortStats[cohortAgeGroup].status[3].nonevents[4]++;
	    }
	}
    }
}



int Report::getGroup(int age_, vector<rangeStruct> groups_temp){
    vector<rangeStruct>::iterator itAge = groups_temp.begin();
    int count = 0;
    for(; itAge != groups_temp.end(); itAge++){
	if((double )age_ / 365.0 >= (*itAge).min && (double) age_ / 365.0 < (*itAge).max){
	    return count;
	}
	count++;
    }
    return -1;
}

void Report::printSpatialReport(int currDay){
    if(!spatialData.empty()){
	for(auto it = spatialData.begin(); it != spatialData.end(); ++it){
	    outSpatial << (*it) << "\n";
	}
    }
}

void Report::printFOIReport(int currDay){
    vector<string> foi_values; string outstring;
    foi_values.clear();
    for(int i = 0; i < 4; i++){
	if(foiTypes[i]){
	    double foi_temp = susceptibles[i] > 0 ? (double) newInfections[i] / (double) susceptibles[i] : 1;
	    foi_values.push_back(std::to_string(foi_temp));
	    foi_values.push_back(std::to_string(susceptibles[i]));
	    foi_values.push_back(std::to_string(mozSusceptibles[i]));
	    foi_values.push_back(std::to_string(mozExposed[i]));
	    foi_values.push_back(std::to_string(mozInfectious[i]));
	}
    }
    if(!foi_values.empty()){
	Report::join(foi_values,',',outstring);
	outFOI << currDay << ",";
	outFOI << outstring;
    }
}

void Report::printAgesReport(int currDay){
    outAges << currDay << ",";

    for(int i = 0; i < 5 ; i++){
	if(ageEvents[i] == 1){
	    if(ageStatus[0] > 0 || ageStatus[1] > 0){
		if(ageStatus[0] > 0 && ageStatus[1] > 0){
		    for(int j = 0; j < 4; j++){
			for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
			    if(printAgesPop == true){
				outAges << ageStats[k].status[j].nonevents[i]<<",";
			    }
			    outAges << ageStats[k].status[j].events[i];
			    if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min && j == 3){
				outAges << "\n";
			    }else{
				outAges << ",";
			    }
			}
		    }
		}else{
		    int inc_ = 0;
		    int sum_ = 0;
		    if(ageStatus[0] > 0){
			inc_ = 2;
			sum_ = 1;
		    }else if(ageStatus[1] > 0){ 
			inc_ = 1;
			sum_ = 2;
		    }
		    for(int j = 0;j < 2 * inc_;j = j = j + inc_){
			for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
			    if(printAgesPop == true){
				outAges << ageStats[k].status[j].nonevents[i] + ageStats[k].status[j + sum_].nonevents[i]<<",";
			    }
			    outAges << ageStats[k].status[j].events[i] + ageStats[k].status[j + sum_].events[i];
			    if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min && j == inc_){
				outAges << "\n";
			    }else{
				outAges << ",";
			    }
			}
		    }
		}

	    }else{
		for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
		    if(printAgesPop == true){
			outAges << ageStats[k].total.nonevents[i]<<",";
		    }
		    outAges << ageStats[k].total.events[i];
		    if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min){
			outAges << "\n";
		    }else{
			outAges << ",";
		    }
		}
	    }
	}
    }
}

	

void Report::printGroupsReport(int currDay){
    outGroups << currDay << ",";
    if(printGroupsAgeFirst == true){
	outGroups << (double) groupsAvgAgeFirst / (double) groupsTotalFirstInf / 365.0 << ",";
    }
    for(int i = 0; i < 5 ; i++){
	if(groupsEvents[i] == 1){
	    if(groupsStatus[0] > 0 || groupsStatus[1] > 0){
		if(groupsStatus[0] > 0 && groupsStatus[1] > 0){
		    for(int j = 0;j < 4 ;j++){
			for(int k = 0; k < groupsAges.size(); k++){
			    if(printGroupsPop == true){
				outGroups << groupsStats[k].status[j].nonevents[i]<<",";
			    }
			    outGroups << groupsStats[k].status[j].events[i];

			    if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
				if(printGroupsPop == true){
				    outGroups << "," << groupsTotalAgeStats.status[j].nonevents[i];
				}
				outGroups << "," << groupsTotalAgeStats.status[j].events[i];
			    }
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == 3){
				outGroups << "\n";
			    }else{
				outGroups << ",";
			    }
			}
		    }
		}else{
		    int inc_ = 0;
		    int sum_ = 0;
		    if(groupsStatus[0] > 0){
			inc_ = 2;
			sum_ = 1;
		    }else if(groupsStatus[1] > 0){ 
			inc_ = 1;
			sum_ = 2;
		    }
		    for(int j = 0;j < 2 * inc_;j = j = j + inc_){
			for(int k = 0; k < groupsAges.size(); k++){
			    if(printGroupsPop == true){
				outGroups << groupsStats[k].status[j].nonevents[i] + groupsStats[k].status[j + sum_].nonevents[i]<<",";
			    }
			    outGroups << groupsStats[k].status[j].events[i] + groupsStats[k].status[j + sum_].events[i];

			    if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
				if(printGroupsPop == true){
				    outGroups << "," << groupsTotalAgeStats.status[j].nonevents[i] + groupsTotalAgeStats.status[j + sum_].nonevents[i];
				}
				outGroups << "," << groupsTotalAgeStats.status[j].events[i] + groupsTotalAgeStats.status[j + sum_].events[i];
			    }
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == inc_){
				outGroups << "\n";
			    }else{
				outGroups << ",";
			    }
			}
		    }
		}
	    }else{
		for(int k = 0; k < groupsStats.size(); k++){
		    if(printGroupsPop == true){
			outGroups << groupsStats[k].total.nonevents[i]<<",";
		    }
		    outGroups << groupsStats[k].total.events[i];

		    if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
			if(printGroupsPop == true){
			    outGroups << "," <<  groupsTotalAgeStats.total.nonevents[i];
			}
			outGroups << "," << groupsTotalAgeStats.total.events[i];
		    }
		    if(i == groupsMaxIndex && k == groupsStats.size() - 1){
			outGroups << "\n";
		    }else{
			outGroups << ",";
		    }
		}
	    }
	}
    }
}



void Report::printCohortReport(int currDay){
    outCohort << currDay << ",";

    for(int i = 0; i < 5 ; i++){
	if(cohortEvents[i] == 1){
	    if(cohortStatus[0] > 0 || cohortStatus[1] > 0){
		if(cohortStatus[0] > 0 && cohortStatus[1] > 0){
		    for(int j = 0;j < 4 ;j++){
			for(int k = 0; k < cohortAges.size(); k++){
			    if(printCohortPop == true){
				outCohort << cohortStats[k].status[j].nonevents[i]<<",";
			    }
			    outCohort << cohortStats[k].status[j].events[i];
			    if(i == cohortMaxIndex && k == cohortAges.size() - 1 && j == 3){
				outCohort << "\n";
			    }else{
				outCohort << ",";
			    }
			}
		    }
		}else{
		    int inc_ = 0;
		    int sum_ = 0;
		    if(cohortStatus[0] > 0){
			inc_ = 2;
			sum_ = 1;
		    }else if(cohortStatus[1] > 0){ 
			inc_ = 1;
			sum_ = 2;
		    }
		    for(int j = 0;j < 2 * inc_;j = j = j + inc_){
			for(int k = 0; k < cohortAges.size(); k++){
			    if(printCohortPop == true){
				outCohort << cohortStats[k].status[j].nonevents[i] + cohortStats[k].status[j + sum_].nonevents[i]<<",";
			    }
			    outCohort << cohortStats[k].status[j].events[i] + cohortStats[k].status[j + sum_].events[i];
			    if(i == cohortMaxIndex && k == cohortAges.size() - 1 && j == inc_){
				outCohort << "\n";
			    }else{
				outCohort << ",";
			    }
			}
		    }
		}

	    }else{
		for(int k = 0; k < cohortStats.size(); k++){
		    if(printCohortPop == true){
			outCohort << cohortStats[k].total.nonevents[i]<<",";
		    }
		    outCohort << cohortStats[k].total.events[i];
		    if(i == cohortMaxIndex && k == cohortStats.size() - 1){
			outCohort << "\n";
		    }else{
			outCohort << ",";
		    }
		}
	    }
	}
    }
}



void Report::printHeaders(){
    if(reportGroups == true){
	printGroupsHeader();
    }
    if(reportCohort == true){
	printCohortHeader();
    }
    if(reportAges == true){
	printAgesHeader();
    }
    if(reportFOI == true){
	printFOIHeader();
    }
    if(reportSpatial == true){
	printSpatialHeader();
    }
}

void Report::join(const vector<string>& v, char c, string& s) {

    s.clear();

    for (vector<string>::const_iterator p = v.begin(); p != v.end(); ++p) {
	s += *p;
	if (p != v.end() - 1){
	    s += c;
	}else{
	    s +="\n";
	}
    }

}

void Report::printSpatialHeader(){
    vector<string> headers; string outstring;
    headers.clear();
    headers.push_back("Xcoor,Ycoor");
    if(spatialMosquitoes == true){
	headers.push_back("start_inf_mosquito");
    }else{
	headers.push_back("start_inf_human");
    }
    headers.push_back("serotype");
    if(!headers.empty()){
	Report::join(headers,',',outstring);
	outSpatial << outstring;
    }
}

void Report::printFOIHeader(){
    vector<string> headers; string outstring;
    headers.clear();
    for(int i = 0; i < 4; i++){
	if(foiTypes[i]){
	    headers.push_back("Denv" + std::to_string(i+1));
	    headers.push_back("Susceptible_" + std::to_string(i+1));
	    headers.push_back("MozSusceptible_" + std::to_string(i+1));
	    headers.push_back("MozExposed_" + std::to_string(i+1));
	    headers.push_back("MozInfectious_" + std::to_string(i+1));
	}
    }
    if(!headers.empty()){
	Report::join(headers,',',outstring);
	outFOI << "day,";
	outFOI << outstring;
    }
}

void Report::printAgesHeader(){
    outAges << "day,";
    for(int i = 0; i < 5 ; i++){
	if(ageEvents[i] == 1){
	    if(ageStatus[0] > 0 || ageStatus[1] > 0){
		if(ageStatus[0] > 0 && ageStatus[1] > 0){
		    for(int j = 0;j < 2 ;j++){
			for(int jj = 2; jj < 4; jj++){
			    for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
				if(printAgesPop == true){
				    outAges << status[j].c_str() <<"_"<< status[jj].c_str() << "_age_" << k + discreteAges.min << "_";
				    outAges << "no"<<events[i].c_str()<<",";
				}
				outAges << status[j].c_str() << "_" << status[jj].c_str() << "_age_" << k + discreteAges.min << "_";
				outAges << events[i].c_str();
				if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min && j == 1 && jj == 3){
				    outAges << "\n";
				}else{
				    outAges << ",";
				}
			    }
			}
		    }
		}else{
		    int ind_ = 0;
		    if(ageStatus[0] > 0){
			ind_ = 0;
		    }else if(ageStatus[1] > 0){ 
			ind_ = 2;
		    }
		    for(int j = ind_;j < ind_ + 2;j++){
			for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
			    if(printAgesPop == true){
				outAges << status[j].c_str() << "_age_" << k + discreteAges.min << "_";
				outAges << "no"<<events[i].c_str()<<",";
			    }
			    outAges << status[j].c_str() << "_age_" << k + discreteAges.min << "_";
			    outAges << events[i].c_str();
			    if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min && j == ind_ + 1){
				outAges << "\n";
			    }else{
				outAges << ",";
			    }
			}
		    }
		}

	    }else{
		for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
		    if(printAgesPop == true){
			outAges << "age_" << k + discreteAges.min << "_";
			outAges << "no"<<events[i].c_str()<<",";
		    }
		    outAges << "age_" << k + discreteAges.min << "_";
		    outAges << events[i].c_str();
		    if(i == ageMaxIndex && k == discreteAges.max - discreteAges.min){
			outAges << "\n";
		    }else{
			outAges << ",";
		    }
		}
	    }
	}
    }
}

void Report::printGroupsHeader(){
    outGroups << "day,";
    if(printGroupsAgeFirst == true){
	outGroups << "avg_age_first,";
    }
    for(int i = 0; i < 5 ; i++){
	if(groupsEvents[i] == 1){
	    if(groupsStatus[0] > 0 || groupsStatus[1] > 0){
		if(groupsStatus[0] > 0 && groupsStatus[1] > 0){
		    for(int j = 0;j < 2 ;j++){
			for(int jj = 2; jj < 4; jj++){
			    for(int k = 0; k < groupsAges.size(); k++){
				if(printGroupsPop == true){
				    outGroups << status[j].c_str() <<"_"<< status[jj].c_str() << "_age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
				    outGroups << "no"<<events[i].c_str()<<",";
				}
				outGroups << status[j].c_str() << "_" << status[jj].c_str() << "_age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
				outGroups << events[i].c_str();

				if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
				    if(printGroupsPop == true){
					outGroups << "," << status[j].c_str() <<"_"<< status[jj].c_str() << "_all_";
					outGroups << "no"<<events[i].c_str();
				    }
				    outGroups << "," << status[j].c_str() << "_" << status[jj].c_str() << "_all_";
				    outGroups << events[i].c_str();
				}
				if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == 1 && jj == 3){
				    outGroups << "\n";
				}else{			       
				    outGroups << ",";
				}
			    }
			}
		    }
		}else{
		    int ind_ = 0;
		    if(groupsStatus[0] > 0){
			ind_ = 0;
		    }else if(groupsStatus[1] > 0){ 
			ind_ = 2;
		    }
		    for(int j = ind_;j < ind_ + 2;j++){
			for(int k = 0; k < groupsAges.size(); k++){
			    if(printGroupsPop == true){
				outGroups << status[j].c_str() << "_age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
				outGroups << "no"<<events[i].c_str()<<",";
			    }
			    outGroups << status[j].c_str() << "_age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
			    outGroups << events[i].c_str();

			    if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
				if(printGroupsPop == true){
				    outGroups << "," << status[j].c_str() << "_all_";
				    outGroups << "no"<<events[i].c_str();
				}
				outGroups << "," << status[j].c_str() << "_all_";
				outGroups << events[i].c_str();
			    }
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == ind_ + 1){
				outGroups << "\n";
			    }else{
				outGroups << ",";
			    }
			}
		    }
		}

	    }else{
		for(int k = 0; k < groupsAges.size(); k++){
		    if(printGroupsPop == true){
			outGroups << "age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
			outGroups << "no"<<events[i].c_str()<<",";
		    }
		    outGroups << "age_" << groupsAges[k].min << "_" << groupsAges[k].max << "_";
		    outGroups << events[i].c_str();

		    if(printGroupsTotalAges == true && k == groupsAges.size() - 1){
			if(printGroupsPop == true){
			    outGroups << "," <<  "all_" ;
			    outGroups << "no"<<events[i].c_str();
			}
			outGroups << "," << "all_";
			outGroups << events[i].c_str();
		    }
		    if(i == groupsMaxIndex && k == groupsAges.size() - 1){
			outGroups << "\n";
		    }else{
			outGroups << ",";
		    }
		}
	    }
	}
    }
}



void Report::printCohortHeader(){
    outCohort << "day,";
    for(int i = 0; i < 5 ; i++){
	if(cohortEvents[i] == 1){
	    if(cohortStatus[0] > 0 || cohortStatus[1] > 0){
		if(cohortStatus[0] > 0 && cohortStatus[1] > 0){
		    for(int j = 0;j < 2 ;j++){
			for(int jj = 2; jj < 4; jj++){
			    for(int k = 0; k < cohortAges.size(); k++){
				if(printCohortPop == true){
				    outCohort << status[j].c_str() <<"_"<< status[jj].c_str() << "_age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
				    outCohort << "no"<<events[i].c_str()<<",";
				}
				outCohort << status[j].c_str() << "_" << status[jj].c_str() << "_age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
				outCohort << events[i].c_str();
				if(i == cohortMaxIndex && k == cohortAges.size() - 1 && j == 1 && jj == 3){
				    outCohort << "\n";
				}else{
				    outCohort << ",";
				}
			    }
			}
		    }
		}else{
		    int ind_ = 0;
		    if(cohortStatus[0] > 0){
			ind_ = 0;
		    }else if(cohortStatus[1] > 0){ 
			ind_ = 2;
		    }
		    for(int j = ind_;j < ind_ + 2;j++){
			for(int k = 0; k < cohortAges.size(); k++){
			    if(printCohortPop == true){
				outCohort << status[j].c_str() << "_age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
				outCohort << "no"<<events[i].c_str()<<",";
			    }
			    outCohort << status[j].c_str() << "_age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
			    outCohort << events[i].c_str();
			    if(i == cohortMaxIndex && k == cohortAges.size() - 1 && j == ind_ + 1){
				outCohort << "\n";
			    }else{
				outCohort << ",";
			    }
			}
		    }
		}

	    }else{
		for(int k = 0; k < cohortAges.size(); k++){
		    if(printCohortPop == true){
			outCohort << "age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
			outCohort << "no"<<events[i].c_str()<<",";
		    }
		    outCohort << "age_" << cohortAges[k].min << "_" << cohortAges[k].max << "_";
		    outCohort << events[i].c_str();
		    if(i == cohortMaxIndex && k == cohortAges.size() - 1){
			outCohort << "\n";
		    }else{
			outCohort << ",";
		    }
		}
	    }
	}
    }
}



void Report::resetReports(){
    if(reportGroups == true){
	resetGroupStats();
    }
    if(reportCohort == true){
	resetCohortStats();
    }
    if(reportAges == true){
	resetAgeStats();
    }
    if(reportFOI == true){
	resetFOIStats();
    }
    if(reportSpatial == true){
	resetSpatialStats();
    }
}
void Report::resetSpatialStats(){
    spatialData.clear();
}
void Report::resetFOIStats(){
    for(int i = 0; i < 4; i++){
	newInfections[i] = 0;
	susceptibles[i] = 0;
	mozSusceptibles[i] = 0;
	mozInfectious[i] = 0;
	mozExposed[i] = 0;
    }
}

void Report::resetAgeStats(){
    ageStats.clear();
    if(discreteAges.min + discreteAges.max <= 0 || discreteAges.min >= discreteAges.max || discreteAges.min < 0){
	exit(1);
    }

    for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
	reportStats tempStats;
	for(int i = 0; i < 5; i++){
	    // There are four status VacSero+ VacSero- PlacSero+ PlacSero-
	    for(int j = 0; j < 4; j++){
		tempStats.status[j].events[i] = 0;
		tempStats.status[j].nonevents[i] = 0;
	    }
	    tempStats.total.events[i] = 0;
	    tempStats.total.nonevents[i] = 0;
	}
	ageStats.push_back(tempStats);
    }

    for(int i = 0; i < 5; i++){
	if(ageEvents[i] == 1){
	    ageMaxIndex = i;
	}
    }
}

void Report::resetCohortStats(){
    cohortStats.clear();
    if(cohortAges.empty()){
	exit(1);
    }

    for(int k = 0; k < cohortAges.size(); k++){
	reportStats tempStats;
	for(int i = 0; i < 5; i++){
	    // There are four status VacSero+ VacSero- PlacSero+ PlacSero-
	    for(int j = 0; j < 4; j++){
		tempStats.status[j].events[i] = 0;
		tempStats.status[j].nonevents[i] = 0;
	    }
	    tempStats.total.events[i] = 0;
	    tempStats.total.nonevents[i] = 0;
	}
	cohortStats.push_back(tempStats);
    }

    for(int i = 0; i < 5; i++){
	if(cohortEvents[i] == 1){
	    cohortMaxIndex = i;
	}
    }
}


void Report::resetGroupStats(){
    groupsAvgAgeFirst = 0;
    groupsTotalFirstInf = 0;
    groupsStats.clear();

    if(groupsAges.empty()){
	exit(1);
    }
    for(int k = 0; k < groupsAges.size(); k++){
	reportStats tempStats;
	for(int i = 0; i < 5; i++){
	    // There are four status VacSero+ VacSero- PlacSero+ PlacSero-
	    for(int j = 0; j < 4; j++){
		tempStats.status[j].events[i] = 0;
		tempStats.status[j].nonevents[i] = 0;
	    }
	    tempStats.total.events[i] = 0;
	    tempStats.total.nonevents[i] = 0;
	}
	groupsStats.push_back(tempStats);
    }

    for(int i = 0; i < 5; i++){
	if(groupsEvents[i] == 1){
	    groupsMaxIndex = i;
	}
    }

    for(int i = 0; i < 5; i++){
	// There are four status VacSero+ VacSero- PlacSero+ PlacSero-
	for(int j = 0; j < 4; j++){
	    groupsTotalAgeStats.status[j].events[i] = 0;
	    groupsTotalAgeStats.status[j].nonevents[i] = 0;
	}
	groupsTotalAgeStats.total.events[i] = 0;
	groupsTotalAgeStats.total.nonevents[i] = 0;
    }
}



void Report::finalizeReport(){
    outCohort.close();
    outAges.close();
    outGroups.close();
    outFOI.close();
    outSpatial.close();
}



//Report::~Report() {
//}

