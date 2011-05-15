#include "home.h"

LoggerPtr loggerDelivery(Logger::getLogger( "Delivery"));
LoggerPtr loggerMain(Logger::getLogger( "main"));
LoggerPtr loggerThread(Logger::getLogger( "Thread"));

/*!
//lista dei frame elaborati*/
static list<FrameObject*> frame;
static list<FrameObject> ordered;
static vector<HANDLE> handle;
static HANDLE mutex,started;
volatile int size;
bool thread_saving;

//Parametri delle immagini memorizzate su disco
int p[3];    	



template <class T>
string to_string(T t, int n,bool saving = FALSE)
{
  std::ostringstream oss;
  if(saving) 
	  oss << ".\\detected\\frame" << t << "_object" << n << ".jpg";
  else oss << n << t;
  return oss.str();
}

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

void SaveDetectedToImage(FrameObject* currentFrame){
		list<DetectedObject*>::iterator i;
		list<DetectedObject*> det;
		try{
			det = currentFrame->getDetectedObject();
			int n = 0;
			for(i=det.begin(); i != det.end(); ++i){
				//reShadowing((*j),bgModel);
				n++;
				IplImage *frame=currentFrame->getFrame();
				IplImage *result=currentFrame->getFrame();
				IplImage *salient=currentFrame->getSalientMask();
				cvZero(result);
				cvOr(frame,result,result,(*i)->mvoMask);		
				cvSaveImage(to_string(currentFrame->getFrameNumber(),n,TRUE).c_str(),result,p);

				cvReleaseImage(&salient);
				cvReleaseImage(&frame);
				cvReleaseImage(&result);
			}
		}
		catch (exception& e){
			throw e.what();
		}
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
	
	LOG4CXX_INFO(loggerThread,to_string("Thread ", pa.threadNum));
	for(j=pa.frameList.begin(),s=pa.salient.begin(); j != pa.frameList.end(), s != pa.salient.end(); j++,s++){
		temp = new FrameObject((*j),pa.background, (*s),count);
		count++;
		temp->detectAll(initPar);
		if(thread_saving==TRUE){
			LOG4CXX_DEBUG(loggerThread,"Saving detected to file");
			SaveDetectedToImage(temp);
			temp->~FrameObject();
		}
		else{	
			myFrames.push_back(dynamic_cast<FrameObject*>(temp));
		}
 	}
	if(thread_saving==TRUE) {
		LOG4CXX_INFO(loggerThread,to_string("Stopping thread ",pa.threadNum));	
		int repeat = 0;
		while(TRUE && repeat<10){
			if(handle.size() > pa.threadNum+1) break;
			Sleep(500);
			repeat++;
		}

		if(handle.size() < pa.threadNum+1) return 0;
		
		LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " set event to "<< pa.threadNum+1);
		SetEvent(handle.at(pa.threadNum+1));
		return 0;
	}
	LOG4CXX_INFO(loggerThread,"Thread n# "<< pa.threadNum << " -> Prepare to accode frame for delivery service");
	//soggetto a errori se ancora non esiste l'handle su cui fa SetEvent
	if(pa.threadNum%10 != 0){
		LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " waiting previous thread completition");
		WaitForSingleObject(handle.at(pa.threadNum),INFINITE);
		LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " previous thread completition success");
	}

	LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " waiting mutex");
	WaitForSingleObject(mutex,INFINITE);
	LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " waiting mutex success");
	int suc = 0;
	list<FrameObject*>::iterator x;
	for(x=myFrames.begin(); x != myFrames.end(); x++){
		frame.push_back(dynamic_cast<FrameObject*>(*x));
		suc++;
	}
	SetEvent(started);
	ReleaseMutex(mutex);
	LOG4CXX_INFO(loggerThread,"Thread n# " << pa.threadNum << ": " << suc << " on "<< pa.frameList.size() << " accoded");
	while(TRUE){
			if(handle.size() > pa.threadNum+1) break;	
		}
	LOG4CXX_DEBUG(loggerThread,"Thread n# " << pa.threadNum << " set event to "<< pa.threadNum+1);
	SetEvent(handle.at(pa.threadNum+1));
	return 0;
}

