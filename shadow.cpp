#include "shadow.h"

LoggerPtr loggerCameraCorrection(Logger::getLogger( "Camera Correction"));
LoggerPtr loggershadowDetection(Logger::getLogger( "Shadow Detection"));
LoggerPtr loggerBackgroundSuppression(Logger::getLogger( "Background Suppression"));
LoggerPtr loggerBlobAnalysis(Logger::getLogger( "Blob analysis"));


/*!
//void cameraCorrection(IplImage* src,IplImage* dst,int type, double A, int size)
//	Corregge un singolo frame:
//	@param[in] src l'immagine da correggere
//	@param[out] dst dove viene memorizzato il risultato	
//	@param[in] type(default=MEDIAN): può assumere i seguenti valori...
//		MEDIAN filtro mediano per ridurre il rumore
//		SHARPENING filto di sharpening con maskera (0,-1,0;-1,5,-1;0,-1,0)
//		HIGH_BOOST sharpening high boost, dipendente dal valore di A(default=1.1)
//			maschera	-1   -1   -1 
//						-1  A*9-1 -1
//						-1   -1   -1
//		Sono inoltre consentite le seguenti combinazioni 
//						(cambiando l'ordine degli addendi il risultato non cambia, le operazioni verranno applicate sempre con lo stesso ordine)
//		MEDIAN+SHARPENING
//		MEDIAN+SHARPENING+MEDIAN
//		MEDIAN+HIGH_BOOST
//		MEDIAN+HIGH_BOOST+MEDIAN
//	@param[in] A (default=1.1) parametro per filtraggio con HIGH_BOOST
//	@param[in] size (default=5) dimensione della maschera del filtro mediano
*/
void cameraCorrection(IplImage* src,IplImage* dst,int type, double A, int size){	
	double w;
	CvMat* kernel = cvCreateMat( 3, 3, CV_32FC1) ;
	LOG4CXX_TRACE(loggerCameraCorrection , "Camera correction started");
	try{

		switch(type){
		case MEDIAN:
			cvSmooth(src,dst,CV_MEDIAN,size,0,0,0);
			LOG4CXX_DEBUG(loggerCameraCorrection,"Median filter application");
			break;
		case HIGH_BOOST:
			w=9*A-1;
			cvSet2D(kernel, 0, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(w) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( -1) );

			cvFilter2D(src, dst,kernel,cvPoint(0,0));
			LOG4CXX_DEBUG(loggerCameraCorrection,"High boost application");
			break;
		case SHARPENING:
			cvSet2D(kernel, 0, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( 0) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(5) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( 0) );

			cvFilter2D(src, dst,kernel,cvPoint(0,0));
			LOG4CXX_DEBUG(loggerCameraCorrection,"Sharpening application");
			break;
		case (MEDIAN+SHARPENING):
			cvSet2D(kernel, 0, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( 0) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(5) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( 0) );
		
			cvSmooth(src,dst,CV_MEDIAN,size,0,0,0);
			cvFilter2D(dst, dst,kernel,cvPoint(0,0));
			break;
			LOG4CXX_DEBUG(loggerCameraCorrection,"Median filter and Sharpening application");
		case (MEDIAN+HIGH_BOOST):
			w=9*A-1;
			cvSet2D(kernel, 0, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(w) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( -1) );

			cvSmooth(src,dst,CV_MEDIAN,size,0,0,0);
			cvFilter2D(dst, dst,kernel,cvPoint(0,0));
			LOG4CXX_DEBUG(loggerCameraCorrection,"Median filter and high boost application");
			break;
		case (MEDIAN+HIGH_BOOST+MEDIAN):
			w=9*A-1;
			cvSet2D(kernel, 0, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(w) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( -1) );

			cvSmooth(src,dst,CV_MEDIAN,size,0,0,0);
			cvFilter2D(dst, dst,kernel,cvPoint(0,0));
			cvSmooth(dst,dst,CV_MEDIAN,size-2,0,0,0);
			LOG4CXX_DEBUG(loggerCameraCorrection,"Median filter, high boost and median filter application again ");
			break;
		case(MEDIAN+SHARPENING+MEDIAN):
			cvSet2D(kernel, 0, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 0, 1, cvRealScalar( -1) ); 
			cvSet2D(kernel, 0, 2, cvRealScalar( 0) );
			cvSet2D(kernel, 1, 0, cvRealScalar( -1) );
			cvSet2D(kernel, 1, 1, cvRealScalar(5) );
			cvSet2D(kernel, 1, 2, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 0, cvRealScalar( 0) );
			cvSet2D(kernel, 2, 1, cvRealScalar( -1) );
			cvSet2D(kernel, 2, 2, cvRealScalar( 0) );

			cvSmooth(src,dst,CV_MEDIAN,size,0,0,0);
			cvFilter2D(dst, dst,kernel,cvPoint(0,0));
			cvSmooth(dst,dst,CV_MEDIAN,size-2,0,0,0);
			LOG4CXX_DEBUG(loggerCameraCorrection,"Median filter, sharpening and median filter application again ");
			break;
		default:
			throw 1;
		}
	}catch(int e){
		LOG4CXX_ERROR(loggerCameraCorrection,"Exception: the inserted parameter 'type' in cameraCorrection() isn't allowed");
	}
}

