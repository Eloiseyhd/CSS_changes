#include <fstream>
#include <string>
#include <iostream>
#include <utility>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>
#include "Report.h"

using namespace std;


Report::Report(){
    reportCohort = false;
    reportAges = false;
    reportGroups = false;

    printCohortPop = false;
    printAgesPop = false;
    printGroupsPop = false;

    printGroupsAgeFirst = false;
    printGroupsTotalAges = false;

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
    if (file.length() == 0) {
		printf("File name %s is empty\n",file.c_str());
		exit(1);
    }

    string line;
    ifstream infile(file);

    int count;
    if(!infile.good()){
		printf("File %s is corrupted\n",file.c_str());
		exit(1);
    }

    while(getline(infile,line,'\n')){
	stringstream linetemp;
	string line2;
	string line3;
	linetemp.clear();
	linetemp << line;
	getline(linetemp,line2,'=');
	getline(linetemp,line3,'=');
	linetemp.clear();
	linetemp << line2;
	getline(linetemp,line2,' ');

	// group variables		
	if(line2 == "groups_events"){
	    parseEvents(line3,groupsEvents);
	}
	if(line2 == "groups_status"){
	    parseStatus(line3,groupsStatus);
	}
	if(line2 == "groups_ages"){
	    parseGroupsAges(line3,&groupsAges);
	}
	if(line2 == "groups_report_period"){
	    parsePeriod(line3,groupsReportPeriod);
	}
	if(line2 == "groups_complement"){
	    printGroupsPop = parseComplement(line3);
	}
	if(line2 == "groups_avg_first"){
	    printGroupsAgeFirst = parseGroupsAgeFirst(line3);
	}	
	if(line2 == "groups_print"){
	    reportGroups = parsePrintFlag(line3);
	}
	if(line2 == "groups_print_total_ages"){
	    printGroupsTotalAges = parsePrintFlag(line3);
	}

	// cohort variables
	if(line2 == "cohort_events"){
	    parseEvents(line3,cohortEvents);
	}
	if(line2 == "cohort_status"){
	    parseStatus(line3,cohortStatus);
	}	
	if(line2 == "cohort_ages"){
	    parseGroupsAges(line3,&cohortAges);
	}
	if(line2 == "cohort_report_period"){
	    parsePeriod(line3,cohortReportPeriod);
	}
	if(line2 == "cohort_complement"){
	    printCohortPop = parseComplement(line3);
	}
	if(line2 == "cohort_print"){
	    reportCohort = parsePrintFlag(line3);
	}
	// discrete ages variables
	if(line2 == "age_events"){
	    parseEvents(line3,ageEvents);
	}
	if(line2 == "age_status"){
	    parseStatus(line3,ageStatus);
	}	
	if(line2 == "age_ages"){
	    discreteAges = parseDiscreteAges(line3);
	}
	if(line2 == "age_report_period"){
	    parsePeriod(line3,ageReportPeriod);
	}
	if(line2 == "age_complement"){
	    printAgesPop = parseComplement(line3);
	}
	if(line2 == "age_print"){
	    reportAges = parsePrintFlag(line3);
	}
    }
    infile.close();

    if(reportGroups == true){
	outputGroupsFile = outputPath_ + "/" + simName_ + "_pop.csv";
	outGroups.open(outputGroupsFile);
	if (!outGroups.good()) {
	    printf("file %s is broken\n", outputGroupsFile.c_str());
	    exit(1);
	}
    }

    if(reportCohort == true){
	outputCohortFile = outputPath_ + "/" + simName_ + "_cohort.csv";
	outCohort.open(outputCohortFile);
	if (!outCohort.good()) {
	    printf("file %s is broken\n", outputCohortFile.c_str());
	    exit(1);
	}
    }

    if(reportAges == true){
	outputAgesFile = outputPath_ + "/" + simName_ + "_ages.csv";
	outAges.open(outputAgesFile);
	if (!outAges.good()) {
	    printf("file %s is broken\n", outputAgesFile.c_str());
	    exit(1);
	}
    }

    resetReports();
    printHeaders();
}



bool Report::parsePrintFlag(std::string line){
   bool print_temp = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
   return print_temp;
}



bool Report::parseGroupsAgeFirst(std::string line){
    bool age_temp = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    return age_temp;
}



bool Report::parseComplement(std::string line){
    bool complement_temp = (stoi(line.c_str(), NULL, 10) == 0 ? false : true);
    return complement_temp;
}



void Report::parsePeriod(std::string line, int * period_temp){
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
    if(count < 3 || !(period_temp[0] < period_temp[1] + period_temp[2] && period_temp[1] < period_temp[2])){
		exit(1);
    }
}



void Report::parseGroupsAges(std::string line, std::vector<rangeStruct> * ages_temp){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
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
		printf("File does not contain valid values for the group age ranges\n");
		exit(1);
    }
}

rangeStruct Report::parseDiscreteAges(std::string line){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    rangeStruct rangeTemp;
    getline(linetemp,line2,',');
    rangeTemp.min = strtol(line2.c_str(), NULL, 10);
    getline(linetemp,line2,',');
    rangeTemp.max = strtol(line2.c_str(), NULL, 10);
    if(rangeTemp.min + rangeTemp.max > 0 && rangeTemp.min < rangeTemp.max && rangeTemp.min >= 0){
	return rangeTemp;
    }else{
	printf("File does not contain valid values for the group age ranges\n");
	exit(1);
    }
}



