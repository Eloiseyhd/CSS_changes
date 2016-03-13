/* 
 * File:   tests.cpp
 * Author: amit
 *
 * Created on August 29, 2014, 2:03 AM
 */

#include <cstdlib>
#include "Simulation.h"
#include "RandomNumGenerator.h"
#include <random>
#include <iostream>

using namespace std;

/*
 * 
 */
//int main(int argc, char** argv) {
int test(){
    
    //test code
    
    Simulation sim;
    sim.readLocationFile("C:\\Users\\amit\\Desktop\\denguesim\\locations_20140807_test.csv");
    //sim.printLocations();
    sim.readHumanFile("C:\\Users\\amit\\Desktop\\denguesim\\trajectories_test.txt");
    //sim.printHumans();
    sim.readMosquitoEmergenceFile("C:\\Users\\amit\\Desktop\\denguesim\\Mosquitoes_test.csv");
    //sim.printMEmergence();
    sim.generateMosquitoes();
    //sim.printMosquitoes();
    //sim.humanDynamics();
    
    sim.tests();
    
    /* 
     * 
     * List of unit tests
     * 1. Mosquito infection PASSED
     * 2. Human infection PASSED
     * 3. Human immunity
     * 4. New methods in RandomNumGen PASSED
     * 5. Location getDistance PASSED
     * 6. Location add closeLoc PASSED
     * 7. Location random closeLoc PASSED
     * 8. Simulation
       9. setLocNeighborhood(); PASSED
     * 10.simEngine(); PASSED
     * 11.humanDynamics();
    12. void humanVisit();
    13. void attemptBite();
    14. void mosquitoDynamics(); PASSED
     
    //Test code for random num
    RandomNumGenerator rnum1(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum1.getMozLifeSpan() <<" ";
    }
    random_device rd;
    RandomNumGenerator rnum2(rd());
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum2.getMozLifeSpan() <<" ";
    }
    RandomNumGenerator rnum3(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum3.getMozLifeSpan() <<" ";
    }
    cout << "\n";
    
    RandomNumGenerator rnum1(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum1.getEventProbability() <<" ";
    }
    

    RandomNumGenerator rnum11(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum11.getMozIncubationDays() <<" ";
    }
    RandomNumGenerator rnum12(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum12.getHumanTrajectory() <<" ";
    }
    RandomNumGenerator rnum13(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum13.getHumanImmunity() <<" ";
    }
    RandomNumGenerator rnum14(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum14.getMozRestDays() <<" ";
    }
    RandomNumGenerator rnum15(17);
    cout << "\n";
    for (int i=0; i<5; i++) {
        cout<< rnum15.getMozNextLoc(7) <<" ";
    }
     */
    return 0;
}

void Simulation::tests() {
    Infection inf(0, 5, 10, Infection::InfType::DENV1);
    Infection inf2(0, 6, 11, Infection::InfType::DENV2);
    cout << "\nt" << inf.toString();

    //humans.begin()->second->addInfection(inf);
    //humans.begin()->second->addInfection(inf2);
    //mosquitoes.begin()->second->infections->push_back(inf2);

    printHumans();
    printMosquitoes();



    cout << "\nDistance test:";
    auto& l1 = locations.find("PUE070")->second;
    auto& l2 = locations.find("TAB431-432")->second;
    cout << "\nDistance of : " << l1->getLocID() << " from " << l2->getLocID() << " is " << l1->getDistanceFromLoc((*l2));
    cout << " \n\nNeigh test\n";
    setLocNeighborhood(50);
    printLocations();

    cout << "\nPick random";
    cout << "\nRandom neighbor of " << l1->getLocID() << " is ";
    cout << l1->getRandomCloseLoc(rGen);
    currentDay = 0;
    numDays = 5;

    //
    cout << "\n\nTest human dynamics";
    cout << "\n--------------------";
    //simEngine();
    //humanDynamics();
    //currentDay++;
    //cout<<"\n\ncurrDay:"<<currentDay;
    //humanDynamics();

}