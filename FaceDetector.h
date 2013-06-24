/********************************************************************
 * @Name:	FaceDetector.h											*
 * @Desc:	A class declaration for the face detector defined in	*
 *			FaceDetector.cpp										*
 ********************************************************************/

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

class FaceDetector{
	public :
		FaceDetector();
		~FaceDetector();
		CvCapture* captureFrame();
		CvRect* detectFace(IplImage*);
		IplImage* setFaceROI(CvRect*);
		IplImage* nextFrame(CvCapture*, int);

	private :
		//Fields
		double scale;
		IplImage *frame, *copy, *img_roi;
		CvRect* rect;
		CvMemStorage* storage;
		CvHaarClassifierCascade* cascade;
		CvCapture* capture;
		const char* classifier_name;

		//Methods
		bool setClassifier();
		void setStorage();
};