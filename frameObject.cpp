//#include "shadow.h"
#include "frameObject.h"
#define TH 30
#define TL 10

FrameObject::FrameObject(){
	frame = NULL; 
	background = NULL; 
	foregroundMask = NULL;
	salientForegroundMask = NULL;
	frameNumber = 0;
}

FrameObject::~FrameObject(){
	try{
		cvReleaseImage(&frame);
		cvReleaseImage(&foregroundMask);
		cvReleaseImage(&salientForegroundMask);
		cvReleaseBlobs(frameBlobs);
		list<DetectedObject*>::iterator i;
	
		for(i=detectedObject.begin();i!=detectedObject.end(); i++){
			(*i)->~_DetectedObject();
		}
	}
	catch(exception& e){
		throw e.what();
	}
}

FrameObject::FrameObject(IplImage * currentFrame, IplImage * currentBackground,IplImage *salient, int nFrame = 1){
	size = cvGetSize(currentFrame);
	frame = currentFrame;
	background = currentBackground;
	foregroundMask = cvCreateImage(size, currentFrame->depth,1);
	salientForegroundMask = salient;
	frameNumber = nFrame;
	frameBlobs = CvBlobs();

}

IplImage* FrameObject::getFrame(){return cvCloneImage(frame);}
IplImage* FrameObject::getBackground(){return cvCloneImage(background);}
IplImage* FrameObject::getForegroundMask(){return cvCloneImage(foregroundMask);}
IplImage* FrameObject::getSalientMask(){return cvCloneImage(salientForegroundMask);}
int FrameObject::getFrameNumber(){return frameNumber;}

void FrameObject::detectAll(){
		IplImage * img =  getFrame();
		IplImage * background = getBackground();
		DetectedObject *temp = new DetectedObject(size,img->depth);
		
		//cameraCorrection(img,img,MEDIAN,1.1,5);
		
		//rimuovo il background, ottengo la maschera del foreground
		backgroundSuppression(img,background,this->foregroundMask);
		//cvThreshold(this->foregroundMask,this->salientForegroundMask,TL,255,CV_THRESH_BINARY);
		cvThreshold(this->foregroundMask,this->foregroundMask,TH,255,CV_THRESH_BINARY);

		//elemento strutturante
		IplConvKernel *element=cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_ELLIPSE, NULL);
		//operazione morfolologica del gradiente, OPEN per migliorare la maschera
		cvMorphologyEx (this->foregroundMask, this->foregroundMask, NULL, element, CV_MOP_OPEN, 1);
		cvReleaseStructuringElement(&element);
		//estraggo le blob relative alla scena
		IplImage *labelImg=cvCreateImage(size, IPL_DEPTH_LABEL, 1);
		unsigned int labeled=cvLabel(this->foregroundMask, labelImg,this->frameBlobs);
		cvFilterByArea(this->frameBlobs,200,1000000);

		IplImage *src = cvCreateImage(size,8,3);

		//salvo i valori relativi all'oggetto :
		//    creo la mashera di ogni blob e divido ombra da oggetto
		for (CvBlobs::const_iterator it=this->frameBlobs.begin(); it!=this->frameBlobs.end(); ++it)
        {
			cvZero(src);
			cvRenderBlob(labelImg,it->second,img,src,CV_BLOB_RENDER_COLOR);
			
			cvZero(temp->totalMask);
			cvZero(temp->shadowMask);

			cvCvtColor(src,temp->totalMask,CV_RGB2GRAY);
			//maschera dell'ombra
			shadowDetection(img,background,temp->totalMask,temp->shadowMask);
			//maschera invertita dell'ombra
			cvThreshold(temp->shadowMask,temp->invertedShadowMask,120,255,CV_THRESH_BINARY_INV);
			//and tra la maschera del foreground e la maschera invertita dell'ombra
			//               ottengo la maschera del MVO
			cvAnd(temp->totalMask,temp->invertedShadowMask,temp->mvoMask);
			//creo l'ogetto blob dell'ogetto necessario per il traking
			IplImage *label=cvCreateImage(size, IPL_DEPTH_LABEL, 1);
			unsigned int labelResult=cvLabel(temp->mvoMask, label, temp->mvoBlobs);

					/*LA BLOB DEVE ESSERE UN SALIENT FOREGROUND*/
			cvReleaseImage(&label);
			
		//IplImage * t = cvCloneImage(temp.mvoMask);
		//uchar* datat    = (uchar *)t->imageData;
		//int stept = t->widthStep/sizeof(uchar);

		//for(int i=0; i<t->height;i++){
		//	for(int j=0; j<t->width;j++){
		//		datat[i*stept+j] = 255;
		//	}
		//}

		//cvOr(temp.mvoMask,this->salientForegroundMask,temp.totalMask,temp.mvoMask);
			//salvo il risultato nella lista
			detectedObject.push_front(temp);
			temp =  new DetectedObject(size,img->depth);
        }

	cvReleaseImage(&labelImg);
	cvReleaseImage(&img);
	cvReleaseImage(&background);
	cvReleaseImage(&src);	
}

list<DetectedObject*> FrameObject:: getDetectedObject(){
	return detectedObject;
}
