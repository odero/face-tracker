/********************************************************************
 * @Name:	FeatureTracker.h										*
 * @Desc:	A class declaration for FeatureTracker.cpp				*
 ********************************************************************/


#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

#define MAX_FEATURES	15
#define STD_DEV_THRESHOLD 150

class FeatureTracker{
	public:
		FeatureTracker();
		~FeatureTracker();
		void track(IplImage*, CvPoint2D32f* , int, bool, bool);
		int getCornerCount();
		CvPoint2D32f *frame1_features, *frame2_features;
		CvPoint getCurrentPoint(bool);
		int getLineLength(void);
		bool needsReInit(void);
		bool noMotion(void);
		float getScale(void);
		int getTilt(void);
	private:
		//fields
		float	screen_image_width_ratio,
				screen_image_height_ratio;
//		int img_width, img_height;
		float face_height;
		float prev_height, prev_width;
		float prev_x, prev_y, scale;
//		int line[2];
		int frame_count;
		bool motion_detected;
		int feature_count;
		char features_found[MAX_FEATURES];
		CvSize flow_window;

		CvPoint2D32f *base_points;
		CvPoint2D32f *dist;
		CvPoint2D32f *base_mean;
		CvPoint2D32f *swap_points;
		IplImage *frame1, *frame2, *pyramid1, *pyramid2, *swap_temp;

		//methods
		void swapBuffers(void);
		CvPoint mean_diff(CvPoint2D32f *base, CvPoint2D32f *curr);
		void setBaseFeatures(void);
		void calcBaseMean(CvPoint2D32f *base);
		CvPoint getStdDeviation(CvPoint2D32f *curr);
		float testZoom(void);
		
};