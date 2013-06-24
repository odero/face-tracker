/********************************************************************
 * @Name:	FeaturePointSelector.h									*
 * @Desc:	A class declaration for FeaturePointSelector.cpp		*
 ********************************************************************/


#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

#define MAX_CORNERS		15
#define QUALITY			0.01
#define MIN_DISTANCE	10

class FeaturePointSelector{
	public:
		FeaturePointSelector();
		~FeaturePointSelector();
		CvPoint2D32f* selectFeaturePoints(IplImage*, CvRect*);
		int getCornerCount();

	private:
		int corner_count;
		IplImage *eigen;
		IplImage *temp;
		IplImage *gray;
		CvPoint2D32f *corners;
		CvSize flow_window;
};