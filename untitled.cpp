#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

void drawText(Mat & image);

int main(int argc, char** argv)
{


	Mat blue(480, 640, CV_8UC3, Scalar(255, 0, 0));
    namedWindow("blue", WINDOW_NORMAL);
  
    imshow("blue", blue);

    Mat green(480, 640, CV_8UC3, Scalar(0, 255, 0));
    namedWindow("green", WINDOW_NORMAL);
  
    imshow("green", green);

    Mat red(480, 640, CV_8UC3, Scalar(0, 0, 255));
    namedWindow("red", WINDOW_NORMAL);
  
    imshow("red", red);

    Mat black(480, 640, CV_8UC3, Scalar(0, 0, 0));
    namedWindow("black", WINDOW_NORMAL);
  
    imshow("black", black);

    Mat white(480, 640, CV_8UC3, Scalar(255, 255, 255));
    namedWindow("white", WINDOW_NORMAL);
  
    imshow("white", white);





    cout << "Built with OpenCV " << CV_VERSION << endl;
    Mat image;
    VideoCapture capture;
    capture.set(CV_CAP_PROP_FRAME_WIDTH,1920);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,1080);
    capture.open(1);

    Mat captureSize;
    capture >> captureSize;

    cout << "Capture is in " << captureSize.size().width << " x " << captureSize.size().height << " resolution.\n";
    if(capture.isOpened())
    {
        cout << "Capture is opened, press esc to close" << endl;
        for(;;)
        {

        	
            capture >> image;
            if(image.empty())
                break;
            drawText(image);
            namedWindow("Sample", WINDOW_NORMAL);
  
            imshow("Sample", image);

            

            

           



            if(waitKey(30) == 27) // press esc to close
                break;
        }
    }
    else
    {
        cout << "No capture" << endl;
        
    }

    return 0;

}

void drawText(Mat & image)
{
    putText(image, "Hello OpenCV",
            Point(20, 50),
            FONT_HERSHEY_COMPLEX, 1, // font face and scale
            Scalar(255, 255, 255), // white
            1, LINE_AA); // line thickness and type
}





