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
int gap;

//Parametri delle immagini memorizzate su disco
int p[3];    	

void reShadowing(FrameObject *frame, IplImage *background) {
	IplImage *source = frame->getFrame();
	CvSize size = cvGetSize(source);
	IplImage *ghostMask = cvCreateImage(size,8,1);
	cvZero(ghostMask);

	//al momento costruisce una maschera dalle blob...si potrebbe passare direttamente la maschera dei ghost e saltare questo ciclo
	//for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
 //       {
	//		cvRenderBlob(labelImg,it->second,source,src,CV_BLOB_RENDER_COLOR);
 //       }

	
	list<DetectedObject*>::iterator it;
	list<DetectedObject*> det;
	//det = frame.getDetectedGhost();
	det = frame->getDetectedObject();
	///
	for(it=det.begin(); it != det.end(); ++it){
		if((*it)->isGhost){
			cvOr(ghostMask,(*it)->totalMask,ghostMask);
		}
	}

	int channel = source->nChannels;

	uchar* dataSrc = (uchar *)source->imageData;
	int stepSrc = source->widthStep/sizeof(uchar);
	
	uchar* dataBkg= (uchar *)background->imageData;
	int stepBkg = background->widthStep/sizeof(uchar);
	
	uchar* dataGhost = (uchar *)ghostMask->imageData;
	int stepGhost = ghostMask->widthStep/sizeof(uchar);

	int i,j,k;

	for(i=0; i<source->height;i++){
		for(j=0; j<source->width;j++){
			if((float)dataGhost[i*stepGhost+j] == 255){
				for(k=0;k<channel;k++)
					dataBkg[i*stepBkg+j*channel+k]=dataSrc[i*stepSrc+j*channel+k];
				
			}
		}
	}

	cvReleaseImage(&ghostMask);
	cvReleaseImage(&source);
}

bool isGhost(IplImage *detected){
	int res=0;
	cvNamedWindow("Supervisioning press Esc if is a ghost, else press enter",0);
	cvShowImage("Supervisioning press Esc if is a ghost, else press enter",detected);
	res = cvWaitKey(0);
	cvDestroyAllWindows();
	if(res == 27) return TRUE;
	else return FALSE;
}

void SaveDetectedToImage(FrameObject* currentFrame,bool three){
	list<DetectedObject*>::iterator i;
	list<DetectedObject*> det;
	IplImage *current=currentFrame->getFrame();
	try{
		det = currentFrame->getDetectedObject();
		int n = 0;
		stringstream filename;
		string temp,tempS;
		CreateDirectory("detected",NULL);

		for(i=det.begin(); i != det.end(); ++i){
			//reShadowing((*j),bgModel);
			n++;

			if(three){
				filename << "detected/frame"<< currentFrame->getFrameNumber();
				filename >> temp;
				CreateDirectory(temp.c_str(),NULL);
				filename.clear();
				filename << "detected/frame"<< currentFrame->getFrameNumber() <<"/frame.jpg"; 
				filename>>temp;
				cvSaveImage(temp.c_str(),current);
				filename.clear();
				filename << "detected/frame"<< currentFrame->getFrameNumber() << "/object" << n << ".jpg";
				filename >> temp;
				if(initPar.saveShadow==TRUE) {
					filename.clear();
					filename << "detected/frame"<< currentFrame->getFrameNumber() << "/shadow" << n << ".jpg"; 
					filename >> tempS;
					cvSaveImage(tempS.c_str(),(*i)->shadowMask,p);	
				}
			}
			else{
				filename << "detected/frame"<< currentFrame->getFrameNumber() <<".jpg"; 
				filename>>temp;
				cvSaveImage(temp.c_str(),current);
				filename.clear();
				filename << "detected/frame"<< currentFrame->getFrameNumber() << "_object" << n << ".jpg"; 
				filename >> temp;
				if(initPar.saveShadow==TRUE){
					filename.clear();
					filename << "detected/frame"<< currentFrame->getFrameNumber() << "_shadow" << n << ".jpg"; 
					filename >> tempS;
					cvSaveImage(tempS.c_str(),(*i)->shadowMask,p);
				}
			}
			
			cvSaveImage(temp.c_str(),(*i)->mvo,p);
			}
		cvReleaseImage(&current);
	}
	catch (exception& e){
		throw e.what();
	}
}


