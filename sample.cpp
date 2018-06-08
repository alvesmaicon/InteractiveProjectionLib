#include "InteractiveProjection.hpp"



using namespace std;
using namespace cv;



int main(int argc, const char** argv)
{
    InteractiveProjection interProj;
    interProj.Start(1);



    Mat img = imread("images/img1.jpg", CV_LOAD_IMAGE_COLOR);
    if (img.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    for(;;){

        

        interProj.Apply();
        
        
        interProj.KeyListener();

        Rect r(400, 450, 100, 100);
        Rect r2(400, 300, 100, 100);
        rectangle(interProj.capturedImage,r,Scalar(255,0,0),1,8,0);
        rectangle(interProj.capturedImage,r2,Scalar(0,0,255),1,8,0);

        interProj.ShowCapturedImage();
        interProj.ShowDetectedInteraction();

        

        if(interProj.FingertipInteraction(r2)){
            cout << "Touch red rect with fingertip" << endl;
        }

        
        if(interProj.RegionInteraction(r)){
            cout << "Touch blue rect with any part of contour" << endl;
        }
        
    }
    return 0;
}
