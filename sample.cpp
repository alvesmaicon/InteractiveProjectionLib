#include "InteractiveProjection.hpp"

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    InteractiveProjection interProj;
    interProj.Start(1);

    // reading projected image
    Mat img = imread("images/Screen-002.png", CV_LOAD_IMAGE_COLOR);
    if (img.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    bool running = false, started = false;
    int num_count = 0;

    // defining interactions rectangles
    Rect start(45, 270, 215, 65);
    Rect pause(45, 380, 215, 65);
    Rect reset(45, 490, 215, 65);
    Rect number(70, 100, 150, 70);
    Rect text(400, 250, 300, 70);

    for(;;){

        // updating model
        interProj.Apply();
        
        // listening keyboard interaction
        interProj.KeyListener();

        rectangle(interProj.drawing,number,Scalar(0,0,0),-1,8,0);
        rectangle(interProj.drawing,text,Scalar(0,0,0),-1,8,0);
       
        // showing images
        interProj.ShowDetectedInteraction();
        interProj.ShowCapturedImage();

        // verifing if detecting touch is active
        if(interProj.detectingTouch){

            // showing projected image
            interProj.UpdateProjectedImage(img);

            // cleaning projected image
            rectangle(interProj.projectedImage,number,Scalar(250,250,255),-1,8,0);
            rectangle(interProj.projectedImage,text,Scalar(250,250,255),-1,8,0);
            
            // verifing if ocours fingertip interaction
            if(interProj.FingertipInteraction(start)){
                running = true;
                started = true;
            }

            if(interProj.FingertipInteraction(pause)){
                running = false;
            }

            if(started && running){
                num_count++;
            }

            if(interProj.FingertipInteraction(reset)){
                num_count = 0;
                interProj.ShowProjectedImage();          
            }

            std::ostringstream strs;
            strs << num_count;
            std::string str = strs.str();

            putText(interProj.projectedImage, str, Point2f(90,150), FONT_HERSHEY_TRIPLEX, 1.2, Scalar(50, 50, 50), 5, 8, false );
            putText(interProj.projectedImage, "Start", Point2f(55,320), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            putText(interProj.projectedImage, "Reset", Point2f(55,540), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            putText(interProj.projectedImage, "Pause", Point2f(55,430), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );

            if(started && running){
                putText(interProj.projectedImage, "Running", Point2f(400,300), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            }
            if(started && !running){
                putText(interProj.projectedImage, "Paused", Point2f(400,300), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            }

            interProj.ShowProjectedImage();
        }
    }
    return 0;
}