#include "Vaccine.h"
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <string>

using namespace std;

Vaccine::Vaccine(){
    vaccineID = -1;
    mode = "none";
    name = "none";
    waning = 0.0;
    protection = 0.0;
    propInf = 0.0;
    normdev = 0.0;
    for(unsigned k = 0; k < 3; k++){
	VE_pos.insert(make_pair(k,0.0));
	VE_neg.insert(make_pair(k,0.0));
    }
    total_VE = 0.0;
    doses = 1;
    relative_schedule.clear();
}

int Vaccine::getNextDoseDay(int vday, int received){
    if(received < doses){
	return (relative_schedule[received] + vday);
    }else{
	return (relative_schedule.back() + vday);
    }
}
double Vaccine::getRR(double prevInf, double ageDays){
    if(prevInf > 0){
	total_VE = 1.0 - VE_pos.at(0) / (1.0 + exp(VE_pos.at(1) * ageDays / 365.0 - VE_pos.at(2)));
    }else{
	total_VE = 1.0 - VE_neg.at(0) / (1.0 + exp(VE_neg.at(1) * ageDays / 365.0 - VE_neg.at(2)));
    }
    double RR = exp(log(1.0 - total_VE) + normdev * pow(1.0 / 100.5 + 1.0 / (100.0 * (1.0 - total_VE) + 0.5), 0.5));
    return RR;
}

void Vaccine::printVaccine(){
    printf("Vaccine ID: %u Name: %s Mode: %s Waning %.2f Protection %.2f Efficacy %.2f PropInf %.2f NormDev %.2f Doses %d\n",vaccineID, name.c_str(), mode.c_str(), waning,protection,total_VE,
	   propInf, normdev, doses);
    for(unsigned j = 0; j < 3; j++){
	printf("PAR %d VE pos %.2f VE neg %.2f\n",j,VE_pos.at(j), VE_neg.at(j));
    }
    for(int j = 0; j < relative_schedule.size(); j++){
	printf("Dose %d Day %d\t", j + 1, relative_schedule[j]);
    }
    printf("\n");
}
void Vaccine::setRelativeSchedule(std::vector<int> rs){
    relative_schedule = rs;
    if(relative_schedule.size() > 0){
	doses = relative_schedule.size();
    }
}


Vaccine::~Vaccine() {
}

