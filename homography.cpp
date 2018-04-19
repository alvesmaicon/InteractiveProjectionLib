#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;


const char* liveCaptureHelp =
    "When the live video from camera is used as input, the following hot-keys may be used:\n"
        "  <ESC>, 'q' - quit the program\n"
        "  'c' - start calibrate perspective\n"
        "  'u' - switch undistortion on/off\n";


enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };


int main( int argc, char** argv ){
	
	Size boardSize, imageSize;
	Mat cameraMatrix, chessboardMatrix, H;
	int i, nframes;
	bool undistortImage = false;
	VideoCapture capture;
	int mode = DETECTION;

	/* Selected camera is usb camera by default*/
	int cameraId = 1;

	vector<Point2f> cornersPattern, cornersCameraView;



	boardSize.width = 9;
    boardSize.height = 6;


    capture.open(cameraId);

    if( !capture.isOpened())
        return fprintf( stderr, "Could not initialize video (%d) capture\n",cameraId ), -2;


    /* Code for open chessboard image and find corners*/
    chessboardMatrix = imread("patternsmall.png", 0);
	if (chessboardMatrix.empty())
    	return fprintf( stderr, "Failed imread(): image not found\n"), -2;
  
	bool foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                    CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


	namedWindow("Projection", WINDOW_NORMAL);
	imshow("Projection", chessboardMatrix);


	namedWindow( "Camera View", 1 );


	for(i = 0;;i++){
		Mat view;

		if(capture.isOpened()){
			capture >> view;
		}

		imageSize = view.size();


		if( mode == CAPTURING ){

		bool foundCornersView = findChessboardCorners( view, boardSize, cornersCameraView,
                    CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


		  if(foundCornersView){
                drawChessboardCorners( view, boardSize, Mat(cornersCameraView), foundCornersView );


                H = findHomography(cornersCameraView, cornersPattern);
    		  mode = CALIBRATED;
            }
			
    	}

    	if( mode == CALIBRATED && undistortImage){
    		Mat temp = view.clone();
    		warpPerspective(temp, view, H, temp.size());
    	}


        imshow("Camera View", view);

        char key = (char)waitKey(capture.isOpened() ? 50 : 500);


        if( key == 27 )
            break;

        if( key == 'u' && mode == CALIBRATED )
            undistortImage = !undistortImage;


        if( capture.isOpened() && key == 'c' )
        {
            mode = CAPTURING;
        }

    }
}
