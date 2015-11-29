#include <cstdlib>
#include <random>
#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <chrono>
#include "ThreadPool.h"
#include "Simulation.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    srand(time(0));
    char *configFileName;
    
    cout << "\nDengueSim_v1.0.0beta1:" << endl;

    if (argc != 2 && argc != 3 && argc != 5) { // Check for number of input arguments
        cout << "\nCheck arguments!\nOptions:\nDengueSim [SimControlFile]\nDengueSim [SimControlFile] [NumOfThreads]\n";
        cout << "DengueSim -d <Distance> [LocationsFile] [NeighborhoodFile]\n";
        exit(1); // Exit if not enough arguments
    } else if (argc == 2) {
        chrono::time_point<chrono::high_resolution_clock> begin, end, newSimBegin;
        begin = chrono::high_resolution_clock::now();
        configFileName = argv [1];
        
        string conFile(configFileName);
        double min, sec;

        ifstream infile(conFile);
        if (!infile.good()) {
            cout << "\n\n" << "Can't open file:" << conFile << ". Exiting.\n\n";
            exit(1);
        }
        string line;
        getline(infile, line);
        while (getline(infile, line)) {
            newSimBegin = chrono::high_resolution_clock::now();
            Simulation sim(line);
            string simName = sim.readInputs();
            end = chrono::high_resolution_clock::now();
            min = chrono::duration_cast<std::chrono::nanoseconds> (end-newSimBegin).count()/ 60000000000.;
            sec = (min - (int)min)*60.0;
            cout << "\n\n" << simName <<  ": Simulation generation took " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
            sim.simulate();
            end = chrono::high_resolution_clock::now();
            min = chrono::duration_cast<std::chrono::nanoseconds> (end-newSimBegin).count()/ 60000000000.;
            sec = (min - (int)min)*60.0;
            cout << "\n\n" << simName <<  ": Simulation completed in " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
            while (infile.peek() == '\n') {
                infile.ignore(1, '\n');
            }                     
        }
        infile.close();

        end = chrono::high_resolution_clock::now();
        min = chrono::duration_cast<std::chrono::nanoseconds> (end-begin).count()/ 60000000000.;
        sec = (min - (int)min)*60.0;
        cout<<"\n\nTotal simulation time was " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
        cout<<"\n";

    } else if (argc == 3) {
        chrono::time_point<chrono::high_resolution_clock> begin, end, newSimBegin;
        begin = chrono::high_resolution_clock::now();
        unsigned maxThreads = strtol(argv[2], NULL, 10);
        configFileName = argv [1];
        Simulation sim(configFileName);
        string conFile(configFileName);

        ifstream infile(conFile);
        if (!infile.good()) {
            cout << "\n\n" << "Can't open file:" << conFile << ". Exiting.\n\n";
            exit(1);
        }
        string line;
        getline(infile, line);

        ThreadPool pool(maxThreads);
        std::vector< std::future<int> > results;

        while (getline(infile, line)) {
            results.push_back(
                pool.enqueue([line,&infile,&begin] {
                    chrono::time_point<chrono::high_resolution_clock> newSimBegin, newSimEnd;
                    newSimBegin = chrono::high_resolution_clock::now();
                    Simulation sim(line);
                    string simName = sim.readInputs();
                    newSimEnd = chrono::high_resolution_clock::now();
                    double min = chrono::duration_cast<std::chrono::nanoseconds> (newSimEnd-newSimBegin).count()/ 60000000000.;
                    double sec = (min - (int)min)*60.0;
                    cout << "\n\n" << simName <<  ": Simulation generation took " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
                    sim.simulate();
                    newSimEnd = chrono::high_resolution_clock::now();
                    min = chrono::duration_cast<std::chrono::nanoseconds> (newSimEnd-newSimBegin).count()/ 60000000000.;
                    sec = (min - (int)min)*60.0;
                    cout << "\n\n" << simName <<  ": Simulation completed in " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
                    while (infile.peek() == '\n') {
                        infile.ignore(1, '\n');
                    }  
                    return 0;
                })
            );
        }
        infile.close();

        for(size_t i = 0;i<results.size();++i) {
            results[i].get();
        }
        end = chrono::high_resolution_clock::now();
        double min = chrono::duration_cast<std::chrono::nanoseconds> (end-begin).count()/ 60000000000.;
        double sec = (min - (int)min)*60.0;
        cout<<"\n\nTotal simulation time was " << (int)min << " minutes and "<< (int) sec<<" seconds." << endl;
        cout<<"\n";

    } else if (argc == 5) {
        if (string(argv[1]) != "-d") {
            cout << "\nCheck arguments!\nOptions:\nDengueSim [SimControFile]\nDengueSim -d <Distance(meters)> [LocationsFile] [NeighborhoodFile]\n";
            exit(1);
        }
        Simulation sim;
        sim.createNeighborhoodFile(argv[2], argv[3], argv[4]);
    }
    return 0;
}
