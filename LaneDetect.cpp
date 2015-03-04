#include "opencv2/highgui/highgui.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "linefinder.h"

#define PI 3.1415926

using namespace cv;

int main(int argc, char* argv[]) {
	int weight = 120;
	bool showSteps = true;

	string window_name = "Processed Video";
	namedWindow(window_name, CV_WINDOW_KEEPRATIO); //resizable window;
	
std::string pathToData("images/mono_%010d.png");
cv::VideoCapture sequence(pathToData);
Mat image;
	
        while (1)
        {

sequence >> image; // Reads cap_00000001.bmp

            if (image.empty())
                break;
            	Mat gray;
            	cvtColor(image,gray,CV_RGB2GRAY);
            	vector<string> codes;
            	Mat corners;
            	findDataMatrix(gray, codes, corners);
            	drawDataMatrixCodes(image, codes, corners);

		Rect roi(0,image.cols/3,image.cols-1,image.rows - image.cols/3);// set the ROI for the image
		Mat imgROI = image(roi);
    // Display the image
	if(showSteps){
		//namedWindow("Original Image");
		//imshow("Original Image",imgROI);
		//imwrite("original.bmp", imgROI);
	}

   // Canny algorithm
	Mat contours;
	Canny(imgROI,contours,50,200);
	Mat contoursInv;
	threshold(contours,contoursInv,128,255,THRESH_BINARY_INV);

   // Display Canny image
	if(showSteps){
		namedWindow("Contours");
		imshow("Contours1",contoursInv);
		imwrite("contours.bmp", contoursInv);
	}

 /* 
	Hough tranform for line detection with feedback
	Increase by 25 for the next frame if we found some lines.  
	This is so we don't miss other lines that may crop up in the next frame
	but at the same time we don't want to start the feed back loop from scratch. 
*/
	std::vector<Vec2f> lines;
	//if (weight < 1 or lines.size() > 2){ // we lost all lines. reset 
	//	weight = 100; 
	//}
	//else{ weight += 25;} 
	//while(lines.size() < 5 && weight > 0){
		HoughLines(contours,lines,1,PI/180, weight);
	//	weight -= 5;
	//}
	std::cout << weight << "\n";
	Mat result(imgROI.size(),CV_8U,Scalar(255));
	imgROI.copyTo(result);

   // Draw the limes
	std::vector<Vec2f>::const_iterator it= lines.begin();
	Mat hough(imgROI.size(),CV_8U,Scalar(0));
	while (it!=lines.end()) {

		float rho= (*it)[0];   // first element is distance rho
		float theta= (*it)[1]; // second element is angle theta
		
		if ( theta > 0.09 && theta < 1.48 || theta < 3.14 && theta > 1.66 ) { // filter to remove vertical and horizontal lines
		
			// point of intersection of the line with first row
			Point pt1(rho/cos(theta),0);        
			// point of intersection of the line with last row
			Point pt2((rho-result.rows*sin(theta))/cos(theta),result.rows);
			// draw a white line
			line( result, pt1, pt2, Scalar(255), 8); 
			line( hough, pt1, pt2, Scalar(255), 8);
		}

		//std::cout << "line: (" << rho << "," << theta << ")\n"; 
		++it;
	}

    // Display the detected line image
	if(showSteps){
		namedWindow("Detected Lines with Hough");
		imshow("Detected Lines with Hough",result);
		imwrite("hough.bmp", result);
	}
   // Create LineFinder instance
	LineFinder ld;

   // Set probabilistic Hough parameters
	ld.setLineLengthAndGap(60,5);
	ld.setMinVote(4);

   // Detect lines
	std::vector<Vec4i> li= ld.findLines(contours);
	Mat houghP(imgROI.size(),CV_8U,Scalar(0));
	ld.setShift(0);
	ld.drawDetectedLines(houghP);
	std::cout << "First Hough" << "\n";

	if(showSteps){
		//namedWindow("Detected Lines with HoughP");
		//imshow("Detected Lines with HoughP", houghP);
		//imwrite("houghP.bmp", houghP);
	}

   // bitwise AND of the two hough images
	bitwise_and(houghP,hough,houghP);
	Mat houghPinv(imgROI.size(),CV_8U,Scalar(0));
	Mat dst(imgROI.size(),CV_8U,Scalar(0));
	threshold(houghP,houghPinv,150,255,THRESH_BINARY_INV); // threshold and invert to black lines

	if(showSteps){
		//namedWindow("Detected Lines with Bitwise");
		//imshow("Detected Lines with Bitwise", houghPinv);
	}

	Canny(houghPinv,contours,100,350);
	li= ld.findLines(contours);
   // Display Canny image
	if(showSteps){
		//namedWindow("Contours");
		//imshow("Contours2",contours);
		//imwrite("contours.bmp", contoursInv);
	}

	   // Set probabilistic Hough parameters
	ld.setLineLengthAndGap(5,2);
	ld.setMinVote(1);
	ld.setShift(image.cols/3);
	ld.drawDetectedLines2(image);
		
	std::stringstream stream;
	stream << "Lines Segments: " << lines.size();
	
	putText(image, stream.str(), Point(10,image.rows-10), 2, 0.8, Scalar(0,0,255),0);
        imshow(window_name, image); 
	imwrite("processed.bmp", image);


	waitKey(0);

 	lines.clear();
	}
}




