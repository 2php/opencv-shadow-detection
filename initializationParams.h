#ifndef _initializationParams_h
#define _initializationParams_h


#define DEFAULT_THRESHOLD 30
#define DEFAULT_K 1
#define DEFAULT_THREAD_NUM 30

typedef struct _initializationParams {
        int useDefault;
		int THREAD_NUM;
		float THRESHOLD;
        double alfa;
		double beta;
		double Th;
		double Ts;
		float K;
   // costruttore della struttura
        _initializationParams() : 
        useDefault(1),THRESHOLD(DEFAULT_THRESHOLD),K(DEFAULT_K),THREAD_NUM(DEFAULT_THREAD_NUM){ }
  }initializationParams;

//extern struct initializationParams;
#endif