/*!
//void shadowDetection(IplImage *src, IplImage *background,IplImage *foregroundSelection)
// \link shadowDetection \endlink
//Metodo per il detecting delle ombre (HSV space)
//	@param[in] src The source image for shadow detection
//	@param[in] background The background image
//	@param[in] foreground selection The mask (binary image) that limit the area of the shadow detection process
//	@param[out] result a mask that rappresent the shadow
*/
void shadowDetection(IplImage *src, IplImage *background,IplImage *foregroundSelection,IplImage *result,initializationParams initPar){

	IplImage *temp,*Dbkg;
	CvScalar MED1,MED2,MAD,MED;
	
	float K = initPar.K;
	double alfa = initPar.alfa;
	double beta = initPar.beta;
	double Th = initPar.Th;
	double Ts = initPar.Ts;
	int Delta = initPar.Delta;

	LOG4CXX_DEBUG(loggershadowDetection, "Shadow Detection started....");

	try{
		IplImage *hsv;
		IplImage *H=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		IplImage *S=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		IplImage *V=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		IplImage *bH=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		IplImage *bS=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		IplImage *bV=cvCreateImage(cvGetSize(src), src->depth,1 ); 
		cvZero(result);
		cvZero(H);
		cvZero(S);
		cvZero(V);
		cvZero(bH);
		cvZero(bS);
		cvZero(bV);
		hsv=cvCloneImage(src);
		cvCvtColor(src,hsv,CV_BGR2HSV);
		cvCvtColor(background,background,CV_BGR2HSV);
		LOG4CXX_DEBUG(loggershadowDetection, "Conversion to HSV");
		//cvCvtColor(dst,dst,CV_BGR2HSV);

		if(alfa == -1 || beta ==-1 || Th == -1 || Ts == -1){
			IplImage *temp,*Dbkg;
			Dbkg=cvCloneImage(hsv);
			cvZero(Dbkg);

			cvAbsDiff(hsv,background,Dbkg);
			CvScalar MED = cvAvg(Dbkg,foregroundSelection);
			temp=cvCloneImage(hsv);
			cvAbsDiffS(Dbkg,temp,MED);
			CvScalar MAD = cvAvg(temp,foregroundSelection);

			CvScalar MED1 = cvAvg(hsv,foregroundSelection);
			CvScalar MED2 = cvAvg(background,foregroundSelection);
		
			double med=((MED.val[2]+3*1.4826*MAD.val[2])/MED2.val[2]);			
			
			if(alfa == -1)
				//alfa=med-K;
				alfa = MED1.val[2]/MED2.val[2];
			if(beta == -1)
				beta = med;
			if(Th == -1)
				Th = (MED.val[0]+MAD.val[0])/2;
		
			if(Ts == -1)
				Ts = (MED.val[1]+MAD.val[1])/2;

			cvReleaseImage(&Dbkg);
			cvReleaseImage(&temp);
		}
		LOG4CXX_DEBUG(loggershadowDetection, "Param Define conclused");

		cvSplit( hsv, H, S, V, 0);
		cvSplit( background, bH, bS, bV, 0);


		uchar* dataFs = (uchar *)foregroundSelection->imageData;
		int stepFs = foregroundSelection->widthStep/sizeof(uchar);
	
		uchar* data = (uchar *)result->imageData;
		int step = result->widthStep/sizeof(uchar);
	
		uchar* dataH = (uchar *)H->imageData;
		int stepH = H->widthStep/sizeof(uchar);
	
		uchar* dataS = (uchar *)S->imageData;
		int stepS = S->widthStep/sizeof(uchar);
	
		uchar* dataV    = (uchar *)V->imageData;
		int stepV = V->widthStep/sizeof(uchar);
	
		uchar* databH = (uchar *)bH->imageData;
		int stepbH = bH->widthStep/sizeof(uchar);
	
		uchar* databS = (uchar *)bS->imageData;
		int stepbS = bS->widthStep/sizeof(uchar);
	
		uchar* databV    = (uchar *)bV->imageData;
		int stepbV = bV->widthStep/sizeof(uchar);

		for(int i=0; i<hsv->height;i++){
			for(int j=0; j<hsv->width;j++){
				if((float)dataFs[i*stepFs+j] == 255){

					if(
					((float)dataV[i*stepV+j]/(float)databV[i*stepbV+j]>=alfa) && ((float)dataV[i*stepV+j]/(float)databV[i*stepbV+j]<=beta)
																&&
										  (fabsf((float)dataH[i*stepH+j]-(float)databH[i*stepbH+j])<=Th)
																&&
								   			  (((float)dataS[i*stepS+j]-(float)databS[i*stepbS+j])<=Ts)
				
					){ 
						data[i*step+j] = 255;
					}else 
						data[i*step+j] = 0;
				}
			}
		}

		cvCvtColor(background,background,CV_HSV2BGR);
		cvReleaseImage(&hsv);
		cvReleaseImage(&H);
		cvReleaseImage(&S);
		cvReleaseImage(&V);
		cvReleaseImage(&bH);
		cvReleaseImage(&bS);
		cvReleaseImage(&bV);
		LOG4CXX_DEBUG(loggershadowDetection, "shadow detection and memory release conclused");
		}
		catch(exception& e){
			LOG4CXX_ERROR(loggershadowDetection,e.what());
			throw e.what();
		}
}

