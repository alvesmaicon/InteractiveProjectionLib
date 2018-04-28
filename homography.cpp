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
    "The following hot-keys may be used:\n"
        "  <ESC>, 'q' - quit the program\n"
        "  'c' - start calibrate perspective\n"
        "  'u' - switch undistortion on/off\n";


void setPixelCv(cv::Mat& dataMatrix, int i, int j, uchar r, uchar g, uchar b)
{

    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[0] = b;
    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[1] = g;
    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[2] = r;

}

uchar getRedCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[2];
}

uchar getGreenCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[1];

}

uchar getBlueCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[0];

}





enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2};





int main( int argc, char** argv ){
	
    float Baverage = 0, Raverage = 0, Gaverage = 0; 

	Size boardSize, imageSize;
	Mat cameraMatrix, chessboardMatrix, chessboardMatrix2, H, inverseH, cameraView;
	int i, nframes;
	bool undistortImage = false, undistortProjection = false;
	VideoCapture capture;
	int mode = DETECTION;

	/* Selected camera is usb camera by default*/
	int cameraId = 1;
    Mat red(480, 640, CV_8UC3, Scalar(0, 0, 255));
	vector<Point2f> cornersPattern, cornersCameraView;



	boardSize.width = 9;
    boardSize.height = 6;


    capture.open(cameraId);

    capture >> cameraView;

    /**setting the camera resolution**/

    capture.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 600);

    

    if( !capture.isOpened())
        return fprintf( stderr, "Could not initialize video (%d) capture\n",cameraId ), -2;


    /* Code for open chessboard image and find corners*/
    chessboardMatrix = imread("pattern-800x600.png", 0);
	if (chessboardMatrix.empty())
    	return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    /*
  
    chessboardMatrix2 = imread("imagem.jpg", 0);
    if (chessboardMatrix2.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;
    
    else {

        for(int i = 0; i < red.size().height; i++){
            for(int j = 0; j < red.size().width; j++){
                Raverage += getRedCv(red, j, i);
                Gaverage += getGreenCv(red, j, i);
                Baverage += getBlueCv(red, j, i);



            }
        }
    }

    

    float tam = red.size().width * red.size().height;
    cout << " \nR: " << Raverage/tam << "\nB: " << Baverage/tam << "\nG: " << Gaverage/tam << endl;

    */

	bool foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                    CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


	namedWindow("Projection", WINDOW_NORMAL);
	imshow("Projection", chessboardMatrix);


	namedWindow("Camera View", WINDOW_NORMAL);


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
                /*undistorting projeted image*/
                /*
                H.at<double>(3,1) /= 2; 
                H.at<double>(3,2) /= 2; 
                H.at<double>(1,3) *= 2; 
                H.at<double>(2,3) *= 2; 

                warpPerspective(chessboardMatrix2, chessboardMatrix2, H, chessboardMatrix.size());

                imshow("Projection", chessboardMatrix2);

                */
    		    
            }
			
    	}

    	if( mode == CALIBRATED && undistortImage){
    		Mat temp = view.clone();
            /*undistorting camera image view to center top perspective*/
    		warpPerspective(temp, view, H, temp.size());
            
    	}


        imshow("Camera View", view);

        char key = (char)waitKey(capture.isOpened() ? 50 : 500);


        if( key == 27 )
            break;

        if( key == 'u' && mode == CALIBRATED )
            undistortImage = !undistortImage;
            undistortProjection = !undistortProjection;


        if( capture.isOpened() && key == 'c' )
        {
            mode = CAPTURING;
        }

        

    }
}
