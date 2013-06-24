/********************************************************************
 * @Name:	FeatureTracker.h										*
 * @Desc:	A class definition for the component that tracks feature*
 *			points using Optical Flow Technique implemented using	*
 *			Pyramidical Lucas-Kanade algorithm.						*
 ********************************************************************/

#include <stdio.h>
#include "FeatureTracker.h"
static float ratio = 0.0;
static float curr_top_pointx = 0.0;
static float prev_top_pointx = 0.0;
static float curr_bottom_pointx = 0.0;
static float prev_bottom_pointx = 0.0;


FeatureTracker::FeatureTracker(){
//	img_width = 0, img_height = 0;
	frame_count = 0;
	prev_height = 0.0, prev_width = 0.0, face_height = 0.0;
	prev_x = 0.0, prev_y = 0.0, scale = 0.0;
	motion_detected = true;
	feature_count = MAX_FEATURES;
	flow_window = cvSize(10,10);
	frame1 = 0; frame2 = 0; pyramid1 = 0; pyramid2 = 0; swap_temp = 0;
	base_points = (CvPoint2D32f*)cvAlloc(MAX_FEATURES*sizeof(CvPoint2D32f));
	dist = (CvPoint2D32f*)cvAlloc(sizeof(CvPoint2D32f));
	dist->x = 0.0; dist->y = 0.0;
	base_mean = (CvPoint2D32f*)cvAlloc(sizeof(CvPoint2D32f));
	frame1_features = (CvPoint2D32f*)cvAlloc(MAX_FEATURES*sizeof(CvPoint2D32f));
}

FeatureTracker::~FeatureTracker(){
}

int FeatureTracker::getCornerCount(){
	return feature_count;
}


void FeatureTracker::track(IplImage* img, CvPoint2D32f* corners, int corner_count, bool first_run, bool visible_pts){
	
	CvTermCriteria termination = cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, .3);
	CvSize size = cvGetSize(img);
	int flags = 0;

	if (!frame1){
		screen_image_width_ratio = (1024.0f/img->width * 1.4f);
		screen_image_height_ratio = (768.0f/img->height * 1.4f);

		
		frame1 = cvCreateImage(size, IPL_DEPTH_8U, 1);
		frame2 = cvCreateImage(size, IPL_DEPTH_8U, 1);
		pyramid1 = cvCreateImage(size, IPL_DEPTH_8U, 1);
		pyramid2 = cvCreateImage(size, IPL_DEPTH_8U, 1);
		frame2_features = (CvPoint2D32f*)cvAlloc(MAX_FEATURES*sizeof(CvPoint2D32f));
	}

	cvCvtColor(img, frame1, CV_BGR2GRAY);

	if (!first_run){
        cvCalcOpticalFlowPyrLK(frame2, frame1, pyramid2, pyramid1,
            frame2_features, frame1_features, feature_count, flow_window, 3, features_found, 
 			0, termination, flags);

		if (visible_pts){
			for( int i = 0; i < feature_count; i++ )
			{
				cvCircle( img, cvPointFrom32f(frame1_features[i]), 3, CV_RGB(0,255,0), -1, 8,0);
			}
		}
		//getStdDeviation(frame2_features);
	}else{
		setBaseFeatures();
		calcBaseMean(base_points);
	}
	swapBuffers();
	mean_diff(base_points, frame2_features);
	//testZoom();

	return;
}

void FeatureTracker::swapBuffers(void){
    CV_SWAP( frame2, frame1, swap_temp );
    CV_SWAP( pyramid2, pyramid1, swap_temp );
    CV_SWAP( frame2_features, frame1_features, swap_points );
}


/*
* Calculates the mean difference between base point(s) and current point(s)
*/
CvPoint FeatureTracker::mean_diff(CvPoint2D32f *base, CvPoint2D32f *curr){

	CvPoint2D32f *curr_mean = (CvPoint2D32f*)cvAlloc(sizeof(CvPoint2D32f));
	curr_mean->x = 0.0, curr_mean->y = 0.0;

	for (int a = 0; a < MAX_FEATURES; a++){
		curr_mean->x += curr[a].x;
		curr_mean->y += curr[a].y;
	}
	
	//calculate mean for current array
	curr_mean->x /= MAX_FEATURES;
	curr_mean->y /= MAX_FEATURES;


	//get actual position as ratio percentage position of 
	//mean point on the entire image width/height

	prev_x = dist[0].x - curr_mean->x * screen_image_width_ratio;
	prev_y = dist[0].y - curr_mean->y * screen_image_height_ratio;

//	printf("X = %.3f, Y = %.3f\n", prev_x, prev_y);

	if (prev_x > 9.0||prev_x < -9.0||prev_y > 9.0||prev_y < -9.0){
		dist[0].x = curr_mean->x * screen_image_width_ratio;
		dist[0].y = curr_mean->y * screen_image_height_ratio;
		frame_count = 0;
		//printf("reset count\n");
	}else{
		frame_count++;
	}

	return cvPointFrom32f(dist[0]);
/*
	disp_x  = curr_mean->x - base_mean->x;
	disp_y  = curr_mean->y - base_mean->y;

	//printf("%d\n", abs(int(disp_x - dist->x)));

	//update new location
	//executed ONLY either if its first run or if displacement is substancial i.e > 2
	if (dist->x == 0.0 ||abs(int(disp_x - dist->x)) > 2 || abs(int(disp_y - dist->y)) > 2){
		dist->x = disp_x;
		dist->y = disp_y;
		frame_count 
		= 0;
		//printf("reset count\n");
	}else
		frame_count++;

	//pr
	intf("X = %.3f, Y = %.3f\n", dist[0].x, dist[0].y);
	return cvPointFrom32f(dist[0]);
*/
}


