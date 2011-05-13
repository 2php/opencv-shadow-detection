#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"

#define THREAD_NUM 30
#define POOL 30
#define TL 15
#define video "C:/Users/Paolo/Videos/prova12.avi"

/*!
//lista dei frame elaborati*/
static list<FrameObject*> frame;
static list<FrameObject> ordered;
static vector<HANDLE> handle;
static HANDLE mutex,newFrame;
volatile long n;

void reShadowing(FrameObject frame, CvBGStatModel *bgModel) {
	IplImage *source = cvCloneImage(frame.getFrame());
	CvSize size = cvGetSize(source);
	IplImage *ghostMask = cvCreateImage(size,8,1);
	IplImage *src = cvCreateImage(size,8,3);
	IplImage *result = cvCloneImage(bgModel->background);
	cvZero(ghostMask);
	cvZero(src);

	//al momento costruisce una maschera dalle blob...si potrebbe passare direttamente la maschera dei ghost e saltare questo ciclo
	//for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
 //       {
	//		cvRenderBlob(labelImg,it->second,source,src,CV_BLOB_RENDER_COLOR);
 //       }

	
	list<DetectedObject*>::iterator it;
	list<DetectedObject*> det;
	//det = frame.getDetectedGhost();
	det = frame.getDetectedObject();
	///
	for(it=det.begin(); it != det.end(); ++it){
		cvOr(ghostMask,(*it)->totalMask,ghostMask);
	}

	int channel = source->nChannels;

	uchar* dataSrc = (uchar *)source->imageData;
	int stepSrc = source->widthStep/sizeof(uchar);
	
	uchar* dataBkg= (uchar *)result->imageData;
	int stepBkg = result->widthStep/sizeof(uchar);
	
	uchar* dataGhost = (uchar *)ghostMask->imageData;
	int stepGhost = ghostMask->widthStep/sizeof(uchar);
	
	/*uchar* dataS = (uchar *)S->imageData;
	int stepS = S->widthStep/sizeof(uchar);
	
	uchar* dataV    = (uchar *)V->imageData;
	int stepV = V->widthStep/sizeof(uchar);
	
	uchar* databH = (uchar *)bH->imageData;
	int stepbH = bH->widthStep/sizeof(uchar);
	
	uchar* databS = (uchar *)bS->imageData;
	int stepbS = bS->widthStep/sizeof(uchar);
	
	uchar* databV    = (uchar *)bV->imageData;
	int stepbV = bV->widthStep/sizeof(uchar);*/

	int i,j,k;

	for(i=0; i<src->height;i++){
		for(j=0; j<src->width;j++){
			if((float)dataGhost[i*stepGhost+j] == 255){
				for(k=0;k<channel;k++)
					dataBkg[i*stepBkg+j*channel+k]=dataSrc[i*stepSrc+j*channel+k];
				
			}
		}
	}
	
	cvShowImage("result",result);
	cvWaitKey(1);

	cvUpdateBGStatModel(result,bgModel);

}

typedef struct _threadParam {
	IplImage* background;
	list<IplImage*> salient;
	list<IplImage*> frameList;
	int numOfFirstFrame;
	int threadNum;
   // costruttore della struttura
	_threadParam() : 
   	background(NULL), frameList(NULL){ }
    _threadParam(IplImage* _background,list<IplImage*> lista,int n,int thread,list<IplImage*> _salient) : 
   	background(_background), frameList(lista), numOfFirstFrame(n),threadNum(thread),salient(_salient){ }
  }threadParam;

