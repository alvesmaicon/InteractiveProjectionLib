#include "InteractiveProjection.hpp"



using namespace std;
using namespace cv;



int main(int argc, const char** argv)
{
    InteractiveProjection interProj;
    interProj.Start(1);



    Mat img = imread("images/Screen-002.png", CV_LOAD_IMAGE_COLOR);
    if (img.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    bool running = false, started = false;

    int num_count = 0;

    Rect start(45, 270, 215, 65);
    Rect pause(45, 380, 215, 65);
    Rect reset(45, 490, 215, 65);
    Rect number(70, 50, 180, 170);

    for(;;){

        

        interProj.Apply();
        
        
        interProj.KeyListener();

        
        rectangle(interProj.capturedImage,start,Scalar(255,0,0),1,8,0);
        rectangle(interProj.capturedImage,pause,Scalar(0,0,255),1,8,0);
        rectangle(interProj.capturedImage,reset,Scalar(0,255,0),1,8,0);

        interProj.ShowCapturedImage();
        interProj.ShowDetectedInteraction();

        //if(interProj.mode == CALIBRATED && interProj.detectingTouch){
        if(interProj.detectingTouch){
            interProj.UpdateProjectedImage(img);

            //putText(interProj.projectedImage, "Start", Point2f(55,335), FONT_HERSHEY_TRIPLEX, 4,  Scalar(50, 50, 50));
            putText(interProj.projectedImage, "Start", Point2f(55,320), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            putText(interProj.projectedImage, "Pause", Point2f(55,430), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            putText(interProj.projectedImage, "Reset", Point2f(55,540), FONT_HERSHEY_TRIPLEX, 1.8, Scalar(50, 50, 50), 5, 8, false );
            //putText(InputOutputArray img, const String& text, Point org, int fontFace, double fontScale, Scalar color, int thickness=1, int lineType=LINE_8, bool bottomLeftOrigin=false )

            rectangle(interProj.projectedImage,number,Scalar(250,250,250),-1,8,0);

            if(interProj.FingertipInteraction(start)){
                running = true;
                started = true;
            }


            if(interProj.FingertipInteraction(pause)){
                running = false;
            }


            if(running)
            if(interProj.FingertipInteraction(reset)){
                num_count = 0;
            }

            if(started && running){
                num_count++;
                
            }
            std::ostringstream strs;
            strs << num_count;
            std::string str = strs.str();
            putText(interProj.projectedImage, str, Point2f(70,200), FONT_HERSHEY_TRIPLEX, 1.2, Scalar(50, 50, 50), 5, 8, false );

            interProj.ShowProjectedImage();


        }
        
    }
    return 0;
}
