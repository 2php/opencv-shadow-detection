//#ifndef _FrameObject_
//#define _FrameObject_
#include <cv.h>
#include <list>
#include "shadow.h"
#include "initializationParams.h"

using namespace std;

/*!
//@typedef DetectedObject
*/
typedef struct _DetectedObject
{
	/*!
	//Maschera dell'oggetto con la sua ombra*/
	IplImage * totalMask;	
	/*!
	//Maschera dell'oggetto*/
	IplImage * mvoMask;
	/*!
	//Maschera dell'ombra*/
	IplImage * shadowMask;
	/*!
	//Maschera invertita dell'ombra*/
	IplImage * invertedShadowMask;
	/*!
	//Blobs dell'mvo*/	
	CvBlobs mvoBlobs;
	_DetectedObject(CvSize size,int depth){
		totalMask = cvCreateImage(size,depth,1);				
		mvoMask = cvCreateImage(size,depth,1);		
		shadowMask = cvCreateImage(size,depth,1);
		invertedShadowMask = cvCreateImage(size,depth,1);
		mvoBlobs = CvBlobs();
	}
	
	virtual ~_DetectedObject(){
		try{
			cvReleaseImage(&totalMask);
			cvReleaseImage(&mvoMask);
			cvReleaseImage(&shadowMask);
			cvReleaseImage(&invertedShadowMask);
			cvReleaseBlobs(mvoBlobs);
		}
		catch(exception& e){
			throw e.what();
		}
	}
}
DetectedObject;


/*!
//Classe wrap per un singolo frame*/
class FrameObject{
private:
	/*!
	//il frame*/
	IplImage * frame;
	/*!
	//il background*/
	IplImage * background;
	/*!
	//maschera del foreground del frame*/
	IplImage * foregroundMask;
	/*!
	//maschera del salient foreground*/
	IplImage * salientForegroundMask;
	/*!
	//lista contenente i singoli oggetti rilevati nel frame*/
	list<DetectedObject*> detectedObject;
	/*!
	//numero del frame*/
	int frameNumber;
	/*!
	//dimensione in pizel del frame*/
	CvSize size;
	/*!
	//Blobs della scena*/	
	CvBlobs frameBlobs;

public:
	/*!
	//default constructor*/
	FrameObject();
	~FrameObject();
	/*!
	//constructor with parameter*/
	FrameObject(IplImage * currentFrame, IplImage * currentBackground, IplImage* salient,int nFrame);	
	/*!
	//ritorna una copia del frame*/
	IplImage * getFrame();
	/*!
	//ritorna una copia del background*/
	IplImage * getBackground();
	/*!
	//ritorna una copia della maschera del foreground del frame*/
	IplImage * getForegroundMask();
	IplImage * getSalientMask();
	/*!
	//ritorna il numero di sequenza del frame*/
	int getFrameNumber();
	/*!
	//esegue il detecting di tutti gli oggetti nella scena e ne salva Mvo e shadow nella lista detectedObject*/
	void detectAll(initializationParams initPar);
	/*!
	//ritorna la lista degli ogetti trovati nella scena*/
	list<DetectedObject*> getDetectedObject();

};

//#endif