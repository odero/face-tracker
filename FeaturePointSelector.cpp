/********************************************************************
 * @Name:	FeaturePointSelector.h									*
 * @Desc:	A class definition for the component that locates		*
 *			good feature points (i.e. those with large eigen values)*
 *			on the selected ROI.									*
 ********************************************************************/

#include <stdio.h>
#include "FeaturePointSelector.h"

FeaturePointSelector::FeaturePointSelector(){
	corner_count = MAX_CORNERS;
	eigen = 0;
	temp = 0;
	gray = 0;
	flow_window = cvSize(10,10);
}

FeaturePointSelector::~FeaturePointSelector(){
	printf("Destroying Feature selector...");
}

int FeaturePointSelector::getCornerCount(){
	return corner_count;
}

CvPoint2D32f* FeaturePointSelector::selectFeaturePoints(IplImage* img_roi, CvRect* rect){
	

	CvSize size = cvGetSize(img_roi);
	corners = (CvPoint2D32f*)cvAlloc(sizeof(CvPoint2D32f[MAX_CORNERS]));

//	if (!gray){
		gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
		eigen = cvCreateImage(size, IPL_DEPTH_32F, 1);
		temp = cvCreateImage(size, IPL_DEPTH_32F, 1);
//	}

	corner_count = MAX_CORNERS;

	cvCvtColor(img_roi, gray, CV_BGR2GRAY);
	cvGoodFeaturesToTrack(gray, eigen, temp, corners, &corner_count, QUALITY, MIN_DISTANCE, 0);
/*    cvFindCornerSubPix(gray, corners, corner_count,
        flow_window, cvSize(-1,-1),
        cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
*/

	for(register int a = 0; a < corner_count; a++){
		corners[a].x = corners[a].x + rect->x;
		corners[a].y = corners[a].y + rect->y;
	}

	//corner_count = c_count;
	//printf("Number of corners %d\n", corner_count);
	return corners;
}