/********************************************************************
 * @Name:	FaceDetector.cpp										*
 * @Desc:	A Class Definition for the face detector that finds &	*
 *			returns the first human face detected from the webcam	*
 *			stream.													*
 ********************************************************************/

#include <stdio.h>
#include "FaceDetector.h"

FaceDetector::FaceDetector(){
	scale = 1.5;
	frame = 0; copy = 0;
	storage = 0; cascade = 0; capture = 0;
	classifier_name = "haarcascade_frontalface_alt2.xml";

	/*If the cascade classifier cannot be loaded then exit the application*/
	if (!setClassifier())
		exit(-1);

	setStorage();
}

FaceDetector::~FaceDetector(){
	printf("Destroying Face Detector...\n");
	cvClearMemStorage(storage);
	//cvReleaseImage(&frame);
	cvReleaseImage(&copy);
	capture = 0;
	cascade = 0;
}

/*Selects the classifier to be used for face detection*/
bool FaceDetector::setClassifier(){
	cascade = (CvHaarClassifierCascade*)cvLoad(classifier_name, 0, 0, 0);
    if(!cascade)
    {
        fprintf(stderr, "ERROR: Could not load classifier cascade\n\a");
        return false;
    }

	return true;
}

/*Allocates memory for captured images*/
void FaceDetector::setStorage(){
	storage = cvCreateMemStorage(0);
}

/*Captures the Frames from the webcam*/
CvCapture* FaceDetector::captureFrame(){
	
	cvClearMemStorage(storage);

	/*Read in the video stream from the webcam*/
	capture = cvCaptureFromCAM(0);
	return capture;
}

IplImage* FaceDetector::nextFrame(CvCapture* capture, int direction){

	if (capture){

		/*If it was succesfully captured, capture the frame*/
		/*then retrieve the image from it and load it into memory*/
        frame = cvQueryFrame(capture);

		/*if there's no frame to capture exit*/
        if(!frame)
            return 0;

        if(!copy)
            copy = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, frame->nChannels);

		/*Retrieved images might appear reversed/upside down and therefore
		*need to be flipped
		*Also, cvQueryFrame always returns a pointer to same memory location
		*hence the need to make a copy (below)
		*/

        if(frame->origin == IPL_ORIGIN_TL)
            cvCopy(frame, copy, 0);
        else
            cvFlip(frame, copy, direction);
	}

	return copy;

}


/*Detect a face (Only once) and return the rectangle
*that specifies its location in the frame
*/
CvRect* FaceDetector::detectFace(IplImage* img){

	rect = 0;
	IplImage *gray, *small_img = 0;
	
    gray = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
    small_img = cvCreateImage(cvSize(cvRound(img->width/scale), cvRound(img->height/scale)), 
		IPL_DEPTH_8U, 1 );
    
    cvCvtColor(img, gray, CV_BGR2GRAY);
    cvResize(gray, small_img, CV_INTER_LINEAR);

	/*Equalize the color histogram to improve the contrast
	  and hence better detection of corners
	*/
    cvEqualizeHist(small_img, small_img);

	/*Clear storage which has been used before using it again*/
    cvClearMemStorage(storage);

	/*if the cascade classifier was successfully loaded...*/
	if(cascade){

		/*detect the face(s) and store them*/
		CvSeq* faces = cvHaarDetectObjects(small_img, cascade, storage, 1.1, 2, 
			CV_HAAR_DO_CANNY_PRUNING, cvSize(30, 30) );

		//printf("detected %d\n", faces->total);
		/*Extract only one face*/
		if (faces->total > 0){
			rect = (CvRect*)cvGetSeqElem(faces, 0);
			
		}//else{
			/*if no face was detected*/
			//Do nothing
			//return 0;
	}

	return rect;

}

IplImage* FaceDetector::setFaceROI(CvRect* r){
	img_roi = 0;

	int width = cvRound(r->width * 0.35);
	int height = cvRound(r->height * 0.3);

	r->x += width;
	r->y += height;
	r->width -= width * 2;
	r->height -= height * 2;
	
	r->x = cvRound(r->x*scale);
	r->y = cvRound(r->y*scale);
	r->width = cvRound(r->width * scale);
	r->height = cvRound(r->height * scale);
	

	img_roi = cvCreateImage(cvGetSize(copy), IPL_DEPTH_8U, 3);
	cvCopy(copy, img_roi);

	CvRect r_tmp = cvRect(r->x, r->y, r->width, r->height);
	cvSetImageROI(img_roi, r_tmp);

	return img_roi;
}