DWORD WINAPI Thread(void* param)
  { 
	UserPoolData* poolData = (UserPoolData*)param;
	LPVOID myData = poolData->pData;
	FrameObject *temp;
	threadParam &pa=*static_cast<threadParam *>(myData);
	list<FrameObject*> myFrames;
	int count = pa.numOfFirstFrame;
	IplImage *newBackground;
	list<preprocessStruct*>::iterator j;
	list<DetectedObject*>::iterator detIt;
	list<DetectedObject*> det;
	bool reshadowed=FALSE;
	bool ghost=FALSE;
	int supervisioned=0;

	LOG4CXX_INFO(loggerThread,"Thread "<< pa.threadNum << " started");	
	
	newBackground=cvCloneImage((*pa.child.begin())->background);

	for(j=pa.child.begin(); j != pa.child.end();j++){
		if(!reshadowed){
			cameraCorrection((*j)->background,(*j)->background,MEDIAN,1.1,5);		
			temp = new FrameObject((*j)->frame,(*j)->background,(*j)->salient, count);
		}
		else{
			temp = new FrameObject((*j)->frame,cvCloneImage(newBackground),(*j)->salient, count);
		}
		count++;
		temp->detectAll(initPar);
		if(initPar.supervisioning==TRUE){
			WaitForSingleObject(started,INFINITE);
			det=temp->getDetectedObject();
			for(detIt=det.begin();detIt!=det.end();detIt++){
				if(supervisioned > initPar.maxSupervisioned) break;
				ghost=isGhost((*detIt)->mvo);
				supervisioned++;
				if(ghost) (*detIt)->isGhost = TRUE;
			}
			ReleaseMutex(started);
		}
		if(!ghost){
			if(thread_saving==TRUE){
				LOG4CXX_DEBUG(loggerThread,"Saving detected to file");
				SaveDetectedToImage(temp,initPar.three);
				temp->~FrameObject();
			}
			else{	
				myFrames.push_back(dynamic_cast<FrameObject*>(temp));
			}
		}
		else{
			if(thread_saving==TRUE){
				LOG4CXX_DEBUG(loggerThread,"Reshadowing");
				reShadowing(temp,newBackground);
				reshadowed=TRUE;
				LOG4CXX_DEBUG(loggerThread,"Deleting ghost from memory");
				temp->~FrameObject();
			}
		}
 	}

	cvReleaseImage(&newBackground);

	if(thread_saving==TRUE) {
		LOG4CXX_INFO(loggerThread,"Stopping thread " << pa.threadNum);	
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

	LOG4CXX_DEBUG(loggerThread,"Thread n# "<< pa.threadNum << " -> Prepare to accode frame for delivery service");
	//soggetto a errori se ancora non esiste l'handle su cui fa SetEvent
	if(pa.threadNum%gap != 0){
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
	LOG4CXX_INFO(loggerThread,"Thread n# " << pa.threadNum << ": " << suc << " on "<< pa.child.size() << " accoded");
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
				
				if(!isGhost((*j)->getForegroundMask()))
					SaveDetectedToImage((*j),initPar.three);
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

/*!
//void Start (initializationParams par, string path)
//Mvo's detection and shadow suppression method 
// 	@param[in] par the inizialization parameter
//	@param[in] path the path of video suorce file
*/
void Start (initializationParams par, string path)
{
	initPar=par;
	CThreadPool threadPool = CThreadPool(initPar.POOL,TRUE,INFINITE);	
	gap=initPar.wait;
	int cicle_background = par.cicle_background;
	//parametri salvataggio immaggini
	p[0] = CV_IMWRITE_JPEG_QUALITY;
	p[1] = 90;
	p[2] = 0;
	thread_saving = initPar.thread_saving;
	string video_path = path;
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

	 CvCapture* capture = cvCaptureFromAVI(video_path.c_str());

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
	cout << endl;
	LOG4CXX_INFO(loggerMain,"Modello del background creato correttamente");
	list<DetectedObject>::iterator i;
	list<DetectedObject> det;

	started = CreateMutex(NULL,FALSE,NULL);

	list<preprocessStruct*>temporaryList;

	int cicle_num = 0;
	//int divider=initPar.THREAD_NUM;
	int divider=numOfTotalFrames/(initPar.THREAD_NUM-1);
	int count=0;
	int first=0;
	int thread_num=0;
	int index;
	int flag = TRUE;
	preprocessStruct *temp;
	handle.clear();
	threadPool.Run(Delivery,NULL,High);
	//_beginthread(Delivery,0,NULL);
	while(thread_num<divider && flag){
		temporaryList.clear();
		first = count;
		cicle_num = numOfTotalFrames/divider;
		index=0;
		flag = cvGrabFrame(capture);

		while (cicle_num>=0 && flag){
			img = cvRetrieveFrame(capture);
			if(index%gap==0) {

															    //<-------------				
				if(cicle_background != 1)						//				|
					cvUpdateBGStatModel(img,bgModel);			//				|	influenza in background update
				cameraCorrection(img,img,MEDIAN,1.1,5);  		//>-------------

				temp = new preprocessStruct(cvCloneImage(img),cvCloneImage(bgModel->background),cvCloneImage(bgModel->foreground));
				temporaryList.push_back(temp);					
				cicle_num--;
				count++;			
			}
			index++;
			flag = cvGrabFrame(capture);
		}	
		/*******MULTITHREAD**********/
		/*Gestione coda degli handle*/
		handle.push_back(CreateEvent( NULL, FALSE, FALSE, NULL ));
		/****************************/
		threadPool.Run(Thread,(void*)new _threadParam(temporaryList,first,thread_num),Low);		
		
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
	for(int waiting=1;waiting<=thread_num;waiting++)
		WaitForSingleObject(handle.at(waiting),INFINITE);
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
	LOG4CXX_INFO(loggerMain,"Elaborazione video " << video_path << " terminata.");
	try{
		cvReleaseBGStatModel(&bgModel);
		threadPool.Destroy();
	}catch(exception &e){
		e.what();
	}
	system("PAUSE");
}

