#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <stdio.h> 
#include <exception>
#include <iostream>
#include <cvblob.h>
#include <math.h>

using namespace cvb;

#define MEDIAN 21
#define HIGH_BOOST 33
#define SHARPENING 15

//Prototipi
//void cameraCorrection(IplImage* src,IplImage* dst,int typez= MEDIAN, double A=1.1, int size = 5);  //con paramtri di default non compila
void cameraCorrection(IplImage* src,IplImage* dst,int type, double A, int size);
void shadowDetection(IplImage *src, IplImage *background,IplImage *foregroundSelection,IplImage *result);
void backgroundSuppression(IplImage *src, IplImage *background,IplImage *result);
void blobAnalysis(IplImage *imgA, IplImage *imgB);
//IplImage* reShadowing(FrameObject frame, IplImage *background);