#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <cvblob.h>
#include <stdio.h> 
#include <exception>
#include <iostream>
#include <math.h>
#include "initializationParams.h"
#include <log4cxx\logger.h>
#include <log4cxx\xml\domconfigurator.h>

using namespace cvb;
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;

#define MEDIAN 21
#define HIGH_BOOST 33
#define SHARPENING 15

//Prototipi
//void cameraCorrection(IplImage* src,IplImage* dst,int typez= MEDIAN, double A=1.1, int size = 5);  //con paramtri di default non compila
void cameraCorrection(IplImage* src,IplImage* dst,int type, double A, int size);
void shadowDetection(IplImage *src, IplImage *background,IplImage *foregroundSelection,IplImage *result,initializationParams initPar);
void backgroundSuppression(IplImage *src, IplImage *background,IplImage *result);
void blobAnalysis(IplImage *imgA, IplImage *imgB);
//IplImage* reShadowing(FrameObject frame, IplImage *background);