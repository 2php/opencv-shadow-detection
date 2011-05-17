#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"
#include <iostream>
#include <string>

#define TL 15

static initializationParams initPar;
void Start(initializationParams par,string path);

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