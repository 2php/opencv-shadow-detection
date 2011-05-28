#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#define TL 15

static LoggerPtr loggerDelivery(Logger::getLogger( "Delivery"));
static LoggerPtr loggerMain(Logger::getLogger( "main"));
static LoggerPtr loggerThread(Logger::getLogger( "Thread"));

/*!
//lista dei frame elaborati*/
static list<FrameObject*> frame;
static list<FrameObject> ordered;
static vector<HANDLE> handle;
static HANDLE mutex,started;
static initializationParams initPar;
static volatile int size;
static bool thread_saving;
static int gap;

//Parametri delle immagini memorizzate su disco
static int p[3];    	

/*!
//void Start (initializationParams par, string path)
//Mvo's detection and shadow suppression method 
// 	@param[in] par the inizialization parameter
//	@param[in] path the path of video suorce file
*/
void Start(initializationParams par,string path);
/*!
//reShadowing(FrameObject *frame, IplImage *background)
//Reinsert detected ghost into background
// 	@param[in] frame the frame object
//	@param[out] background the result image 
*/
void reShadowing(FrameObject *frame, IplImage *background);
/*!
//void SaveDetectedToImage(FrameObject* currentFrame,bool three)
//Save the detected object of one frame into a file 
// 	@param[in] currentFrame the frame object
//	@param[in] three if true save in a three folder 
*/
void SaveDetectedToImage(FrameObject* currentFrame,bool three);

typedef struct _preprocessStruct{
	IplImage *frame,*background,*salient;
	_preprocessStruct() :
	frame(NULL), background(NULL),salient(NULL){}

	_preprocessStruct(IplImage* _frame,IplImage* _background,IplImage* _salient){
		frame = _frame;
		background = _background;
		salient = _salient;
	}

}preprocessStruct;

typedef struct _threadParam {
	list<preprocessStruct*> child;
	int numOfFirstFrame;
	int threadNum;
   // costruttore della struttura
	_threadParam() : 
   	child(NULL){ }
    _threadParam(list<preprocessStruct*> _child,int n,int thread) : 
   	child(_child), numOfFirstFrame(n),threadNum(thread){ }
  }threadParam;