#ifndef _initializationParams_h
#define _initializationParams_h

#include <iostream>
#include <string.h>

#define DEFAULT_THRESHOLD 30
#define DEFAULT_K 10
#define DEFAULT_THREAD_NUM 30

typedef struct _initializationParams {
        int useDefault;
		int THREAD_NUM;
		int POOL;
		float THRESHOLD;
        double alfa;
		double beta;
		double Th;
		double Ts;
		float K;
		int Delta;
		int cicle_background;
		bool thread_saving;
		string videoPath;
   // costruttore della struttura
        _initializationParams() : 
			useDefault(1),
			THRESHOLD(DEFAULT_THRESHOLD),
			K(DEFAULT_K),
			THREAD_NUM(DEFAULT_THREAD_NUM),
			alfa(-1),beta(-1),Th(-1),Ts(-1),Delta(10),
			cicle_background(1),
			thread_saving(TRUE),
			POOL(30){ }
  }initializationParams;

//extern struct initializationParams;
#endif