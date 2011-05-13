#include <iostream>
#include <fstream>
#include <string>
#pragma once

using namespace std;

////Questo va scritto nel main x inizializzare azzerare il file log.txt
//
//{
//    fstream f("log.txt", ios::out); //apre il file in scrittura (cancellando quello che già c'era)
//	
//    if(!f) {
//        cout< <"Errore nella creazione del file!";
//        return -1;
//    }
//



void logCon(string s);
void logFile(string s);


