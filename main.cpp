#include "opencv2/highgui/highgui.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string>
#include <stdlib.h> 
#include <unistd.h>
#include "linefinder.h"

#define PI 3.1415926

using namespace cv;


int main(int argc, char* argv[]) {

int k = 0;
int x = 2376;
std::ofstream intercepts; 
intercepts.open("intercepts.csv", std::ofstream::app); 

	int weight = 120;
	
	string window_name = "Output";
	namedWindow(window_name);
	
	
 
while(k!=x)
{
std::stringstream ss;
String s;
if (k == 348)
k = 2050;

s = "mono_000000000";
if (k>9 && k<100)
s = "mono_00000000";
else if (k>99 && k<1000)
s = "mono_0000000";
else if (k>999 && k<10000)
s = "mono_000000";
ss<<"images/"<<s<<k<<".png";
string fullPath = ss.str();
std::cout<<fullPath;

Mat image = imread(fullPath);

            	Mat gray;
            	cvtColor(image,gray,CV_RGB2GRAY);
            	vector<string> codes;
            	Mat corners;
            	findDataMatrix(gray, codes, corners);
            	drawDataMatrixCodes(image, codes, corners);

		Rect roi(0,image.cols/3,image.cols-1,image.rows - image.cols/3);
		Mat imgROI = image(roi);
    

   // Canny
	Mat contours;
	Canny(imgROI,contours,50,200);
	Mat contoursInv;
	//adaptiveThreshold( contours, contours, 255,CV_ADAPTIVE_THRESH_MEAN_C,CV_THRESH_BINARY_INV,13, 1 );

	threshold(contours,contoursInv,128,255,THRESH_BINARY_INV);

  	std::vector<Vec2f> lines;
	HoughLines(contours,lines,1,PI/180, weight);
	Mat result(imgROI.size(),CV_8U,Scalar(255));
	imgROI.copyTo(result);

   // Draw lines
	std::vector<Vec2f>::const_iterator it= lines.begin();
	Mat hough(imgROI.size(),CV_8U,Scalar(0));
	while (it!=lines.end()) {

		float rho= (*it)[0];   
		float theta= (*it)[1]; 
		
		if ( theta < 1 || theta > 2) { 
		
			Point pt1(rho/cos(theta),0);        
			Point pt2((rho-result.rows*sin(theta))/cos(theta),result.rows);
			line( result, pt1, pt2, Scalar(255), 8); 
			line( hough, pt1, pt2, Scalar(255), 8);
		}
		++it;
	}

    LineFinder ld;

	ld.setLineLengthAndGap(60,5);
	ld.setMinVote(4);

   // Detect lines
	std::vector<Vec4i> li= ld.findLines(contours);
	Mat houghP(imgROI.size(),CV_8U,Scalar(0));
	ld.setShift(0);
	ld.drawDetectedLines(houghP);

	bitwise_and(houghP,hough,houghP);
	Mat houghPinv(imgROI.size(),CV_8U,Scalar(0));
	Mat dst(imgROI.size(),CV_8U,Scalar(0));
	//adaptiveThreshold( houghP, houghP, 255,CV_ADAPTIVE_THRESH_MEAN_C,CV_THRESH_BINARY_INV,13, 1 );
	threshold(houghP,houghPinv,150,255,THRESH_BINARY_INV);
	
	Canny(houghPinv,contours,100,350);
	li= ld.findLines(contours);
	
	   //Hough parameters
	ld.setLineLengthAndGap(5,2);
	ld.setMinVote(1);
	ld.setShift(image.cols/3);
	ld.drawDetectedLines2(image);
	
	
        



intercepts << s <<k<<", "<< linx<<", "<< linx2 <<std::endl;
k++;

imshow(window_name, image); 


	//usleep(600);
waitKey(1);

 	lines.clear();
	}
intercepts.close();
}




