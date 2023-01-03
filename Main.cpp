#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <opencv2/calib3d/calib3d_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;

char lookup[256];

int iLowH = 35;
int iHighH = 70;
int iLowS = 116; 
int iHighS = 206;

int iLowV = 32;
int iHighV = 113;


int iLastX = -1; 
int iLastY = -1;

Mat imgLines;

VideoCapture cap(0);

void lookupInit() {
    for(int i = 0; i < 256; ++i) {
        lookup[i] =((i/10) * 10);
    }

}


//Color space reduction
void reduction( Mat *image) {

    CV_Assert(image->depth() == CV_8U);

    int channels = image->channels();

    int rows = image->rows;

    int columns = image->cols * channels;

    uchar* p;


    for(int i = 0; i < rows; i++) {

        p = image->ptr<uchar>(i);

        for(int j = 0; j < columns; j++) {
            p[j] = lookup[p[j]];
            
            
        }
    }
}


Mat greenFilter(Mat * image) {

    Mat convImg;

     //Capture a temporary image from the camera
    Mat imgTmp;
    cap.read(imgTmp); 


    //Create a black image with the size as the camera output
    imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );

    reduction(image);

    cvtColor(*image, convImg, COLOR_BGR2HSV);

    Mat greenOnly;

    inRange(convImg, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), greenOnly);

      //morphological opening (remove small objects from the foreground)
    erode(greenOnly, greenOnly, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( greenOnly, greenOnly, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

    //morphological closing (fill small holes in the foreground)
    dilate( greenOnly, greenOnly, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
    erode(greenOnly, greenOnly, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //Calculate the moments of the thresholded image
    Moments oMoments = moments(greenOnly);

    double dM01 = oMoments.m01;
    double dM10 = oMoments.m10;
    double dArea = oMoments.m00;
    
    cout << "Area = " << dArea << endl;
    // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
    if (dArea > 1000000)
    {
        //calculate the position of the object
        int posX = dM10 / dArea;
        int posY = dM01 / dArea;        
            
        cout << posX << endl;
        cout << posY << endl;
    }


    return greenOnly;

}



int main() {


    namedWindow("Control", WINDOW_AUTOSIZE); //create a window called "Control"

   

    //Create trackbars in "Control" window
    createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179);

    createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255);

    createTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255);


    

    bool loopin = true;
    Mat image;
    cv::namedWindow("DisplayWindow");
    lookupInit();

    if(!cap.isOpened()) {
        cout << "Cannot open camera\n";
    }

    while(loopin){
        cap >> image;

        Mat greenOnly = greenFilter(&image);

        imshow("DisplayWindow", greenOnly);
        int k = waitKey(25);

        if(k == 'q') {
            loopin = false;
        }

    }


    return 0;
}