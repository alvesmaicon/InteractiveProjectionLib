#include "opencv2/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/video.hpp"
#include <iostream>
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iomanip>


// Using this to calcule contours



using namespace std;
using namespace cv;




enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2};

int main(int argc, const char** argv)
{
    int count_save = 0;
    
    Size boardSize;

    int mode = DETECTION;

    Mat capturedImage, projectedImage, H, foregroundMask, foreground, background, img1, img2, img3, img4, img5, img6, chessboardMatrix;


    vector<Point2f> cornersPattern, cornersCameraView;

    boardSize.width = 9;
    boardSize.height = 6;

    /* Code for open chessboard image and find corners*/
    chessboardMatrix = imread("images/pattern-800x600.png", CV_LOAD_IMAGE_COLOR);
    if (chessboardMatrix.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    bool foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                               CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


    /* Code for open image*/
    img1 = imread("images/img1.jpg", CV_LOAD_IMAGE_COLOR);
    if (img1.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    img2 = imread("images/img2.jpg", CV_LOAD_IMAGE_COLOR);
    if (img2.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    img3 = imread("images/img3.jpg", CV_LOAD_IMAGE_COLOR);
    if (img3.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    img4 = imread("images/img4.jpg", CV_LOAD_IMAGE_COLOR);
    if (img4.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    img5 = imread("images/img5.jpg", CV_LOAD_IMAGE_COLOR);
    if (img5.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;

    img6 = imread("images/img6.jpg", CV_LOAD_IMAGE_COLOR);
    if (img6.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;


    projectedImage = chessboardMatrix;

    namedWindow("Projection", WINDOW_NORMAL);
    imshow("Projection", projectedImage);

    /* Selected camera is usb camera by default*/
    int cameraId = 1;


    VideoCapture cap;

    cap.open(cameraId);

    if( !cap.isOpened())
        return fprintf( stderr, "Could not initialize video (%d) capture\n",cameraId ), -2;

    cap >> capturedImage;

    // setting the camera resolution
    cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);


    cap >> capturedImage;

    Mat drawing = Mat::zeros( capturedImage.size(), CV_8UC3 );

    Ptr<BackgroundSubtractorMOG2> model;
    model = createBackgroundSubtractorMOG2();
    // setting to not detect shadows
    model->setDetectShadows(true);
    // setting tolerance match
    model->setShadowValue(0);
    model->setVarThreshold(40);


    bool doUpdateModel = false, undistortImage = false, detectingTouch = false;



    RNG rng(12345);


    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;



    for (;;)
    {
        // prepare input frame
        cap >> capturedImage;
        if (capturedImage.empty())
        {
            cout << "Finished reading: empty frame" << endl;
            break;
        }

        if( mode == CAPTURING )
        {

            bool foundCornersView = findChessboardCorners( capturedImage, boardSize, cornersCameraView,
                                    CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


            if(foundCornersView && foundCornersPattern)
            {
                drawChessboardCorners( capturedImage, boardSize, Mat(cornersCameraView), foundCornersView );


                H = findHomography(cornersCameraView, cornersPattern);
                mode = CALIBRATED;

            }

        }

        if( mode == CALIBRATED && undistortImage)
        {
            Mat temp = capturedImage.clone();

            /*undistorting camera image view to top center perspective*/
            warpPerspective(temp, capturedImage, H, temp.size());
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
            int max_contour;
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

                Point fingertip;
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







        imshow("image", capturedImage);
        imshow("Fingertip identification", drawing);
        imshow("Projection", projectedImage);







        // interact with user
        const char key = (char)waitKey(30);
        if (key == 27 || key == 'q') // ESC
        {
            cout << "Exit requested" << endl;
            break;
        }
        if( key == 'w')
        {
            stringstream captured_image_name, contour_image_name ;

            captured_image_name << std::setw(3) << std::setfill('0') << count_save << "_captured.jpg";
                                imwrite( captured_image_name.str(), capturedImage);

            contour_image_name << std::setw(3) << std::setfill('0') << count_save << "_contour.jpg";
                               imwrite( contour_image_name.str(), drawing);

            count_save ++;
        }

        if( cap.isOpened() && key == 'c' && mode == DETECTION)
        {
            mode = CAPTURING;
        }

        if(key == 'd' )
        {
            detectingTouch = !detectingTouch;
            cout << "Toggle detecting touch: " << (detectingTouch ? "ON" : "OFF") << endl;
        }

        if(key == '1')
        {
            projectedImage = img1;
            imshow("Projection", projectedImage);
        }

        if(key == '2')
        {
            projectedImage = img2;
            imshow("Projection", projectedImage);
        }

        if(key == '3')
        {
            projectedImage = img3;
            imshow("Projection", projectedImage);
        }

        if(key == '4')
        {
            projectedImage = img4;
            imshow("Projection", projectedImage);
        }

        if(key == '5')
        {
            projectedImage = img5;
            imshow("Projection", projectedImage);
        }

        if(key == '6')
        {
            projectedImage = img6;
            imshow("Projection", projectedImage);
        }

        if( key == 'u' && mode == CALIBRATED )
        {
            undistortImage = !undistortImage;
            cout << "Toggle undistort image: " << (undistortImage ? "ON" : "OFF") << endl;
        }

        else if (key == ' ')
        {
            doUpdateModel = !doUpdateModel;
            cout << "Toggle background update: " << (doUpdateModel ? "ON" : "OFF") << endl;
        }

    }
    return 0;
}