DWORD WINAPI Thread(void* param)
  { 
	UserPoolData* poolData = (UserPoolData*)param;
	LPVOID myData = poolData->pData;
	FrameObject *temp;
	threadParam &pa=*static_cast<threadParam *>(myData);
	list<FrameObject*> myFrames;
	int count = pa.numOfFirstFrame;
	list<IplImage*>::iterator j;
	list<IplImage*>::iterator s;
	cameraCorrection(pa.background,pa.background,MEDIAN,1.1,5);

	for(j=pa.frameList.begin(),s=pa.salient.begin(); j != pa.frameList.end(), s != pa.salient.end(); j++,s++){
		temp = new FrameObject((*j),pa.background, (*s),count);
		count++;
		temp->detectAll();
		myFrames.push_back(dynamic_cast<FrameObject*>(temp));
	}

	//soggetto a errori se ancora non esiste l'handle su cui fa SetEvent
	if(pa.threadNum == 0){
		list<FrameObject*>::iterator x;
		for(x=myFrames.begin(); x != myFrames.end(); x++){
			frame.push_back(dynamic_cast<FrameObject*>(*x));
		}

	}else{
		WaitForSingleObject(handle.at(pa.threadNum),INFINITE);
		WaitForSingleObject(mutex,INFINITE);
		list<FrameObject*>::iterator j;
		for(j=myFrames.begin(); j != myFrames.end(); j++){
			frame.push_back(dynamic_cast<FrameObject*>(*j));
		}
		ReleaseMutex(mutex);
	}

	//WaitForSingleObject(,INFINITE);
	//	frame.push_back(temp);
	//	SetEvent(newFrame);
	//ReleaseMutex(mutex);

	if(handle.size() > pa.threadNum+1) SetEvent(handle.at(pa.threadNum+1));
	return 0;
}

bool nframeSorting ( FrameObject first, FrameObject second)
{
	if (first.getFrameNumber()<second.getFrameNumber()) return true;
    else return false;
}

bool flag = TRUE;

template <class T>
string to_string(T t, int n)
{
  std::ostringstream oss;
  oss << "frame_" << t << "_" << n << ".jpg";
  return oss.str();
}

void Delivery(void *param)
{
	while(TRUE){
	WaitForSingleObject(mutex,INFINITE);
	list<FrameObject*>::iterator j;
	list<DetectedObject*>::iterator i;
	list<DetectedObject*> det;

	int p[3];    
	p[0] = CV_IMWRITE_JPEG_QUALITY;
	p[1] = 10;
	p[2] = 0;

	for(j=frame.begin(); j != frame.end(); j++){
		det = (*j)->getDetectedObject();
		int n = 0;
		for(i=det.begin(); i != det.end(); ++i){
			//reShadowing((*j),bgModel);
			n++;
			IplImage *frame=(*j)->getFrame();
			IplImage *result=(*j)->getFrame();
			IplImage *salient=(*j)->getSalientMask();
			cvZero(result);
			cvOr(frame,result,result,(*i)->mvoMask);
			//cvShowImage("mvo",bgModel->background);			
			cvSaveImage(to_string((*j)->getFrameNumber(),n).c_str(),result,p);

			//cvShowImage("shadow",(*i)->shadowMask);
			//cvShowImage("mvo",result);
			//cvShowImage("frame",frame);
			//cvShowImage("salient",salient);
			//cvWaitKey(0);
			cvReleaseImage(&salient);
			cvReleaseImage(&frame);
			cvReleaseImage(&result);
	/*		delete (*j);*/
		}
	}

	//*********LE RIGHE INCRIMINATE...***********/
	for(j=frame.begin(); j != frame.end(); j++){
		(*j)->~FrameObject();
	}
	frame.clear();
	ReleaseMutex(mutex);
	}
}

