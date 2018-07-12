#include "InteractiveProjection.hpp"
#include <iostream>
#include <time.h>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
	// fps
	int frameCounter = 0, fps = 0;

	RNG rng;

	int time_count = 3;
	int msec = 0, trigger = 1000; /* 1000ms */
	clock_t before, difference;

	// creating and starting interactive projection
    InteractiveProjection interProj;
    interProj.Start(1);

    // reading projected image
    Mat img(600, 800, CV_8UC3, Scalar(255, 255, 255));
    
    int num_count_fail = 5, num_count_success = 0;

    bool running = false, win = false, loose = false;

	Point obj1(2000, 270);

	Point obj2(1600, 500);

	Rect count_fail(10, 50, 100,50);
	Rect count_success(120, 50, 100,50);
	Rect count_start(400,300, 60, 60);
	Rect count_fps(700, 550, 60, 30);

	Rect start(300, 30, 100, 50);
	Rect pause(450, 30, 100, 50);
	Rect reset(600, 30, 100, 50);

	Scalar color1 = Scalar(120, 255, 80);
	Scalar color2 = Scalar(10, 100, 200);

	int vel1 = 18, vel2 = 18;

	int radius = 35;

    for(;;){
    	Mat white(600, 800, CV_8UC3, Scalar(255, 255, 255));

        // updating model
        interProj.Apply();
        
        // listening keyboard interaction
        interProj.KeyListener();

        // showing images
        interProj.ShowDetectedInteraction();
        interProj.ShowCapturedImage();

        if(interProj.mode == CALIBRATED){
        	interProj.UpdateProjectedImage(white);
        	interProj.ShowProjectedImage();
        }

        // verifing if detecting touch is active
        if(interProj.detectingTouch && interProj.mode != DETECTION){


        	if(num_count_fail <= 0 && win == false){
        		loose = true;
        	}

        	if(num_count_success >= 20 && loose == false){
        		win = true;
        	}

        	if(win){
        		putText(interProj.projectedImage, "You Win!", Point2f(350,300), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
        	}

        	if(loose){
        		putText(interProj.projectedImage, "Game Over!", Point2f(350,300), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
        	}

        	rectangle(interProj.projectedImage,start,Scalar(250,200,100),-1,8,0);
            rectangle(interProj.projectedImage,pause,Scalar(250,200,100),-1,8,0);
            rectangle(interProj.projectedImage,reset,Scalar(250,200,100),-1,8,0);

            putText(interProj.projectedImage, "Start", Point2f(320,60), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
            putText(interProj.projectedImage, "Pause", Point2f(470,60), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
            putText(interProj.projectedImage, "Reset", Point2f(620,60), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );

            std::ostringstream strs1;
            strs1 << "Lifes: " << num_count_fail;
            std::string str1 = strs1.str();
            
            std::ostringstream strs2;
            strs2 << "Success: " << num_count_success;
            std::string str2 = strs2.str();

            std::ostringstream strs3;
            strs3 << "FPS: " << fps;
            std::string str3 = strs3.str();

            rectangle(interProj.projectedImage,count_fail,Scalar(250,250,255),-1,8,0);
            putText(interProj.projectedImage, str1, Point2f(20,70), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );

            rectangle(interProj.projectedImage,count_fps,Scalar(250,250,255),-1,8,0);
            putText(interProj.projectedImage, str3, Point2f(700,570), FONT_HERSHEY_DUPLEX, 0.6, Scalar(50, 50, 50), 1, 8, false );

            rectangle(interProj.projectedImage,count_success,Scalar(250,250,255),-1,8,0);
            putText(interProj.projectedImage, str2, Point2f(130,70), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );

            if(interProj.FingertipInteraction(start)){
        		running = true;
        		before = clock();
        	}

        	if(interProj.FingertipInteraction(pause)){
        		running = false;
        	}

        	if(interProj.FingertipInteraction(reset)){
        		num_count_fail = 5;
        		num_count_success = 0;
        		win = false;
        		loose = false;
        		running = false;
        		time_count = 3;
        		obj1.x=2000;
				obj1.y=270;
				obj2.x=1600;
				obj2.y=500;
				vel1 = 18;
				vel2 = 18;
				Point obj2(1400, 500);
        	}

        	if(!win && !loose){
        		circle(interProj.projectedImage,Point(obj1.x, obj1.y), radius, color1,-1,8,0);
	        	circle(interProj.projectedImage,Point(obj2.x, obj2.y), radius ,color2,-1,8,0);
        	}

        	frameCounter++;
    		difference = clock() - before;
            msec = difference * 1000 / CLOCKS_PER_SEC;

            if ( msec >= trigger){
            	before = clock();
            	fps = frameCounter;
            	frameCounter = 0;

            	if(time_count >= 0 && running && !win && !loose)
            		time_count--;
            }

        	if(running && !win && !loose){
        		if(time_count >=0){
        			rectangle(interProj.projectedImage,count_start,Scalar(250,250,255),-1,8,0);

        			std::ostringstream strs3;
            		strs3 << time_count;
            		std::string str3 = strs3.str();
            		if(time_count == 0){
            			putText(interProj.projectedImage, "Start!", Point2f(400,300), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
            		}
            		else{
            			putText(interProj.projectedImage, str3, Point2f(420,300), FONT_HERSHEY_DUPLEX, 0.7, Scalar(50, 50, 50), 2, 8, false );
            		}
        		}

	            if(obj1.x <= 0){
	            	num_count_fail--;
	            	obj1.x = 800;
	            	color1 = Scalar( rng.uniform(0, 200), rng.uniform(0,200), rng.uniform(0,200) );
	            	obj1.y = rng.uniform(150,500);
	            	vel1 = rng.uniform(20,40);
	            }
	            else{
	            	obj1.x-=vel1;
	            }

	            if(obj2.x <= 0){
	            	num_count_fail--;
	            	obj2.x = 800;
	            	color2 = Scalar( rng.uniform(0, 200), rng.uniform(0,200), rng.uniform(0,200) );
	            	obj2.y = rng.uniform(150,500);
	            	vel2 = rng.uniform(20,40);
	            }
	            else{
	            	obj2.x-=vel2;
	            }

	            if(interProj.RegionInteraction(Point(obj1.x, obj1.y), radius)){
	                obj1.x = 800;
	            	color1 = Scalar( rng.uniform(0, 200), rng.uniform(0,200), rng.uniform(0,200) );
	            	obj1.y = rng.uniform(150,500);
	            	num_count_success++;
	            }

	            if(interProj.RegionInteraction(Point(obj2.x, obj2.y), radius)){
	                obj2.x = 800;
	            	color2 = Scalar( rng.uniform(0, 200), rng.uniform(0,200), rng.uniform(0,200) );
	            	obj2.y = rng.uniform(150,500);
	            	num_count_success++;
	            }
        	}

        	// showing projected image
        	interProj.ShowProjectedImage();
        	interProj.UpdateProjectedImage(white);
        }
    }
    return 0;
}