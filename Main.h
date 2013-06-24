#include <stdio.h>

#include "FaceDetector.h"
#include "FeaturePointSelector.h"
#include "FeatureTracker.h"
#include <pthread.h>

#define MAX_FEATURES 15

struct Face{
	int x;
	int y;
	float scale;
	int tilt;
};

class MainProg{
	public:
		void start();
		void pause();
		void resume();
		Face* currentFace();
		MainProg();
		void MainProg::ProgLoop();

	private:
		//Fields
		pthread_t pid;
		int empty_frame_count;	//counts # frames passed without detecting a face
		bool first_run;
		bool roi_set;
		bool reverse_dir;
		int sensitivity;
		pthread_mutex_t thread_count_mutex;
		pthread_cond_t cond_thread;

		FaceDetector detector;
		FeaturePointSelector point_selector;
		FeatureTracker tracker;
		Face* face;
		CvRect *rect;
		CvCapture *capture;
		IplImage *img;
		IplImage *img_roi;

		//Methods
		void reset();

};