bool nframeSorting ( FrameObject first, FrameObject second)
{
	if (first.getFrameNumber()<second.getFrameNumber()) return true;
    else return false;
}


DWORD WINAPI Delivery(void *param)
{ 
	DWORD wait;
	int count = 1;
	if(thread_saving==TRUE){
		LOG4CXX_INFO(loggerDelivery,"Servizio di Delivery disabilitato");
		return 0;
	}

	LOG4CXX_INFO(loggerDelivery, "Delivery thread started...");
	WaitForSingleObject(started,INFINITE);
	while(count <= size){
		LOG4CXX_DEBUG(loggerDelivery,"Waiting for new frame list arrival...");
		WaitForSingleObject(started,INFINITE);
		wait = WaitForSingleObject(mutex,INFINITE);
		LOG4CXX_DEBUG(loggerDelivery,"Frame list (number "<< count << ") delivery process started.");
		list<FrameObject*>::iterator j;
		list<DetectedObject*>::iterator i;
		list<DetectedObject*> det;

		try{
			for(j=frame.begin(); j != frame.end(); j++){
				SaveDetectedToImage((*j));
				(*j)->~FrameObject();
			}

			LOG4CXX_DEBUG(loggerDelivery,"list " << count << " processed correctly.");
			count++;
		}
		catch(exception& e){
			LOG4CXX_ERROR(loggerDelivery,e.what());
		}

		frame.clear();
		ReleaseMutex(mutex);
	}
	if((count-1 - size) != 0)
		LOG4CXX_WARN(loggerDelivery,"!!ATTENTION!! " << count-1 - size <<  "icompleted frame" );
	SetEvent(handle.at(0));
	return 0;
}

void Start ()
{
	CThreadPool threadPool = CThreadPool(initPar.POOL,TRUE,INFINITE);	
	int cicle_background = initPar.cicle_background;
	//parametri salvataggio immaggini
	p[0] = CV_IMWRITE_JPEG_QUALITY;
	p[1] = 90;
	p[2] = 0;
	thread_saving = initPar.thread_saving;
	string video_path = "";
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


    CvCapture* capture = cvCaptureFromAVI(video);

	while( !capture )
    {
		LOG4CXX_WARN(loggerMain,"Could not open video file");
		cout << "Insert a valid video path (type exit to quit the program): ";
		cin >> video_path;
		capture = cvCaptureFromAVI(video_path.c_str());
    }

int nframe = 0;
cvGrabFrame(capture);
img = cvCloneImage(cvRetrieveFrame(capture));
const int numOfTotalFrames = (int) cvGetCaptureProperty( capture , CV_CAP_PROP_FRAME_COUNT );
//cameraCorrection(img,img,MEDIAN,1.1,5);

CvBGStatModel* bgModel = cvCreateFGDStatModel(img,para);
cvUpdateBGStatModel(img,bgModel);
//Crea un modello del backgroud
LOG4CXX_INFO(loggerMain,"Apprendimento background...");
for(;nframe<cicle_background;nframe++){
	cvGrabFrame(capture);
	img = cvRetrieveFrame(capture);
	//cameraCorrection(img,img,MEDIAN,1.1,5);
	if(nframe%10==0){
	cvUpdateBGStatModel(img,bgModel);
	cout << ".";
	}
}
LOG4CXX_INFO(loggerMain,"Modello del background creato correttamente");
list<DetectedObject>::iterator i;
list<DetectedObject> det;

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
			if(cicle_background != 1)
				cvUpdateBGStatModel(img,bgModel);
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
		LOG4CXX_DEBUG(loggerMain,"Thread checkpoint (LOCK): waiting previous thread completition (thread n# "<<thread_num<<"");
		WaitForSingleObject(handle.at(thread_num),INFINITE);
		LOG4CXX_DEBUG(loggerMain,"Thread checkpoint (UNLOCK): continuing execution...");
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
if(thread_saving==FALSE)
	WaitForSingleObject(handle.at(0),INFINITE);
LOG4CXX_INFO(loggerMain,"Chisura applicazione...")
//threadPool.Destroy();
}