//void AbsDiff(IplImage *hsv,IplImage *background,IplImage *Dbkg,IplImage *foregroundSelection){
//	
//	uchar* dataFs = (uchar *)foregroundSelection->imageData;
//	int stepFs = foregroundSelection->widthStep/sizeof(uchar);
//	
//	uchar* data = (uchar *)Dbkg->imageData;
//	int step = Dbkg->widthStep/sizeof(uchar);
//	
//	uchar* dataI = (uchar *)hsv->imageData;
//	int stepH = hsv->widthStep/sizeof(uchar);
//	
//	uchar* dataB = (uchar *)background->imageData;
//	int stepS = background->widthStep/sizeof(uchar);
//
//	int channels = hsv->nChannels;
//
//	for(int i=0; i<hsv->height;i++){
//		for(int j=0; j<hsv->width;j++){
//			if((float)dataFs[i*stepFs+j] == 255){
//				for(int k=0;k < channels;k++){
//					data[i*step+j*channels+k]=fabsf((float)dataI[i*step+j*channels+k]-(float)dataB[i*step+j*channels+k]);
//				}
//			}
//		}
//	}
//}

/*!
//IplImage* backgroundSuppression(IplImage *src, IplImage *background)
//Metodo per la soppressione del background secondo la formula:
// DBt(x,y) = distance(It(x,y),Bt(x,y)) = max(|It(x,y).c - Bt(x,y).c|) , c = R, G, B
//	@param[in] src The source image for background supression
//	@param[in] background The background image
//	@return a GRAY image that rappresent the foreground without background
*/
void backgroundSuppression( IplImage *src, IplImage *background,IplImage *result){
	
	cvZero(result);

	int height,width,step,channels,stepDbt;
	int i,j,k;
	uchar *dataF;

	float dst[3];


	uchar *dataBG;
	uchar *dataDBT;
	try{
		height    = src->height;
		width     = src->width;
		step      = src->widthStep;
		channels  = src->nChannels;
		dataF      = (uchar *)src->imageData;

		dataBG      = (uchar *)background->imageData;

		dataDBT		= (uchar *)result->imageData;
		LOG4CXX_TRACE(loggerBackgroundSuppression, "Background Suppression started....");
		stepDbt = result->widthStep/sizeof(uchar);
		int t =0;

			for(i=0;i<height;i++) {
			for(j=0;j<width;j++) {
				for(k=0;k<channels;k++){
					dst[k] = fabsf(dataF[i*step+j*channels+k]-dataBG[i*step+j*channels+k]);
					if(dst[0] <= dst[k]){
						dst[0] = dst[k];
					}
				}
				dataDBT[i*stepDbt+j] = dst[0];
			}
		}
			LOG4CXX_DEBUG(loggerBackgroundSuppression, "Background suppression conclused");
	}
	catch(exception& e){
		LOG4CXX_ERROR(loggerBackgroundSuppression,"Error in Background Suppression: "<< e.what());
		throw e.what();
	}
}

