#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>


// Using this to calcule contours



using namespace std;
using namespace cv;


double euclideanDist(Point& p, Point& q) {
    Point diff = p - q;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}



int main(int argc, const char** argv)
{
    const String keys = "{c camera||use video stream from camera (default is NO)}"
                        "{fn file_name|../data/tree.avi|video file}"
                        "{m method|mog2|method: background subtraction algorithm ('knn', 'mog2')}"
                        "{h help||show help message}";
    CommandLineParser parser(argc, argv, keys);
    parser.about("This sample demonstrates background segmentation.");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    bool useCamera = parser.has("camera");
    String file = parser.get<String>("file_name");
    String method = parser.get<String>("method");
    if (!parser.check())
    {
        parser.printErrors();
        return 1;
    }


    Mat img = Mat::zeros(800, 600, CV_8UC1);
    Mat inputFrame, frame, foregroundMask, foreground, background;
    Mat croppedCapture = Mat::zeros(200, 200, CV_8UC1);
    Mat croppedProjectedImage = Mat::zeros(200, 200, CV_8UC1);

    VideoCapture cap;

    cap.open(1);

    cap >> frame;


    // setting exposure to manual
    cap.set(CAP_PROP_AUTO_EXPOSURE, 0.25); 
    // turn off autofocus
    cap.set(CAP_PROP_AUTOFOCUS, 0); 

    // setting the camera resolution
    cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);




    Ptr<BackgroundSubtractorMOG2> model;
    model = createBackgroundSubtractorMOG2();
    // setting to not detect shadows
    //model->setDetectShadows(true);
    // setting tolerance match
    model->setShadowValue(0);
    model->setVarThreshold(40);




    cout << "Press <space> to toggle background model update" << endl;
    cout << "Press 's' to toggle foreground mask smoothing" << endl;
    cout << "Press ESC or 'q' to exit" << endl;
    bool doUpdateModel = false;
    bool doSmoothMask = false;




    RNG rng(12345);

    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;


    for (;;)
    {
        // prepare input frame
        cap >> frame;
        if (frame.empty())
        {
            cout << "Finished reading: empty frame" << endl;
            break;
        }



        // pass the frame to background model
        model->apply(frame, foregroundMask, doUpdateModel ? -1 : 0);

        // show processed frame
        

        // show foreground image and mask (with optional smoothing)

        GaussianBlur(foregroundMask, foregroundMask, Size(11, 11), 3.5, 3.5);
        threshold(foregroundMask, foregroundMask, 130, 255, THRESH_BINARY);



        if (foreground.empty())
            foreground.create(frame.size(), frame.type());
        foreground = Scalar::all(0);
        frame.copyTo(foreground, foregroundMask);

        Mat kernel = getStructuringElement(0, Size(5,5), Point(-1,-1) );
        morphologyEx(foregroundMask, foregroundMask, MORPH_OPEN, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );


        //morphologyEx(foregroundMask, foregroundMask, MORPH_CLOSE, kernel,  Point(-1,-1), 1, BORDER_CONSTANT,  morphologyDefaultBorderValue() );
        /// Find contours
        findContours( foregroundMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        /*
			Find max contour area than find hull to this contour.
			Then find center point to this contour
			Then find convex points to this contour.
			Nowfind bigger distance between center and convex.
			Will be a little hope to find the correct point.
        */
        double max_area = 0;
        int max_contour;
        vector<vector<Point>> hull(contours.size());
        Mat drawing = Mat::zeros( foregroundMask.size(), CV_8UC3 );
        if(!contours.empty()){


        	for(int i = 0; i < contours.size(); i++){
        		double a = contourArea( contours[i],false);
        		if(a > max_area){
        			max_area = a;
        			max_contour = i;
        		}
        	}

        	
        	drawContours( drawing, contours, max_contour, Scalar(255,255,255), -1, 8, vector<Vec4i>(), 0, Point() );

        	//Moments m = moments(drawing, true);
			//Point p1(m.m10/m.m00, m.m01/m.m00);

			Moments mu = moments(contours[max_contour]);
    		Point centroid = Point (mu.m10/mu.m00 , mu.m01/mu.m00);

			Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
			
			circle(drawing, centroid, 3, Scalar(255,0,0), -1, 8, 0);
        	convexHull(contours[max_contour], hull[max_contour], false, false);
        	drawContours( drawing, hull, max_contour, color, 1, 8, vector<Vec4i>(), 0, Point() );


        	Point fingertip;
        	double max_dist = 0;
        	for(int i = 0; i < hull[max_contour].size(); i++){
        		double dist = norm(centroid - hull[max_contour][i]);
        		if(dist > max_dist){
        			max_dist = dist;
        			fingertip = hull[max_contour][i];
        		}
        	}


        	circle(drawing, fingertip, 5, Scalar(0,0,255), -1, 8, 0);



        	/// CROPPING IMAGE TO FINGERTIP RECT 200X200
        	int startX, startY;
        	// Limited to border left and top
        	startX = fingertip.x - 100 < 0 ? 0 : fingertip.x - 100;
        	startY = fingertip.y - 100 < 0 ? 0 : fingertip.y - 100;
        	// limited to border right and bottom
        	startX = startX + 200 >= 800 ? 600 : startX;
        	startY = startY + 200 >= 600 ? 400 : startY;

        	cout << "StartX: " << startX << ", endX: " << startX + 200 << endl;
        	cout << "StartY: " << startY << ", endY: " << startY + 200 << endl;
        	
        	Rect croppedRect(startX, startY, 200, 200);
        	croppedCapture = frame(croppedRect);

        }
        imshow("image", frame);
        imshow("Fingertip rectangle", croppedCapture);
        imshow("Fingertip identification", drawing);


        
        model->getBackgroundImage(background);
        if (!background.empty()){
        	croppedProjectedImage = background(croppedRect);
        }
            
        
        
        // interact with user
        const char key = (char)waitKey(30);
        if (key == 27 || key == 'q') // ESC
        {
            cout << "Exit requested" << endl;
            break;
        }
        if( key == 'w')
        {
            imwrite( "capturedImage-.jpg", frame);
        }

        else if (key == ' ')
        {
            doUpdateModel = !doUpdateModel;
            cout << "Toggle background update: " << (doUpdateModel ? "ON" : "OFF") << endl;
        }
        else if (key == 's')
        {
            doSmoothMask = !doSmoothMask;
            cout << "Toggle foreground mask smoothing: " << (doSmoothMask ? "ON" : "OFF") << endl;
        }
    }
    return 0;
}
