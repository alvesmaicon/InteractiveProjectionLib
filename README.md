## Interactive Projection Library
This library was developed for use in interactive projection applications. The library is based on OpenCV and has methods for recognizing interactions with objects in the projection.

To run the example, compile with cmake.
OpenCV 3.0 is required.
### Run example with
 * cmake .
 * make
 * ./sample
 
### Keys to interaction

  Key | Description
------| -----------------------------------------------
'c'   | capture chessboeard corners to find homography
'u'   | undistort captured image                      
'd'   | start interaction detection                   
' '   | active recursive training of background       
'l'   | clear background subtractor history           
esc   | quit application                              

### Interactive projection game made with InteractiveProjectionLib
Make your own interactive projection applications using an opencv open source library.
[![](http://img.youtube.com/vi/FUYwdyEmzTk/0.jpg)](http://www.youtube.com/watch?v=FUYwdyEmzTk "")
