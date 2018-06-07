Mat drawing = Mat::zeros( foregroundMask.size(), CV_8UC3 );
        if(!hull.empty()){
        	for( int i = 0; i< contours.size(); i++ ) // iterate through each contour. 
      		{
       			double a=contourArea( contours[i],false);  //  Find the area of contour
       			if(a>largest_area){
       				largest_area=a;
       				largest_contour_index=i;                //Store the index of largest contour
       			
       			}
  
      		}


      		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
      		drawContours( drawing, hull, largest_contour_index, color, 1, 8, vector<Vec4i>(), 0, Point() );





        }


/*

 /// Find the convex hull object for each contour ///
        vector<vector<Point>> hull(contours.size());
        for( int i = 0; i < contours.size(); i++ )
        {
            convexHull( Mat(contours[i]), hull[i], false );
        }

        /// Draw contours + hull results
        Mat drawing = Mat::zeros( foregroundMask.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
        {
            //changing the color of contour
            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }

        /// End of contours and hull


        /// Drawing min rectangle in contours
        /// Find the rotated rectangles and ellipses for each contour
        vector<RotatedRect> minRect( contours.size() );

        for( int i = 0; i < contours.size(); i++ )
            minRect[i] = minAreaRect( Mat(contours[i]) );
        

        /// Draw rotated rects
        
        for( int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
            // rotated rectangle
            Point2f rect_points[4];
            minRect[i].points( rect_points );
            for( int j = 0; j < 4; j++ )
                line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
        }
        /// End of min rectangle

        //imshow("Hand Subtraction", foregroundMask);
        imshow("Fingertip identification", drawing);
    */
        // show background image
        /*