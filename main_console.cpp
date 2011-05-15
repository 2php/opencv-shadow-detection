#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"
#include <iostream>
#include <string>

initializationParams initPar;
LoggerPtr loggerConsole(Logger::getLogger( "Console"));

void shodowThresholdMenu(){
		//alfa deve essere compreso tra 0 e 1
		do{
			cout << "Define alfa con un valore compreso tra 0 e 1: (-1 to set AUTO)\n "; 
			cin >> initPar.alfa;
		}while(initPar.alfa<0 || initPar.alfa>1);
		//beta deve essere compreso tra 0 e 1, inoltre deve essere maggiore di alfa
		do {
			cout << "Define beta con un valore compreso tra 0 e 1: (-1 to set AUTO)\n "; 
			cin >> initPar.beta;
		}while (initPar.beta<0 || initPar.beta>1 || initPar.alfa>initPar.beta);
		cout << "Define Th: ";
		cin >> initPar.Th;
		cout << "Define Ts: ";
		cin >> initPar.Ts;
		cout << "Define K: ";
		cin >> initPar.K;
}

int ParameterMenu(){
	while(TRUE){	
		cout.clear();
		cout << "Parameter Menu:";
		cout << "\n1 - Thread number"<< endl;
		cout << "2 - Number of processed frame for every single thread"<< endl;		
		cout << "3 - Number of frame for background learning"<< endl;
		cout << "4 - Delivery Service (enable/disable)" << endl;
		cout << "5 - Background suppression threshold" << endl;	
		cout << "6 - Shadow threshold" << endl;
		cout << "0 - TERMINATED" << endl;		
		switch(cin.get()){
			case 1: 
				cout << "\ndefine number of thread: ";
				cin >> initPar.POOL;
				break;
			case 2:
				cout << "max frame for thread number: ";
				cin >> initPar.THREAD_NUM;
				break;
			case 3: 
				cout << "number of training frame: ";
				cin >> initPar.cicle_background;
				break;
			case 4: 
				if(initPar.thread_saving==TRUE){
					initPar.thread_saving=FALSE;
					cout << "Delivery enabled";
				}else{
					initPar.thread_saving=TRUE;
					cout << "Delivery diabled";
				}
				break;
			case 5: 
				do{
					cout << "background suppression threshold: ";
					cin >> initPar.THRESHOLD;
				}while(initPar.THRESHOLD>250 || initPar.THRESHOLD<-1);
				
				break;
			case 6: 
				shodowThresholdMenu();
				break;
			case 0:
				return 0;
			default:
				cout << "Invalid option" << endl;
				break;
		}
	}
}

int HomeMenu(){
	cout << "*******************************************************************************" << endl;
	cout << "*   MVO'S & SHADOW DETECTION console  v.1.0                                   *" << endl;
	cout << "*     (Università degli studi di Catania - 2010-2011)                         *" << endl;
	cout << "*-----------------------------------------------------------------------------*" << endl;
	cout << "* ++ || Paolo Pino || ++ || Pierluigi Sottile || ++ || Vittorio Minacori|| ++ *" << endl;
	cout << "*******************************************************************************" << endl;
	cout << "DEFAULT PARAMS:\n" << endl;
	cout << "Number of frame for background model creation: 1" << endl;
	cout << "Delivery service: OFF" << endl;
	cout << "THRESHOLD: " << initPar.THRESHOLD << "\nK: " << initPar.K << "\nalfa: auto\nbeta: auto\nTh: auto\nTs: auto";
	cout << "\n1 - Start"<< endl;
	cout << "2 - Set video path"<< endl;
	cout << "3 - Change parameter" << endl;
	cout << "0 - QUIT" << endl;
	return cin.get();
}

int main ( int argc, char **argv ){
	string response = "";
	DOMConfigurator::configure("Log4cxxConfig.xml");
	initPar =  _initializationParams();


	while(TRUE){
		switch(HomeMenu()){
			case 1: 
				Start();
				break;
			case 2:
				cout << "Insert video path: ";
				cin >> initPar.videoPath;
				break;
			case 3: 
				ParameterMenu();
				break;
			case 0:
				LOG4CXX_INFO(loggerConsole,"Program closed");
				return 0;
			default:
				cout << "Invalid option" << endl;
				break;
		}			
	}
}	