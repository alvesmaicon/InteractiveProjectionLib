#ifndef INTERACTIVE_PROJECTION_HPP_
#define INTERACTIVE_PROJECTION_HPP_

#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video.hpp"
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"


#define DETECTION 0
#define CAPTURING 1
#define CALIBRATED 2



using namespace std;
using namespace cv;

class InteractiveProjection
{
public:
    InteractiveProjection();
    ~InteractiveProjection();
    void Start(int cameraId);
    void ShowCapturedImage();
    void ShowDetectedInteraction();
    void UpdateProjectedImage(Mat image);
    void Apply();
    bool FingertipInteraction();
    bool RegionInteraction();
    void KeyListener();
    Mat capturedImage, projectedImage, H, foregroundMask, foreground, background, chessboardMatrix, drawing;
    Size boardSize;
    vector<Point2f> cornersPattern, cornersCameraView;

    int mode = DETECTION;

    VideoCapture cap;

    bool foundCornersPattern;
    bool doUpdateModel = false, undistortImage = false, detectingTouch = false;


    Ptr<BackgroundSubtractorMOG2> model;




    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
protected:

private:
    void GetHomography();
    void DoUndistortCapture();



};

#endif //INTERACTIVE_PROJECTION_HPP_