//copiare pixel a pixel tutti i ghost e le loro ombre sull'immagine di background e fare l'update con il risultato
//void reShadowing(FrameObject frame, CvBGStatModel *bgModel) {
//	IplImage *source = frame.getFrame();
//	CvSize size = cvGetSize(source);
//	IplImage *ghostMask = cvCreateImage(size,8,1);
//	IplImage *src = cvCreateImage(size,8,3);
//	IplImage *result = cvCloneImage(bgModel->background);
//	cvZero(ghostMask);
//	cvZero(src);
//
//	//al momento costruisce una maschera dalle blob...si potrebbe passare direttamente la maschera dei ghost e saltare questo ciclo
//	//for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
// //       {
//	//		cvRenderBlob(labelImg,it->second,source,src,CV_BLOB_RENDER_COLOR);
// //       }
//
//	
//	//det = frame.getDetectedGhost();
//	det = frame.getDetectedObject();
//	///
//	for(i=det.begin(); i != det.end(); ++i){
//		cvOr(ghostMask,(*i).totalMask,ghostMask);
//		cvShowImage("",ghostMask);
//		cvWaitKey(0);
//	}
//
//	int channel = source->nChannels;
//
//	uchar* dataSrc = (uchar *)source->imageData;
//	int stepSrc = source->widthStep/sizeof(uchar);
//	
//	uchar* dataBkg= (uchar *)result->imageData;
//	int stepBkg = result->widthStep/sizeof(uchar);
//	
//	uchar* dataGhost = (uchar *)ghostMask->imageData;
//	int stepGhost = ghostMask->widthStep/sizeof(uchar);
//	
//	/*uchar* dataS = (uchar *)S->imageData;
//	int stepS = S->widthStep/sizeof(uchar);
//	
//	uchar* dataV    = (uchar *)V->imageData;
//	int stepV = V->widthStep/sizeof(uchar);
//	
//	uchar* databH = (uchar *)bH->imageData;
//	int stepbH = bH->widthStep/sizeof(uchar);
//	
//	uchar* databS = (uchar *)bS->imageData;
//	int stepbS = bS->widthStep/sizeof(uchar);
//	
//	uchar* databV    = (uchar *)bV->imageData;
//	int stepbV = bV->widthStep/sizeof(uchar);*/
//
//	int i,j,k;
//
//	for(i=0; i<src->height;i++){
//		for(j=0; j<src->width;j++){
//			if((float)dataGhost[i*stepGhost+j] == 255){
//				for(k=0;k<channel;k++)
//					dataBkg[i*stepBkg+j*channel+k]=dataSrc[i*stepSrc+j*channel+k];
//				
//			}
//		}
//	}
//	
//	cvShowImage("result",result);
//	cvWaitKey();
//
//	cvUpdateBGStatModel(result,bgModel);
//
//}

void blobAnalysis(IplImage * imgA, IplImage * imgB){
	const int MAX_CORNERS = 1000;
	CvSize img_sz = cvGetSize( imgA );
	int win_size = 15;
	LOG4CXX_TRACE(loggerBlobAnalysis , "Blob analisis started");
	try{
		IplImage *imgC = cvCreateImage(cvGetSize(imgA), IPL_DEPTH_32F, 3);

		// Get the features for tracking
		IplImage* eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
		IplImage* tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );

		int corner_count = MAX_CORNERS;
		CvPoint2D32f* cornersA = new CvPoint2D32f[ MAX_CORNERS ];

		cvGoodFeaturesToTrack( imgA, eig_image, tmp_image, cornersA, &corner_count,
			0.05, 5.0, 0, 3, 0, 0.04 );

		cvFindCornerSubPix( imgA, cornersA, corner_count, cvSize( win_size, win_size ),
			cvSize( -1, -1 ), cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 ) );

		// Call Lucas Kanade algorithm
		char features_found[ MAX_CORNERS ];
		float feature_errors[ MAX_CORNERS ];

		CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );

		IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
		IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );

		CvPoint2D32f* cornersB = new CvPoint2D32f[ MAX_CORNERS ];

		cvCalcOpticalFlowPyrLK( imgA, imgB, pyrA, pyrB, cornersA, cornersB, corner_count, 
			cvSize( win_size, win_size ), 5, features_found, feature_errors,
			 cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.3 ), 0 );
			 LOG4CXX_DEBUG(loggerBlobAnalysis,"Lucas Kanade algorithm application");
		// Make an image of the results
		for( int i=0; i<1000; i++ ){
				printf("Error is %f/n", feature_errors[i]);
			printf("Got it/n");
			CvPoint p0 = cvPoint( cvRound( cornersA[i].x ), cvRound( cornersA[i].y ) );
			CvPoint p1 = cvPoint( cvRound( cornersB[i].x ), cvRound( cornersB[i].y ) );
			cvLine( imgC, p0, p1, CV_RGB(255,0,0), 2 );
		LOG4CXX_DEBUG(loggerBlobAnalysis,"Make an image of the results");
		}
	
	cvNamedWindow( "ImageA", 0 );
	cvNamedWindow( "ImageB", 0 );
	cvNamedWindow( "LKpyr_OpticalFlow", 0 );

	cvShowImage( "ImageA", imgA );
	cvShowImage( "ImageB", imgB );
	cvShowImage( "LKpyr_OpticalFlow", imgC );
	LOG4CXX_TRACE(loggerBlobAnalysis, "Blob analisis completed");
	cvWaitKey(0);
	}
	catch(exception& e)
	{
		LOG4CXX_ERROR(loggerBlobAnalysis, "Error in Blob Analisis: " << e.what());
	}

}