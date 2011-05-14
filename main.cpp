#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"
#include "initializationParams.h"
#include <log4cxx\logger.h>
#include <log4cxx\xml\domconfigurator.h>
#include <iostream>

#define POOL 400
#define TL 15
#define video "C:/Users/Paolo/Videos/3.avi"


using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;


LoggerPtr loggerDelivery(Logger::getLogger( "Delivery"));
LoggerPtr loggerMain(Logger::getLogger( "main"));

/*!
//lista dei frame elaborati*/
static list<FrameObject*> frame;
static list<FrameObject> ordered;
static vector<HANDLE> handle;
static HANDLE mutex,newFrame,started;
volatile int size;
CThreadPool threadPool = CThreadPool(POOL,TRUE,INFINITE);
initializationParams initPar;

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
		WaitForSingleObject(mutex,INFINITE);
		list<FrameObject*>::iterator x;
		for(x=myFrames.begin(); x != myFrames.end(); x++){
			frame.push_back(dynamic_cast<FrameObject*>(*x));
			
		}
		SetEvent(started);
		ReleaseMutex(mutex);
	}else{
		WaitForSingleObject(handle.at(pa.threadNum),INFINITE);
		WaitForSingleObject(mutex,INFINITE);
		list<FrameObject*>::iterator j;
		for(j=myFrames.begin(); j != myFrames.end(); j++){
			frame.push_back(dynamic_cast<FrameObject*>(*j));
		}
		SetEvent(started);
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

template <class T>
string to_string(T t, int n,bool saving = FALSE)
{
  std::ostringstream oss;
  if(saving) 
	  oss << ".\\detected\\frame" << t << "_object" << n << ".jpg";
  else oss << n << t;
  return oss.str();
}

DWORD WINAPI Delivery(void *param)
{ 
	DWORD wait;
	int count = 1;
	//Parametri delle immagini memorizzate su disco
	int p[3];    
	p[0] = CV_IMWRITE_JPEG_QUALITY;
	p[1] = 90;
	p[2] = 0;
	
	LOG4CXX_INFO(loggerDelivery, "Delivery thread started...");
	WaitForSingleObject(started,INFINITE);
	while(count <= size){
		LOG4CXX_DEBUG(loggerDelivery,"Waiting for new frame list arrival...");
		WaitForSingleObject(started,INFINITE);
		wait = WaitForSingleObject(mutex,INFINITE);
		LOG4CXX_DEBUG(loggerDelivery,"New frame list delivery process started.");
		list<FrameObject*>::iterator j;
		list<DetectedObject*>::iterator i;
		list<DetectedObject*> det;

		try{
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
					cvSaveImage(to_string((*j)->getFrameNumber(),n,TRUE).c_str(),result,p);

					cvReleaseImage(&salient);
					cvReleaseImage(&frame);
					cvReleaseImage(&result);
				}
			}

			for(j=frame.begin(); j != frame.end(); j++){
				(*j)->~FrameObject();
			}

			LOG4CXX_DEBUG(loggerDelivery,to_string("# list processed correctly.",count));
			count++;
		}
		catch(exception& e){
			LOG4CXX_ERROR(loggerDelivery,e.what());
		}

		frame.clear();
		ReleaseMutex(mutex);
	}

	LOG4CXX_INFO(loggerDelivery,to_string(" icompleted frame !!ATTENTION!!",count-1 - size));
	threadPool.Destroy();
	return 0;
}

int main ( int argc, char **argv )
{

	DOMConfigurator::configure("Log4cxxConfig.xml");
    LOG4CXX_TRACE(loggerMain, "debug message (detailed)");
    LOG4CXX_DEBUG(loggerMain, "debug message");
    LOG4CXX_INFO (loggerMain, "info message");
    LOG4CXX_WARN (loggerMain, "warn message");
    LOG4CXX_ERROR(loggerMain, "error message");
    LOG4CXX_FATAL(loggerMain, "fatal message!!!");
    
	string response = "";

	initPar =  _initializationParams();
	cout << "DEFAULT PARAMS:\n\nTHRESHOLD: " << initPar.THRESHOLD << "\nK: " << initPar.K << "\nalfa: auto\nbeta: auto\nTh: auto\nTs: auto";

	cout << "\n\nUse default settings? (y/n): ";
	cin >> response;

	if(response == "y")
	{
		cout << "\nDefault settings loaded"<<endl;
	}else{
		initPar.useDefault = 0;
		cout << "\nDefine THREAD_NUM: ";
		cin >> initPar.THREAD_NUM;
		//threshold deve essere compreso tra 0 e 250
		do{
			cout << "\nDefine THRESHOLD con un valore compreso tra 0 e 250: ";//0 250
			cin >> initPar.THRESHOLD;
		}while(initPar.THRESHOLD<250 && initPar.THRESHOLD>0);
		//alfa deve essere compreso tra 0 e 1
		do{
			cout << "Define alfacon un valore compreso tra 0 e 1: "; 
			cin >> initPar.alfa;
		}while(initPar.alfa>0 && initPar.alfa<1);
		//beta deve essere compreso tra 0 e 1, inoltre deve essere maggiore di alfa
		do {
			cout << "Define beta con un valore compreso tra 0 e 1: "; 
			cin >> initPar.beta;
		}while (initPar.beta>0 && initPar.beta<1 && initPar.alfa<initPar.beta);
		cout << "Define Th: ";
		cin >> initPar.Th;
		cout << "Define Ts: ";
		cin >> initPar.Ts;
		cout << "Define K: ";
		cin >> initPar.K;
	}

size = 10000000;
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
started = CreateEvent(NULL,FALSE,FALSE,NULL);

list<IplImage*>sal;
list<IplImage*>list;

int cicle_num = 0;
int count=0;
int first=0;
int thread_num=0;
int index;
int flag = TRUE;
threadPool.Run(Delivery,NULL,High);
//_beginthread(Delivery,0,NULL);
while(thread_num<initPar.THREAD_NUM && flag){
	sal.clear();
	list.clear();
	first = count;
	cicle_num = numOfTotalFrames/initPar.THREAD_NUM;
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
	threadPool.Run(Thread,(void*)new _threadParam(cvCloneImage(bgModel->background),list,first,thread_num,sal),Low);
	//_beginthread((void)Thread,0,new _threadParam(cvCloneImage(bgModel->background),list,first,thread_num,sal));
	if(thread_num%10==0 && thread_num !=0){ 
		LOG4CXX_DEBUG(loggerMain,"w");
		WaitForSingleObject(handle.at(thread_num-1),INFINITE);
		LOG4CXX_DEBUG(loggerMain,"-w");
	}	
	thread_num++;
/****************BLOB ANALYSIS*****************/
///////bisogna passare le maschere degli oggetti...

}
cvReleaseCapture(&capture);
handle.push_back(CreateEvent( NULL, FALSE, FALSE, NULL ));

size = thread_num-1;
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
while(TRUE) {}
//WAIT_CHILD;
return 0;
}

