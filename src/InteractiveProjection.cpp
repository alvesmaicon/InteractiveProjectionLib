#include "InteractiveProjection.hpp"


// the constructor
InteractiveProjection::InteractiveProjection()
{

}

// the destructor
InteractiveProjection::~InteractiveProjection()
{

}

// inicializating the system
void InteractiveProjection::Start(int cameraId)
{
    RNG rng(12345);

    cap.open(cameraId);
    if( !cap.isOpened())
    {
        cout << "Could not initialize video (" << cameraId << ") capture" << endl;
        exit(0);
    }


    cap >> capturedImage;

    // setting the camera resolution and video format
    cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);


    // reading chessboard image
    chessboardMatrix = imread("images/pattern-800x600.png", CV_LOAD_IMAGE_COLOR);
    if (chessboardMatrix.empty())
    {
        cout << "Failed imread():  chesboard image not found" << endl;
        exit(0);
    }

    boardSize.width = 9;
    boardSize.height = 6;


    // finding chessboard corners in chesboard image
    foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                          CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


    projectedImage = chessboardMatrix;

    namedWindow("Projection", WINDOW_NORMAL);
    imshow("Projection", projectedImage);

    namedWindow("Detcted Interaction", WINDOW_NORMAL);

    namedWindow("Captured Image", WINDOW_NORMAL);


    // creating background subtractor model
    model = createBackgroundSubtractorMOG2();
    // setting to not detect shadows
    model->setDetectShadows(true);
    // setting tolerance matching and shadow value
    model->setShadowValue(0);
    model->setVarThreshold(40);

    drawing = Mat::zeros( projectedImage.size(), CV_8UC3 );
}


// showing capturede image
void InteractiveProjection::ShowCapturedImage()
{
    
    imshow("Captured Image", capturedImage);
}

// showing detected interaction with projection
void InteractiveProjection::ShowDetectedInteraction()
{

    imshow("Detcted Interaction", drawing);
}

// appling background subtraction model and undistorting captured image if solicited
void InteractiveProjection::Apply()
{
    cap >> capturedImage;
    

    if(undistortImage)
    {
        DoUndistortCapture();
    }

    if(detectingTouch)
    {

        cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // setting exposure to manual
        cap.set(CAP_PROP_AUTOFOCUS, 0); // turn the autofocus off

        // pass the frame to background subtraction model
        model->apply(capturedImage, foregroundMask, doUpdateModel ? -1 : 0);


        Mat kernel = getStructuringElement(0, Size(5,5), Point(-1,-1) );

        // removing noise in mask using morphology filter
        morphologyEx(foregroundMask, foregroundMask, MORPH_OPEN, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );
        morphologyEx(foregroundMask, foregroundMask, MORPH_CLOSE, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );

        /// Finding contours in mask
        findContours( foregroundMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        /*
            Find max contour area then find hull to this contour.
            - find center point to this contour
            - find convex points to this contour.
            - find bigger distance between center and convex point. 
            The result will be a fingertip point
        */
        double max_area = 0;
        
        vector<vector<Point>> hull(contours.size());

        // cleaning drawing image
        drawing = Mat::zeros( foregroundMask.size(), CV_8UC3 );

        // drawing contour, fingertip and lines in hand region image
        if(!contours.empty())
        {
            // finding the larger contour
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

            // drawing circle in centroid point
            circle(drawing, centroid, 12, Scalar(255,0,0), -1, 8, 0);

            convexHull(contours[max_contour], hull[max_contour], false, false);
            // drawing lines in contour
            drawContours( drawing, hull, max_contour, color, 4, 8, vector<Vec4i>(), 0, Point() );

            
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

            // drawing a circle in fingertip point
            circle(drawing, fingertip, 12, Scalar(0,0,255), -1, 8, 0);


        }

    }
}

// verifying if exists a fingertip interaction in a rectangular region
bool InteractiveProjection::FingertipInteraction(Rect rectangle){
    if(!contours.empty()){
        if(rectangle.contains(fingertip))
            return true;
    }

    return false;
}

// verifying if exists a interaction in a rectangular region
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

// verifying if exists a fingertip interaction in a circular region
bool InteractiveProjection::FingertipInteraction(Point center, double radius){

    double dist = norm(fingertip - center);
    if (dist < radius)
        return true;
    return false;
}

// verifying if exists a interaction in a circular region
bool InteractiveProjection::RegionInteraction(Point center, double radius){

    
    for(int i = 0; i < contours[max_contour].size(); i++)
    {
        double dist = norm(contours[max_contour][i] - center);
        if (dist < radius)
        return true;
    }

    return false;
}

// updating projected image with a input image
void InteractiveProjection::UpdateProjectedImage(Mat image)
{
    projectedImage = image;
}

// showing projected image
void InteractiveProjection::ShowProjectedImage()
{
    imshow("Projection", projectedImage);
}

// getting homography H
void InteractiveProjection::GetHomography()
{
    // finding chessborad corners in captured image
    bool foundCornersView = findChessboardCorners( capturedImage, boardSize, cornersCameraView,
                            CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


    if(foundCornersView && foundCornersPattern)
    {
        // drawing chessboard corners in captured image
        drawChessboardCorners( capturedImage, boardSize, Mat(cornersCameraView), foundCornersView );

        // finding homography relation between projected image and captured image
        H = findHomography(cornersCameraView, cornersPattern);
        mode = CALIBRATED;
        cout << "Calibrated" << endl;

    }
    // if occurs a calibration failure, the system will detect it
    else
    {
        mode = DETECTION;
        cout << "Calibration failed" << endl;
    }

}

// undistorting captured image
void InteractiveProjection::DoUndistortCapture()
{
    Mat temp = capturedImage.clone();

    /*undistorting camera image view to top center perspective*/
    warpPerspective(temp, capturedImage, H, temp.size());
}


// keyboard interaction with system
void InteractiveProjection::KeyListener()
{
    const char key = (char)waitKey(30);

    // key for quit
    if (key == 27 || key == 'q')
    {
        cout << "Exit requested" << endl;
        exit(0);
    }

    // key for calibrate homography
    if( cap.isOpened() && key == 'c' && mode == DETECTION)
    {
        mode = CAPTURING;
        projectedImage = chessboardMatrix;
        imshow("Projection", projectedImage);
        GetHomography();
        return;
    }

    // key for calibrate homography if system is already calibrated
    if( cap.isOpened() && key == 'c' && mode == CALIBRATED && !undistortImage)
    {
        mode = CAPTURING;
        projectedImage = chessboardMatrix;
        imshow("Projection", projectedImage);
        GetHomography();
    }

    // key for undistort captured image
    if( key == 'u' && mode == CALIBRATED )
    {
        undistortImage = !undistortImage;
        cout << "Toggle undistort image: " << (undistortImage ? "ON" : "OFF") << endl;
    }

    // key for start interaction detection
    if(key == 'd' )
    {
        detectingTouch = !detectingTouch;
        cout << "Toggle detecting touch: " << (detectingTouch ? "ON" : "OFF") << endl;
    }

    // key for change background subtractor learn factor
    if (key == ' ')
    {
        doUpdateModel = !doUpdateModel;
        cout << "Toggle background update: " << (doUpdateModel ? "ON" : "OFF") << endl;
    }

    // key for clear background subtractor model
    if (key == 'l')
    {
        model->clear();
        cout << "Cleaned background subtractor history." << endl;
    }
}