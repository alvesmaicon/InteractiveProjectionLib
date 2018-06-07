#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"


#include <stdio.h>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;


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




int main( int argc, char** argv ){

    double Gaverage = 0, Baverage = 0, Raverage = 0, S = 0.8;

    Mat img1, img2;

        /* Code for open image*/
    img1 = imread("predicted.jpg", CV_LOAD_IMAGE_COLOR);
    if (img1.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    namedWindow("Captured Image", WINDOW_NORMAL);
    imshow("Captured Image", img1);


    /* Code for open image*/
    img2 = imread("predicted1.jpg", CV_LOAD_IMAGE_COLOR);
    if (img2.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    namedWindow("Predicted Image", WINDOW_NORMAL);
    imshow("Predicted Image", img2);


    // calc the difference
    Mat diff;
    absdiff(img1, img2, diff);

    // Get the mask if difference greater than th
    int th = 100;  // 0
    Mat mask(img1.size(), CV_8UC1);
    for(int j=0; j<diff.rows; ++j) {
        for(int i=0; i<diff.cols; ++i){
            cv::Vec3b pix = diff.at<cv::Vec3b>(j,i);
            int val = (pix[0] + pix[1] + pix[2]);
            if(val>th){
                mask.at<unsigned char>(j,i) = 255;
            }
        }
    }

    // get the foreground
    Mat res;
    bitwise_and(img2, img2, res, mask);

    threshold(res, res, 0, 255, THRESH_BINARY);
    // display
    imshow("res", res);
    waitKey();
    return 0;


}



