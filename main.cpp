#include "home.h"

LoggerPtr loggerConsole(Logger::getLogger( "Console"));

void ClearScreen(){
  HANDLE                     hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD                      count;
  DWORD                      cellCount;
  COORD                      homeCoords = { 0, 0 };

  hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  /* Get the number of cells in the current buffer */
  if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
  cellCount = csbi.dwSize.X *csbi.dwSize.Y;

  /* Fill the entire buffer with spaces */
  if (!FillConsoleOutputCharacter(
    hStdOut,
    (TCHAR) ' ',
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Fill the entire buffer with the current colors and attributes */
  if (!FillConsoleOutputAttribute(
    hStdOut,
    csbi.wAttributes,
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Move the cursor home */
  SetConsoleCursorPosition( hStdOut, homeCoords );
 }

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
	int res;
	const char u = ' ';

	while(TRUE){	
		ClearScreen();
		cout << "Parameter Menu:";
		cout << "\n1 - Thread number"<< endl;
		cout << "2 - Number of processed frame for every single thread"<< endl;		
		cout << "3 - Number of frame for background learning"<< endl;
		cout << "4 - Delivery Service (enable/disable)" << endl;
		cout << "5 - Background suppression threshold" << endl;	
		cout << "6 - Shadow threshold" << endl;
		cout << "0 - TERMINATED" << endl;		
		cout << "Input: ";
		cin >> res;

		switch(res){
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
					cout << "Delivery enabled" << endl;
				}else{
					initPar.thread_saving=TRUE;
					cout << "Delivery disabled" << endl;
				}
				system("PAUSE");
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
	int res;
	ClearScreen();
	cout << "*******************************************************************************" << endl;
	cout << "*   MVO'S & SHADOW DETECTION console  v.1.0                                   *" << endl;
	cout << "*     (Università degli studi di Catania - 2010-2011)                         *" << endl;
	cout << "*-----------------------------------------------------------------------------*" << endl;
	cout << "* ++ || Paolo Pino || ++ || Pierluigi Sottile || ++ || Vittorio Minacori|| ++ *" << endl;
	cout << "*******************************************************************************" << endl;
	cout << " -DEFAULT PARAMS----------------------------------------------------" << endl;	
	cout << "| background training: 1 frame | Delivery service: OFF              |" << endl;
	cout << "| background threshold: " << initPar.THRESHOLD << "                                          |" << endl;
	cout << "| K: " << initPar.K << " | alfa: auto |  beta: auto | Th: auto | Ts: auto \t    |"<<endl;
	cout << " -------------------------------------------------------------------" << endl;		
	cout << "\n1 - START"<< endl;
	cout << "2 - Set video path"<< endl;
	cout << "3 - Change parameter" << endl;
	cout << "0 - QUIT" << endl;
	cout << "\nInput: ";
	cin>>res;
	return res;
}

int main ( int argc, char **argv ){
	string response = "";
	DOMConfigurator::configure("Log4cxxConfig.xml");
	initPar =  _initializationParams();
	int res;

	while(TRUE){
		res=HomeMenu();
		switch(res){
			case 1: 
				Start();
				break;
			case 2:
				cout << "Insert video path: DISABLED";
				//cin >> initPar.videoPath;
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