int main ( int argc, char **argv )
{
int MAX_THREAD_NUM = 0;
mutex = CreateMutex(NULL,FALSE,NULL);
//IplImage *bkg = cvLoadImage("C:/Users/Paolo/Pictures/sharp.jpg",1);
//IplImage *test = cvLoadImage("C:/Users/Paolo/Pictures/test.png",1);
IplImage *img = NULL;
list<IplImage*>::iterator it;

CvFGDStatModelParams* para = new CvFGDStatModelParams;
 para->alpha1=0.1f;
 para->alpha2=0.005f;
 para->alpha3=0.1f;
 para->delta=2;
 para->is_obj_without_holes=TRUE;
 para->Lc=128;
 para->Lcc=64;
 para->minArea=15.f;
 para->N1c=15;
 para->N1cc=25;
 para->N2c=25;
 para->N2cc=40;
 para->perform_morphing=1;
 para->T=0.9f;


    CvCapture* capture = 0;

    if(1==0 && argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if(1==1 || argc == 2 )
		capture = cvCaptureFromAVI(video);
        /*capture = cvCaptureFromAVI( argv[1] );*/

    if( !capture )
    {
        fprintf(stderr,"Could not initialize...\n");
        return -1;
    }

int nframe = 0;
cvGrabFrame(capture);
img = cvCloneImage(cvRetrieveFrame(capture));
const int numOfTotalFrames = (int) cvGetCaptureProperty( capture , CV_CAP_PROP_FRAME_COUNT );
//cameraCorrection(img,img,MEDIAN,1.1,5);

CvBGStatModel* bgModel = cvCreateFGDStatModel(img,para);
cvUpdateBGStatModel(img,bgModel);
//Crea un modello del backgroud
for(;nframe<1;nframe++){
	cvGrabFrame(capture);
	img = cvRetrieveFrame(capture);
	//cameraCorrection(img,img,MEDIAN,1.1,5);
	if(nframe%10==0){
	cvUpdateBGStatModel(img,bgModel);
	}
}

cvNamedWindow("Background Model",0);
cvShowImage("Background Model",bgModel->background);
cvWaitKey(0);
cvDestroyWindow("Background Model");

list<DetectedObject>::iterator i;
list<DetectedObject> det;

newFrame = CreateEvent(NULL,FALSE,FALSE,NULL);

CThreadPool threadPool = CThreadPool(POOL,TRUE,INFINITE);

list<IplImage*>sal;
list<IplImage*>list;

MAX_THREAD_NUM = THREAD_NUM;
int cicle_num = 0;
int count=0;
int first=0;
int thread_num=0;
int index;
int flag = TRUE;
_beginthread(Delivery,0,NULL);
while(thread_num<MAX_THREAD_NUM && flag){
	sal.clear();
	list.clear();
	first = count;
	cicle_num = numOfTotalFrames/MAX_THREAD_NUM;
	index=0;
	flag = cvGrabFrame(capture);
	while (cicle_num>=0 && flag){
		img = cvRetrieveFrame(capture);
		if(index%10==0) {
			//cvUpdateBGStatModel(img,bgModel);
			cameraCorrection(img,img,MEDIAN,1.1,5);
			list.push_back(cvCloneImage(img));
			cicle_num--;
			count++;			
			sal.push_back(cvCloneImage(bgModel->foreground));
		}
		index++;
		flag = cvGrabFrame(capture);
	}	
	/*******MULTITHREAD**********/
	/*Gestione coda degli handle*/
	handle.push_back(CreateEvent( NULL, FALSE, FALSE, NULL ));
	/****************************/
	threadPool.Run(Thread,(void*)new _threadParam(cvCloneImage(bgModel->background),list,first,thread_num,sal));
	
	if(thread_num%5==0 && thread_num !=0) 
		WaitForSingleObject(handle.at(thread_num-1),INFINITE);
	thread_num++;
/****************BLOB ANALYSIS*****************/
///////bisogna passare le maschere degli oggetti...

}
cvReleaseCapture(&capture);

handle.push_back(CreateEvent( NULL, FALSE, FALSE, NULL ));

WaitForSingleObject(handle.at(thread_num),INFINITE);
//threadPool.CheckThreadStop();

/****************Reshadowing*****************/

/****blobanalysis****/
//int iu=0;
//list<FrameObject>::iterator j,prev;
//	for(j=frame.begin(); j != frame.end(); ++j){
//		iu++;
//		if((iu%2)==0){
//			prev=j;
//			prev--;
//			blobAnalysis((*j).getForegroundMask(),(*prev).getForegroundMask());
//		}
//	}

	//list<FrameObject>::iterator j;
	//for(j=frame.begin(); j != frame.end(); ++j){
	//	cvShowImage("",(*j).getForegroundMask());
	//		cvWaitKey(10);
	//}
while(flag){}
return 0;
}

