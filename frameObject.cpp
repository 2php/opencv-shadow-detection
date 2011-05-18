//#include "shadow.h"
#include "frameObject.h"
//#define TH 30
#define TL 10

LoggerPtr loggerFrameObject(Logger::getLogger( "FrameObject"));

FrameObject::FrameObject(){
	frame = NULL; 
	background = NULL; 
	foregroundMask = NULL;
	salientForegroundMask = NULL;
	frameNumber = 0;
	size.height=0;
	size.width=0;
}

FrameObject::~FrameObject(){
	try{
		cvReleaseImage(&frame);
		cvReleaseImage(&background);
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
		LOG4CXX_ERROR(loggerFrameObject, "Error in distruction FrameObject: "<< e.what());
	}
}

FrameObject::FrameObject(IplImage * currentFrame, IplImage * currentBackground,IplImage *salient, int nFrame = 1){
	try{
		size = cvGetSize(currentFrame);
		frame = currentFrame;
		background = currentBackground;
		foregroundMask = cvCreateImage(size, currentFrame->depth,1);
		salientForegroundMask = salient;
		frameNumber = nFrame;
		frameBlobs = CvBlobs();
	}
	catch(exception& e){
		LOG4CXX_ERROR(loggerFrameObject, e.what());
	}

}

IplImage* FrameObject::getFrame(){return cvCloneImage(frame);}
IplImage* FrameObject::getBackground(){return cvCloneImage(background);}
IplImage* FrameObject::getForegroundMask(){return foregroundMask;}
IplImage* FrameObject::getSalientMask(){return salientForegroundMask;}
int FrameObject::getFrameNumber(){return frameNumber;}

bool FrameObject::isToSave(IplImage *vMask,IplImage *mask,int threashold){
	int _threashold=0;
	uchar* data_vMask = (uchar *)vMask->imageData;
	int step_vMask = vMask->widthStep/sizeof(uchar);
	uchar* data_mask = (uchar *)mask->imageData;
	int step_mask = mask->widthStep/sizeof(uchar);
	
	for(int i=0; i<vMask->height;i++){
		for(int j=0; j<vMask->width;j++){
				if((float)data_mask[i*step_mask+j] == 255){
					_threashold++;
				}
		}
	}

	if(_threashold<threashold) return FALSE;
	return TRUE;
}

void FrameObject::detectAll(initializationParams initPar){
		LOG4CXX_DEBUG(loggerFrameObject , "Detection All");
		int fitting=initPar.fitting;
		try{
			IplImage * img =  getFrame();
			IplImage * background = getBackground();
			DetectedObject *temp = new DetectedObject(size,img->depth);

			//cameraCorrection(img,img,MEDIAN,1.1,5);
			double TH = initPar.THRESHOLD;					
			backgroundSuppression(img,background,this->foregroundMask);
			
			//cvThreshold(this->foregroundMask,this->salientForegroundMask,TL,255,CV_THRESH_BINARY);
			cvThreshold(this->foregroundMask,this->foregroundMask,TH,255,CV_THRESH_BINARY);
			LOG4CXX_TRACE(loggerFrameObject, "Background mask created");
			
			//elemento strutturante
			LOG4CXX_TRACE(loggerFrameObject, "Morphing correction");
			IplConvKernel *element=cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_ELLIPSE, NULL);
			//operazione morfolologica del gradiente, OPEN per migliorare la maschera
			cvMorphologyEx (this->foregroundMask, this->foregroundMask, NULL, element, CV_MOP_OPEN, 1);
			cvReleaseStructuringElement(&element);
			LOG4CXX_TRACE(loggerFrameObject, "Morphing correction completed");
			//estraggo le blob relative alla scena
			IplImage *labelImg=cvCreateImage(size, IPL_DEPTH_LABEL, 1);
			unsigned int labeled=cvLabel(this->foregroundMask, labelImg,this->frameBlobs);
			LOG4CXX_TRACE(loggerFrameObject, "Area Filtering");			
			cvFilterByArea(this->frameBlobs,200,1000000);

			IplImage *src = cvCreateImage(size,8,3);

			//salvo i valori relativi all'oggetto :
			//    creo la mashera di ogni blob e divido ombra da oggetto
			LOG4CXX_DEBUG(loggerFrameObject, "salvo i valori relativi all'oggetto, creo la mashera di ogni blob e divido ombra da oggetto");
			for (CvBlobs::const_iterator it=this->frameBlobs.begin(); it!=this->frameBlobs.end(); ++it)
			{
				cvZero(src);
				cvRenderBlob(labelImg,it->second,img,src,CV_BLOB_RENDER_COLOR);
			
				cvZero(temp->totalMask);
				cvZero(temp->shadowMask);

				cvCvtColor(src,temp->totalMask,CV_RGB2GRAY);
				//maschera dell'ombra
				shadowDetection(img,background,temp->totalMask,temp->shadowMask,initPar);
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

				cvOr(frame,temp->mvo,temp->mvo,temp->mvoMask); 
			//IplImage * t = cvCloneImage(temp.mvoMask);
			//uchar* datat    = (uchar *)t->imageData;
			//int stept = t->widthStep/sizeof(uchar);

			//for(int i=0; i<t->height;i++){
			//	for(int j=0; j<t->width;j++){
			//		datat[i*stept+j] = 255;
			//	}
			//}
				IplImage * verityMask=cvCloneImage(temp->mvoMask);
				cvZero(verityMask);
				cvAnd(temp->mvoMask,this->getSalientMask(),verityMask,temp->mvoMask);
				if(isToSave(verityMask,temp->mvoMask,fitting)){
				//salvo il risultato nella lista
					detectedObject.push_front(temp);
					temp = new DetectedObject(size,img->depth);
				}
				cvReleaseImage(&verityMask);
			}
		
		temp->~_DetectedObject();
		cvReleaseImage(&labelImg);
		cvReleaseImage(&img);
		cvReleaseImage(&background);
		cvReleaseImage(&src);	
		}
		catch(exception& e)
		{
			LOG4CXX_ERROR(loggerFrameObject, "Frame number " << this->frameNumber << " losted: \n-\t" << e.what());
		}
		LOG4CXX_DEBUG(loggerFrameObject, "Detection completed: " << this->detectedObject.size() << " object detected.");

}

list<DetectedObject*> FrameObject:: getDetectedObject(){
	return detectedObject;
}
