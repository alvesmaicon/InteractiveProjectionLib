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

        interProj.ShowCapturedImage();
        interProj.ShowDetectedInteraction();
    }
    return 0;
}
