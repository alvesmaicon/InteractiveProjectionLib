//#include <cvlib>

#ifdef _CH_
#pragma package <opencv>
#endif

#define CV_NO_BACKWARD_COMPATIBILITY

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif

using namespace std;


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



int main( int argc, char** argv )
{
    if( argc != 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    cv::Mat image;
    image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

   for (int i=0;i<image.size().width;i++){
   for (int j=0;j<image.size().height;j++){

	int r=getRedCv(image, i, j);
	int g=getGreenCv(image, i, j);
	int b=getBlueCv(image, i, j);
	int y=sqrt(r*r+g*g+b*b);

float fr=(float)r;
float fg=(float)g;
float fb=(float)b;
float fy=sqrt(fr*fr+fg*fg+fb*fb);

fr/=fy;
fg/=fy;
fb/=fy;

float scalar=fr;

      if (g>r && g>b)// && r<100 && g> 50 && b <200)
	setPixelCv(image, i, j,  255, 0, 0);
      }
}

double data[100][120];

for (int i=0;i<100;i++)
for (int j=0;j<120;j++)
data[i][j]=rand()%65535;
//{...then assign the values to data...}

cv::Mat dataMatrix(100,120,CV_8UC1, data);
//cv::Mat dataMatrix(100,120,CV_8UC1, data);


cv::namedWindow("myWindow", CV_WINDOW_AUTOSIZE);
//cvNamedWindow("myWindow", CV_WINDOW_AUTOSIZE);


cv::imshow("myWindow", dataMatrix);
cv::imshow("myWindow", image);
//cvShowImage("myWindow", dataMatrix);

char c = cvWaitKey(-1);


return 0;

}

