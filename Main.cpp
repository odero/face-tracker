#include "Main.h"
#include <winable.h>
#include <windows.h>
#include <winuser.h>


static bool click_up_down = true;
static bool enable_scroll = false;
static int direction = -1;
static bool visible_pts = false;

MainProg::MainProg(){
	rect = 0;
	capture = 0;
	img = 0;
	img_roi = 0;
	empty_frame_count = 0;
	reverse_dir = false;
	face = (Face *)malloc(sizeof(Face));

	thread_count_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex lock
	cond_thread = PTHREAD_COND_INITIALIZER; //Condition variable
}

void MainProg::reset(){
	first_run = true;
	rect = 0;
	roi_set = false;
}


void MainProg::pause(){
	/*
	timespec t;
	t.tv_sec = 10;
	t.tv_nsec = 0;

	printf("Going to sleep...\n");
	pthread_mutex_lock(&thread_count_mutex);
	pthread_cond_timedwait(&cond_thread, &thread_count_mutex, &t);
	empty_frame_count = 0;
	*/
	printf("Going to sleep...\n");
//	Sleep(10000);
	empty_frame_count = 0;
	printf("Awake...\n");
//	pthread_cond_wait(&cond_thread, &thread_count_mutex);
}

void MainProg::resume(){
	pthread_mutex_unlock(&thread_count_mutex);
	pthread_cond_signal(&cond_thread);
}

Face* MainProg::currentFace(){
	pthread_mutex_lock(&thread_count_mutex);
	face->x = tracker.getCurrentPoint(reverse_dir).x;
	face->y = tracker.getCurrentPoint(reverse_dir).y;
	face->scale = tracker.getScale();
	face->tilt = tracker.getTilt();
	pthread_mutex_unlock(&thread_count_mutex);

	return face;
}

static void* run(void* arg){
	MainProg p;
	p.ProgLoop();
	return 0;
}

void MainProg::start(){
	pthread_create(&pid, NULL, ::run, NULL);
	pthread_exit(0);
}

void MainProg::ProgLoop(){
	reset();
	sensitivity = 1;
	float old_scale = 0.0, current_scale = 0.0;
	capture = detector.captureFrame();

	if (!capture)
		fprintf(stderr, "ERROR: Cant find the webcam. Ensure that it is connected properly\n\a");


	cvNamedWindow("ROI");
	cvNamedWindow("Orijino");

	while (true){
		img = detector.nextFrame(capture,direction);

		/*if no face is detected attempt to redetect*/
		
		if (!rect){
			rect = detector.detectFace(img);
			if(!rect){
				empty_frame_count++;
				//fprintf(stderr, "Error: No face detected.\n");
				//printf("%d\n", empty_frame_count);
				if (empty_frame_count > 100){
					pause();	//if no motion put application to sleep
				}
					
				continue;
			}
			
		}

		if (first_run){
		//Set ROI once/
			img_roi = detector.setFaceROI(rect);
			tracker.frame1_features = point_selector.selectFeaturePoints(img_roi, rect);
			if (point_selector.getCornerCount() < MAX_FEATURES){
				reset();
				fprintf(stderr, "Error: Too far from the screen.\n");
				continue;
			}
			roi_set = true;
		}

		tracker.track(img, tracker.frame1_features, point_selector.getCornerCount(), first_run, visible_pts);
		first_run = false;

		if (tracker.needsReInit()){
			reset();		
			continue;
		}

		//printf("%d\n", currentFace()->tilt);



		/********************Mouse emulation Begins********************/

		SetCursorPos(currentFace()->x, currentFace()->y);

		//emulate mouse scroll
		if (enable_scroll){
//			if ((currentFace()->scale - old_scale) > 2)
			if (currentFace()->scale > old_scale){
				//emulate scroll up
				mouse_event(MOUSEEVENTF_WHEEL, currentFace()->x , currentFace()->y, 1,0);
			}else{
				//emulate scroll down
				mouse_event(MOUSEEVENTF_WHEEL, currentFace()->x , currentFace()->y, -1,0);
			}

		old_scale = currentFace()->scale;
		printf("%2.3f\n", currentFace()->scale);
		}

		if (!tracker.noMotion()){
			mouse_event(MOUSEEVENTF_LEFTDOWN, currentFace()->x , currentFace()->y, 0,0);
			if (click_up_down)
				mouse_event(MOUSEEVENTF_LEFTUP, currentFace()->x , currentFace()->y, 0,0);

			printf("click\n\a");
		}
		/********************Mouse emulation Ends********************/

		//Draw the rectangle
/*		cvRectangle(img, cvPoint(rect->x, rect->y), 
			cvPoint(rect->x + rect->width, rect->y + rect->height), CV_RGB(255,0,0), 2);
*/
		cvShowImage("ROI", img_roi);
		cvShowImage("Orijino", img);
		int key = cvWaitKey(10);

		if (key == '0'){
			//reverse_dir = !reverse_dir;
			if (direction == 0)
				direction = -1;
			else
				direction = 0;
		}
		else if (key == '+'){
			sensitivity++;
		}
		else if (key == '-'){
			if (sensitivity > 1)	//Ensure sensitivity never goes below 1
				sensitivity--;
		}
		else if (key == 'p')
			pause();
		else if (key == '1'){
			click_up_down = !click_up_down;
			printf("Sticky Click: %d\n", click_up_down);
		}
		else if (key == '2'){
			enable_scroll = !enable_scroll;
			printf("Scroll: %d\n", enable_scroll);
		}
		else if (key == '3'){
			visible_pts = !visible_pts;
		}
		else if(key >= 0)
			break;

	}
	

	cvReleaseImage(&img_roi);
	cvReleaseImage(&img);
	cvDestroyWindow("ROI");
	cvDestroyWindow("Orijino");

	//delete &detector;
	exit(0);
}