void Report::parseStatus(std::string line, int * Status_){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    count = 0;
    while(getline(linetemp,line2,',')){
	if(count > 1){
	    break;
	}
	Status_[count] = strtol(line2.c_str(), NULL, 10);
	if(Status_[count] > 1){
	    Status_[count] = 1;
	}
	if(Status_[count] < 0){
	    Status_[count] = 0;
	}
	count++;
    }
}



void Report::parseEvents(std::string line, int * Events_){
    stringstream linetemp;
    string line2;
    linetemp.clear();
    int count =0;
    linetemp << line;
    while(getline(linetemp,line2,',')){
	if(count > 4){
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



void Report::updateReport(int currDay, Human * h){
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
    //    printf("Human Cohort %d group[%d] and age at enrollment %f\n",cohortNum, cohortAgeGroup, (double) h->getAgeTrialEnrollment()/365.0);
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



int Report::getGroup(int age_, std::vector<rangeStruct> groups_temp){
    std::vector<rangeStruct>::iterator itAge = groups_temp.begin();
    int count = 0;
    for(; itAge != groups_temp.end(); itAge++){
	if((double )age_ / 365.0 >= (*itAge).min && (double) age_ / 365.0 < (*itAge).max){
	    //	    printf("get group::This person of age: %f goes in ageGroup %d to %d\n",(double) age_ / 365.0,(*itAge).min,(*itAge).max);
	    return count;
	}
	count++;
    }
    return -1;
}


void Report::printAgesReport(int currDay){
    outAges << currDay << ",";

    for(int i = 0; i < 5 ; i++){
	if(ageEvents[i] == 1){
	    if(ageStatus[0] > 0 || ageStatus[1] > 0){
		if(ageStatus[0] > 0 && ageStatus[1] > 0){
		    for(int j = 0; j < 4; j++){
			for(int k = 0; k <= discreteAges.max - discreteAges.min; k++){
			    //			    printf("Stats for group %d j %d, event %s = %d, non%s = %d\n",k, j,events[i].c_str(),  groupsStats[k].status[j].events[i], events[i].c_str(),groupsStats[k].status[j].nonevents[i]);
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
			    //			    printf("Stats for group %d j %d, event %s = %d, non%s = %d\n",k, j,events[i].c_str(),  groupsStats[k].status[j].events[i], events[i].c_str(),groupsStats[k].status[j].nonevents[i]);
			    if(printGroupsPop == true){
				outGroups << groupsStats[k].status[j].nonevents[i]<<",";
			    }
			    outGroups << groupsStats[k].status[j].events[i];
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == 3){
				outGroups << "\n";
			    }else{
				if(printGroupsTotalAges){
				    if(printGroupsPop == true){
					outGroups << "," << groupsTotalAgeStats.status[j].nonevents[i];
				    }
				    outGroups << "," << groupsTotalAgeStats.status[j].events[i];
				}
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
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == inc_){
				outGroups << "\n";
			    }else{
				if(printGroupsTotalAges){
				    if(printGroupsPop == true){
					outGroups << "," << groupsTotalAgeStats.status[j].nonevents[i] + groupsTotalAgeStats.status[j + sum_].nonevents[i];
				    }
				    outGroups << "," << groupsTotalAgeStats.status[j].events[i] + groupsTotalAgeStats.status[j + sum_].events[i];
				}
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
		    if(i == groupsMaxIndex && k == groupsStats.size() - 1){
			outGroups << "\n";
		    }else{
			if(printGroupsTotalAges){
			    if(printGroupsPop == true){
				outGroups << "," <<  groupsTotalAgeStats.total.nonevents[i];
			    }
			    outGroups << "," << groupsTotalAgeStats.total.events[i];
			}
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
				if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == 1 && jj == 3){
				    outGroups << "\n";
				}else{
				    if(printGroupsTotalAges == true){
					if(printGroupsPop == true){
					    outGroups << "," << status[j].c_str() <<"_"<< status[jj].c_str() << "_all_";
					    outGroups << "no"<<events[i].c_str();
					}
					outGroups << "," << status[j].c_str() << "_" << status[jj].c_str() << "_all_";
					outGroups << events[i].c_str();
				    }
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
			    if(i == groupsMaxIndex && k == groupsAges.size() - 1 && j == ind_ + 1){
				outGroups << "\n";
			    }else{
				if(printGroupsTotalAges == true){
				    if(printGroupsPop == true){
					outGroups << "," << status[j].c_str() << "_all_";
					outGroups << "no"<<events[i].c_str();
				    }
				    outGroups << "," << status[j].c_str() << "_all_";
				    outGroups << events[i].c_str();
				}
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
		    if(i == groupsMaxIndex && k == groupsAges.size() - 1){
			outGroups << "\n";
		    }else{
			if(printGroupsTotalAges == true){
			    if(printGroupsPop == true){
				outGroups << "," <<  "all_" ;
				outGroups << "no"<<events[i].c_str();
			    }
			    outGroups << "," << "all_";
			    outGroups << events[i].c_str();
			}
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
}



void Report::resetAgeStats(){
    ageStats.clear();
    if(discreteAges.min + discreteAges.max <= 0 || discreteAges.min >= discreteAges.max || discreteAges.min < 0){
	printf("no discrete ages specified\n");
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
	printf("no cohort ages specified\n");
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
	printf("no group ages specified\n");
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
}



Report::~Report() {

}

