#include "InteractiveProjection.hpp"



InteractiveProjection::InteractiveProjection()
{

}

InteractiveProjection::~InteractiveProjection()
{

}

void InteractiveProjection::Start(int cameraId)
{
    cap.open(cameraId);
    if( !cap.isOpened())
    {
        cout << "Could not initialize video (" << cameraId << ") capture" << endl;
        exit(0);
    }


    cap >> capturedImage;

    // setting the camera resolution
    cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);



    chessboardMatrix = imread("images/pattern-800x600.png", CV_LOAD_IMAGE_COLOR);
    if (chessboardMatrix.empty())
    {
        cout << "Failed imread():  chesboard image not found" << endl;
        exit(0);
    }

    boardSize.width = 9;
    boardSize.height = 6;

    foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                          CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


    projectedImage = chessboardMatrix;

    namedWindow("Projection", WINDOW_NORMAL);
    imshow("Projection", projectedImage);


    model = createBackgroundSubtractorMOG2();
    // setting to not detect shadows
    model->setDetectShadows(true);
    // setting tolerance match
    model->setShadowValue(0);
    model->setVarThreshold(40);

    drawing = Mat::zeros( projectedImage.size(), CV_8UC3 );


}

void InteractiveProjection::ShowCapturedImage()
{
    imshow("Captured Image", capturedImage);
}

void InteractiveProjection::ShowDetectedInteraction()
{
    imshow("Detcted Interaction", drawing);
}

void InteractiveProjection::Apply()
{
    cap >> capturedImage;
    RNG rng(12345);

    if(undistortImage)
    {
        DoUndistortCapture();
    }

    if(detectingTouch)
    {

        cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // setting exposure to manual
        cap.set(CAP_PROP_AUTOFOCUS, 0); // turn the autofocus off
        // pass the frame to background model
        model->apply(capturedImage, foregroundMask, doUpdateModel ? -1 : 0);


        // show foreground image and mask (with optional smoothing)
        //GaussianBlur(foregroundMask, foregroundMask, Size(11, 11), 3.5, 3.5);
        //threshold(foregroundMask, foregroundMask, 130, 255, THRESH_BINARY);


        if (foreground.empty())
            foreground.create(capturedImage.size(), capturedImage.type());
        foreground = Scalar::all(0);
        capturedImage.copyTo(foreground, foregroundMask);

        Mat kernel = getStructuringElement(0, Size(5,5), Point(-1,-1) );
        morphologyEx(foregroundMask, foregroundMask, MORPH_OPEN, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );

        morphologyEx(foregroundMask, foregroundMask, MORPH_CLOSE, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );

        /// Find contours
        findContours( foregroundMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        /*
            Find max contour area than find hull to this contour.
            Then find center point to this contour
            Then find convex points to this contour.
            Now find bigger distance between center and convex.
            Will be a little hope to find the correct point.
        */
        double max_area = 0;
        
        vector<vector<Point>> hull(contours.size());


        drawing = Mat::zeros( foregroundMask.size(), CV_8UC3 );
        if(!contours.empty())
        {

            for(int i = 0; i < contours.size(); i++)
            {
                double a = contourArea( contours[i],false);
                if(a > max_area)
                {
                    max_area = a;
                    max_contour = i;
                }
            }


            drawContours( drawing, contours, max_contour, Scalar(255,255,255), -1, 8, vector<Vec4i>(), 0, Point() );

            Moments mu = moments(contours[max_contour]);
            Point centroid = Point (mu.m10/mu.m00, mu.m01/mu.m00);

            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );

            circle(drawing, centroid, 7, Scalar(255,0,0), -1, 8, 0);
            convexHull(contours[max_contour], hull[max_contour], false, false);
            drawContours( drawing, hull, max_contour, color, 1, 8, vector<Vec4i>(), 0, Point() );

            
            double max_dist = 0;
            for(int k = 0; k< hull[max_contour].size(); k++)
            {
                double dist = norm(centroid - hull[max_contour][k]);
                if(dist > max_dist)
                {
                    max_dist = dist;
                    fingertip = hull[max_contour][k];
                }
            }




            circle(drawing, fingertip, 7, Scalar(0,0,255), -1, 8, 0);

            /*
            if(fingertiTime() > 2)
            circle(projectedImage, fingertip, 5, Scalar(0,0,255), -1, 8, 0);
            */
        }

    }
}

bool InteractiveProjection::FingertipInteraction(Rect rectangle){
    if(!contours.empty()){
        if(rectangle.contains(fingertip))
            return true;
    }

    return false;
}

bool InteractiveProjection::RegionInteraction(Rect rectangle){
    if(!contours.empty()){
        for(int i = 0; i < contours[max_contour].size(); i++)
        {
            if(rectangle.contains(contours[max_contour][i]))
                return true;
        }
    }

    return false;
}

bool InteractiveProjection::FingertipInteraction(Point center, double radius){

    double dist = norm(fingertip - center);
    if (dist < radius)
        return true;
    return false;
}

bool InteractiveProjection::RegionInteraction(Point center, double radius){

    
    for(int i = 0; i < contours[max_contour].size(); i++)
    {
        double dist = norm(contours[max_contour][i] - center);
        if (dist < radius)
        return true;
    }

    return false;
}

void InteractiveProjection::UpdateProjectedImage(Mat image)
{
    projectedImage = image;
}

void InteractiveProjection::ShowProjectedImage()
{
    imshow("Projection", projectedImage);
}

void InteractiveProjection::GetHomography()
{

    bool foundCornersView = findChessboardCorners( capturedImage, boardSize, cornersCameraView,
                            CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


    if(foundCornersView && foundCornersPattern)
    {
        drawChessboardCorners( capturedImage, boardSize, Mat(cornersCameraView), foundCornersView );


        H = findHomography(cornersCameraView, cornersPattern);
        mode = CALIBRATED;
        cout << "Calibrated" << endl;

    }
    else
    {
        mode = DETECTION;
        cout << "Calibration failed" << endl;
    }

}

void InteractiveProjection::DoUndistortCapture()
{
    Mat temp = capturedImage.clone();

    /*undistorting camera image view to top center perspective*/
    warpPerspective(temp, capturedImage, H, temp.size());
}


void InteractiveProjection::KeyListener()
{
    const char key = (char)waitKey(30);
    if (key == 27 || key == 'q')
    {
        cout << "Exit requested" << endl;
        exit(0);
    }

    if( cap.isOpened() && key == 'c' && mode == DETECTION)
    {
        mode = CAPTURING;
        projectedImage = chessboardMatrix;
        imshow("Projection", projectedImage);
        GetHomography();
        return;
    }

    if( cap.isOpened() && key == 'c' && mode == CALIBRATED && !undistortImage)
    {
        mode = CAPTURING;
        projectedImage = chessboardMatrix;
        imshow("Projection", projectedImage);
        GetHomography();

    }

    if( key == 'u' && mode == CALIBRATED )
    {
        undistortImage = !undistortImage;
        cout << "Toggle undistort image: " << (undistortImage ? "ON" : "OFF") << endl;

    }

    if(key == 'd' )
    {
        detectingTouch = !detectingTouch;
        cout << "Toggle detecting touch: " << (detectingTouch ? "ON" : "OFF") << endl;
    }

    if (key == ' ')
    {
        doUpdateModel = !doUpdateModel;
        cout << "Toggle background update: " << (doUpdateModel ? "ON" : "OFF") << endl;
    }

    if (key == 'l')
    {
        model->clear();
        cout << "Cleaned background subtractor history." << endl;
    }





}

