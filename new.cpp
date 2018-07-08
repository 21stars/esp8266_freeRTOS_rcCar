#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(int, char**)
{
	String video_name = "solidWhiteRight.mp4";
	VideoCapture cap(video_name);
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	namedWindow("edges", 1);
	for (;;)
	{
		Mat frame, gray, edges, ROI, white, yellow, mask;
		vector<Vec4i> lines;

		cap >> frame; // get a new frame from camera

		inRange(frame, Scalar(200, 200, 200), Scalar(255, 255, 255), white);
		inRange(frame, Scalar(0, 150, 150), Scalar(150, 200, 255), yellow);
		mask = white + yellow;

		imshow("white", white);
		imshow("mask", mask);

		//img smoothing
		GaussianBlur(mask, mask, Size(7, 7), 1.5, 1.5);
		//Canny
		Canny(mask,edges, 0, 30, 3);
		//HoughLine
		HoughLinesP(mask, lines, 1, CV_PI / 180, 50, 50, 50);

		//Angle Filtering
		for (int i = 0; i < lines.size(); i++) {
			Point A(lines[i][0], lines[i][1]), B(lines[i][2], lines[i][3]);
			Vec4i l = lines[i];
			double angle = atan2((A.y - B.y), (A.x - B.x)) * 180 / CV_PI;
			if (((angle < -30) && (angle > -60)) || ((angle < 150) && (angle > 120)) || ((angle > 30) && (angle < 60)) || ((angle > -150) && (angle < -120)))
				line(frame, A, B, Scalar(0, 0, 255), 3, 8);
		}

		imshow("edges", edges);
		imshow("houghLine", frame);

		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}