
#include "DetectionObstacle.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "opencv\cv.h"


#include <opencv2/opencv.hpp>
#include "opencv\highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>
using namespace cv;
using namespace std;

void resetHistogram(long A[])
{

	int j;
	for (j = 0; j <361; j++)
	{
		A[j] = 0;
	}
}

void filterAverage(long histogram[], int radius, int size)
{
	long filtred_histogram[361];
	if (size < 2 * radius + 1)
	{
		cout << "error" << endl;
		return;
	}
	resetHistogram(filtred_histogram);
	for (int i = 1; i <= radius; i++)
	{
		for (int j = 1; j <= i + radius; j++)
		{
			filtred_histogram[i] = filtred_histogram[i] + histogram[j];
		}
		filtred_histogram[i] = filtred_histogram[i] / (2 * radius + 1);
	}
	for (int i = size - radius + 1; i <= radius; i++)
	{
		for (int j = i - radius; j <= size; j++)
		{
			filtred_histogram[i] = filtred_histogram[i] + histogram[j];
		}
		filtred_histogram[i] = filtred_histogram[i] / (2 * radius + 1);
	}
	for (int i = radius + 1; i <= size - radius; i++)
	{
		for (int j = i - radius; j <= i + radius; j++)
		{
			filtred_histogram[i] = filtred_histogram[i] + histogram[j];
		}
		filtred_histogram[i] = filtred_histogram[i] / (2 * radius + 1);
	}
	for (int i = 1; i <= size; i++)
	{
		histogram[i] = filtred_histogram[i];
	}
	return;
}

void validatePixel(Mat im, Mat *mask, int min_intensity, int min_staturation)
{
	int nbrows = im.rows;
	int nbcols = im.cols;
	int i, j;
	for (i = 0; i < nbrows; i++)
	{
		for (j = 0; j < nbcols; j++)
		{
			Vec3b pixel = im.at<Vec3b>(i, j);
			mask->at<uchar>(i, j) = (pixel.val[2] > min_intensity) + ((pixel.val[1] > min_staturation) << 1);
		}
	}

}
// show the histograme of the interested zone
void showHistogram(long histogramH[], long histogramV[])
{
	int i;
	for (i = 1; i < 360; i++)
	{
		printf("%7d   %7d  \n", histogramH[i], histogramV[i]);
	}
}


//
void fonction_trapeze(Mat img, int d, int h, double alpha)
{

	int posX = img.rows;
	int posY = img.cols;

	int linetype = 4;
	Point trapeze[1][4];
	alpha = alpha * 0.01744;
	trapeze[0][0] = Point(d, posX - 10);
	trapeze[0][1] = Point(d + cos(alpha)*h, posX - 10 - sin(alpha)*h);
	trapeze[0][2] = Point(posY - d - cos(alpha)*h, posX - 10 - sin(alpha)*h);
	trapeze[0][3] = Point(posY - d, posX - 10);
	const Point* pts[1] = { trapeze[0] };
	int npts[] = { 4 };
	polylines(img, pts, npts, 1, 1, Scalar(0, 255, 0), 1, 8, 0);

}

// histograme calculation in the interestd zone. 
void calHistogram(Mat img, int d, int h, float alpha, long histogramH[], long histogramV[], Mat masque)
{
	alpha = alpha * 0.01744;
	int nbr_rows = img.rows;
	int nbr_cols = img.cols;
	int X0 = d;
	int Y0 = nbr_rows - 10;
	int Y1 = nbr_rows - 10 - sin(alpha)*h;
	int X3 = nbr_cols - d;
	int i, j;
	Mat img_ext(nbr_rows, nbr_cols, img.type());
	resetHistogram(histogramH);
	resetHistogram(histogramV);

	//calcule the apparence nombre of each value H,V
	for (j = Y1; j<Y0; j++)
	{
		int first_x = X0 + (nbr_rows - 10 - j) / tan(alpha);
		int last_x = X3 - (nbr_rows - 10 - j) / tan(alpha);
		for (i = first_x; i <= last_x; i++)
		{
			Vec3b data = img.at<Vec3b>(Point(i, j));
			histogramH[data.val[0]] = histogramH[data.val[0]] + (masque.at<uchar>(j, i) == 3);
			histogramV[data.val[2]]++;
			img_ext.at<Vec3b>(Point(i, j)) = data;
		}
	}
}

//with each value H and V, we verifie the 2 conditions:
//1. The  hue  histogram  bin  value  at  the  pixel’ s  hue value is below the hue threshold
//2. The  intensity  histogram  bin  value  at  the  pixel’s intensity value is below the intensity threshold
void classifyHV(long histogramH[], long histogramV[], long hue_thresholds, long intensity_thresholds)
{
	for (int i = 0; i < 361; i++)
	{
		histogramH[i] = (histogramH[i] < hue_thresholds);
		histogramV[i] = (histogramV[i] < intensity_thresholds);
	}
}

Mat detectObstacle(Mat im, long histogramH[], long histogramV[])
{
	Mat detection(im.rows, im.cols, CV_8UC1);
	int nbrows = im.rows;
	int nbcols = im.cols;
	int i, j;
	for (i = 0; i < nbrows; i++)
	{
		for (j = 0; j < nbcols; j++)
		{
			Vec3b pixel = im.at<Vec3b>(i, j);
			detection.at<uchar>(i, j) = 255 * histogramH[pixel.val[0]] * histogramV[pixel.val[2]];
		}
	}
	return detection;
	
}

Mat detect(Mat img, int d, int h, float alpha, int min_intensity, int min_staturation, long hue_thresholds, long intensity_thresholds)
{
	Mat imgHSV, img_Filter,detection;
	int n = 5;
	long histogramH[361];
	long histogramV[361];
	int sigma = (n - 1) / 6;
	GaussianBlur(img, img_Filter, Size(n, n), sigma, 0);
	cvtColor(img_Filter, imgHSV, COLOR_RGB2HSV);
	int posX = img.rows;
	int posY = img.cols;

	Mat masque(img.rows, img.cols, CV_8UC2);

	validatePixel(imgHSV, &masque, min_intensity, min_staturation);

	fonction_trapeze(imgHSV,d, h, alpha);

	resetHistogram(histogramH);
	resetHistogram(histogramV);

	calHistogram(imgHSV, d, h, alpha, histogramH, histogramV, masque);

	filterAverage(histogramH, 1, 360);
	filterAverage(histogramV, 1, 350);

	classifyHV(histogramH, histogramV, hue_thresholds, intensity_thresholds);

	detection = detectObstacle(imgHSV, histogramH, histogramV);

	return detection;
}


