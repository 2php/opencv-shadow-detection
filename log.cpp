#include "log.h"


//void logCon(string s)
//fa il logging su console
//@param[in] prende in ingresso la stringa da inserire.
//@param[out] scrive la stringa ricevuta su console.
void logCon(string s)
{
	cout<<s;
}

//void logFile(string s)
//fa il logger su file
//@param[in] prende in ingresso la stringa da inserire.
//@param[out] scrive s sul file in modalità append.
//@param[out] scrive anche su console.
void logFile(string s)
{
	ofstream f("log.txt", ios::app); //apre il file in modalità append, lasciando intatto quello che c'è e scrivendo alla fine
    if(!f) {//controlla che f sia stato creato nel modo giusto.
        cout<<"Errore nell'apertura del file!";
        return ;
    }
	f<<s;
	logCon(s);
	f.close();
}