void FeatureTracker::calcBaseMean(CvPoint2D32f *base){
	base_mean->x = 0, base_mean->y = 0;
	for (int a = 0; a < MAX_FEATURES; a++){
		base_mean->x += base[a].x;
		base_mean->y += base[a].y;
	}

	//calculate mean for base array
	base_mean->x /= MAX_FEATURES;
	base_mean->y /= MAX_FEATURES;
	//printf("%0.2f\n", base_mean->x);
}

/*
* Calculate the std deviation to determine if error occured
* i.e. if error is greater than a pre-specified std dev value re-initialization is required
*/

CvPoint FeatureTracker::getStdDeviation(CvPoint2D32f *curr){

	CvPoint2D32f *tmp = (CvPoint2D32f*)cvAlloc(sizeof(CvPoint2D32f));
	tmp->x = 0, tmp->y = 0;
	//sd = sqrt(1/N * sum(I[x] - Mean)^2)
	for (int a = 0; a < MAX_FEATURES; a++){
		tmp->x += powf(curr[a].x - base_mean->x, 2);
		tmp->y += powf(curr[a].y - base_mean->y, 2);
	}
	
	tmp->x = sqrtf(tmp->x/MAX_FEATURES);
	tmp->y = sqrtf(tmp->y/MAX_FEATURES);

	return cvPointFrom32f(*tmp);
}

/*
* Copy the base values
*/
void FeatureTracker::setBaseFeatures(void){
	memcpy(base_points, frame1_features, sizeof(CvPoint2D32f[MAX_FEATURES]));
}

float FeatureTracker::getScale(void){
	
	return face_height;
}

CvPoint FeatureTracker::getCurrentPoint(bool reverse){
	return cvPointFrom32f(dist[0]);
}
/*
int FeatureTracker::getLineLength(){
	return line;
}
*/

int FeatureTracker::getTilt(void){
	//if top-most moves right while bottom-most moves left
	//infer as tilt to left, 
	//else if vice versa infer tilt to left
	int tilt = 0;
	if ((curr_top_pointx > prev_top_pointx) && (curr_bottom_pointx < prev_bottom_pointx))
		tilt = 1;	//left tilt
	else if ((curr_top_pointx < prev_top_pointx) && (curr_bottom_pointx > prev_bottom_pointx))
		tilt = 2;	//right tilt

	return tilt;
}

/*
* Checks if re-initialization is needed
* i.e. if std dev > threashold re-init
*/
bool FeatureTracker::needsReInit(void){

	float ratio_in = testZoom();

	if (ratio_in < 0.8 || ratio_in > 2.0)
		return true;
	else
		return false;
/*	
	CvPoint std_dev = getStdDeviation(frame2_features);
	printf("%d %d\n", std_dev.x, std_dev.y);
	if (std_dev.x > STD_DEV_THRESHOLD || std_dev.y > STD_DEV_THRESHOLD)
		return true;
	else
		return false;
*/
}

float FeatureTracker::testZoom(void){
	//get the difference between the top-most and bottom-most point
	//if the difference increases, the user is moving closer to the screen = 'ZOOM IN'
	//if its decreasing, the user is moving away = 'ZOOM OUT'
	//get max and min for both y and x
	float min_y, max_y;
	float max_x, min_x;
	//float new_height = 0.0, new_width = 0.0;

	max_x = min_x = frame2_features[0].x;
	max_y = min_y = frame2_features[0].y;

	prev_top_pointx = curr_top_pointx;
	prev_bottom_pointx = curr_bottom_pointx;

	for (int i = 1; i < MAX_FEATURES; i++){
		//get max 'x'
		if (max_x < frame2_features[i].x){
			max_x = frame2_features[i].x;
			curr_top_pointx = max_x;
		}

		//get max 'y'
		if (max_y < frame2_features[i].y){
			max_y = frame2_features[i].y;
			//curr_top_pointx[1] = max_y;
		}

		//get min 'x'
		if (min_x > frame2_features[i].x){
			min_x = frame2_features[i].x;
			curr_bottom_pointx = min_x;
		}

		//get min 'y'
		if (min_y > frame2_features[i].y){
			min_y = frame2_features[i].y;
			//curr_top_pointx[1] = min_y;
		}
	}

	//get the ratio of y vs x
	float new_height = (max_y - min_y);
	float new_width = (max_x - min_x);
	ratio = new_height / new_width;

	//prev_height == 0.0 for first run only, so don calc scale for first run
	if (prev_height != 0.0){
		scale = new_height - prev_height;
	}

	prev_height = new_height;
		
	//Only record significant changes in height
//	if ((new_height - face_height) > 0.3)
		face_height = new_height;

	return ratio;
}

bool FeatureTracker::noMotion(){
	//if no motion is detected after 25 frames return false
	if (frame_count >= 25){
		motion_detected = false;
		frame_count = 0;
	}else
		motion_detected = true;
	return motion_detected;
}
