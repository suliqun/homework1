//之前上传的代码取消，这是新的


#include<stdio.h>
#include<iostream>
#include"SubImageMatch.h"
#include "opencv2/imgproc.hpp"
#include<math.h>

//#define IMG_SHOW
//#define IMG_SHOW_MH

#define Pi 3.14159;
#define Pih 1.57;
using namespace cv;
using namespace std;

char bin[2] = { 255,0 };
char sign[2] = { -1,1 };
int bi[2] = { 360,0 };

static const uint16_t atanlist[256] = {
	0, 229, 458, 687, 916, 1145, 1374, 1603,
	1832, 2061, 2290, 2519, 2748, 2976, 3205, 3433,
	3662, 3890, 4118, 4346, 4574, 4802, 5029, 5257,
	5484, 5711, 5938, 6165, 6391, 6618, 6844, 7070,
	7296, 7521, 7746, 7971, 8196, 8421, 8645, 8869,
	9093, 9317, 9540, 9763, 9986, 10208, 10431, 10652,
	10874, 11095, 11316, 11537, 11757, 11977, 12197, 12416,
	12635, 12853, 13071, 13289, 13507, 13724, 13940, 14157,
	14373, 14588, 14803, 15018, 15232, 15446, 15660, 15873,
	16085, 16297, 16509, 16720, 16931, 17142, 17352, 17561,
	17770, 17979, 18187, 18394, 18601, 18808, 19014, 19220,
	19425, 19630, 19834, 20038, 20241, 20444, 20646, 20848,
	21049, 21250, 21450, 21649, 21848, 22047, 22245, 22443,
	22640, 22836, 23032, 23227, 23422, 23616, 23810, 24003,
	24196, 24388, 24580, 24771, 24961, 25151, 25340, 25529,
	25717, 25905, 26092, 26278, 26464, 26649, 26834, 27018,
	27202, 27385, 27568, 27750, 27931, 28112, 28292, 28471,
	28650, 28829, 29007, 29184, 29361, 29537, 29712, 29887,
	30062, 30236, 30409, 30582, 30754, 30925, 31096, 31266,
	31436, 31605, 31774, 31942, 32109, 32276, 32442, 32608,
	32773, 32938, 33101, 33265, 33428, 33590, 33751, 33913,
	34073, 34233, 34392, 34551, 34709, 34867, 35024, 35180,
	35336, 35492, 35646, 35801, 35954, 36107, 36260, 36412,
	36563, 36714, 36864, 37014, 37163, 37312, 37460, 37607,
	37754, 37901, 38047, 38192, 38337, 38481, 38624, 38768,
	38910, 39052, 39194, 39335, 39475, 39615, 39754, 39893,
	40032, 40169, 40307, 40443, 40580, 40715, 40850, 40985,
	41119, 41253, 41386, 41519, 41651, 41782, 41913, 42044,
	42174, 42303, 42432, 42561, 42689, 42817, 42944, 43070,
	43196, 43322, 43447, 43572, 43696, 43819, 43943, 44065,
	44188, 44309, 44431, 44551, 44672, 44792, 44911, 45030,
	45148, 45266, 45384, 45501, 45618, 45734, 45849, 45965,
};



//牛顿迭代法开方
float SqrtByNewton(float x)
{
	float val = x;//初始值
	float last;
	float eps = x / 1000;
	do
	{
		last = val;
		val = (val + x / val) / 2;
		eps = val / 100;
	} while (fabsf(val - last) > eps);
	return val;
}

//Taylor展开求反三角
float myatan2(float y, float x) {
	if (!x) {
		if (y > 0) return Pih
		else if (y < 0) return -Pih
		else return 0;
	}
	float z = y / x, sum = 0.0f, tem;
	float del = z / 100;
	for (int i = 0; i < 100; i++) {
		z = i*del;
		tem = 1 / (z*z + 1)*del;
		sum += tem;
	}
	if (x > 0) return sum;
	else if (y >= 0) return sum + Pi
	else if (y < 0) return sum - Pi;

	}

//查表法求反三角
float calc_phase(int y, int x)
{
	int ang;
	float index;
	int absy, absx;
	if (!x) {
		if (y > 0) return 90;
		else if (y < 0) return -90;
		else return 0;
	}
	absy = abs(y);
	absx = abs(x);
	if (absy < absx) {
		index = (absy << 8) / absx;
		ang = atanlist[(char)index];
	}
	else {
		index = absy*1.0 / absx;
		index = ((index - 1) / (index + 1)) * 256;
		ang = atanlist[(char)index];
		ang += 46080;
	}
	if (x > 0)
		if (y > 0) return ang*1.0/1024;
		else return -ang*1.0 /1024;
	else if (x < 0)
		if (y >= 0) return 180-ang*1.0 / 1024;
	if (y < 0) return ang*1.0 / 1024 - 180 ;
}
	  


//func1
int ustc_ConvertBgr2Gray(Mat bgrImg, Mat& grayImg) {
	if ((!bgrImg.data) || (!grayImg.data)) {
		cout << "fail";
		return  SUB_IMAGE_MATCH_FAIL;
	}
	if (bgrImg.channels() != 3 || grayImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (bgrImg.rows!= grayImg.rows || bgrImg.cols != grayImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int rowmax = bgrImg.rows;
	int	colmax = bgrImg.cols;
	int max = rowmax*colmax;
	uchar *pbgr=bgrImg.data, 
		  *pgray= grayImg.data;
	for (int i = 0; i < max; i++) {
		int b = pbgr[3 * i];
		int g = pbgr[3 * i + 1];
		int r = pbgr[3 * i + 2];

		int gray_temp = b * 117 + g * 601 + r * 306;
		pgray[i] = gray_temp>>10;
	}

#ifdef IMG_SHOW
	namedWindow("colorImg", 0);
	imshow("colorImg", bgrImg);
	waitKey();
#endif
	return SUB_IMAGE_MATCH_OK;
}

//func2
int ustc_CalcGrad(Mat grayImg, Mat& gradImg_x, Mat& gradImg_y) {
	if ((!grayImg.data) || (!gradImg_x.data) || (!gradImg_y.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || gradImg_x.channels() != 1|| gradImg_y.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}

	int nrow = grayImg.rows;
	int ncol = grayImg.cols;
	int max = ncol*(nrow-1);

	if (gradImg_x.rows != nrow || gradImg_x.cols != ncol|| gradImg_y.rows != nrow|| gradImg_y.cols != ncol) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	uchar *pgray = grayImg.data,
		*px = gradImg_x.data,
		*py = gradImg_y.data;


	
	//////////////////////////////////////////////////////////////////边界赋值
	for (int i = 0; i < ncol; i++) {
		((float*)px)[i] = 0;
		((float*)py)[i] = 0;
		((float*)px)[i + max] = 0;
		((float*)py)[i + max] = 0;
	}
	for (int j = 0; j < nrow; j++) {
		((float*)px)[j*ncol] = 0;
		((float*)py)[j*ncol] = 0;
		((float*)px)[(j+1)*ncol-1] = 0;
		((float*)py)[(j+1)*ncol-1] = 0;
	}
	//////////////////////////////////////////////////////////////////////
	
	for (int i = 1; i < nrow; i++) {
		int Nrow = ncol*i;
		int Nrow_u = Nrow - ncol;
		int Nrow_d = Nrow + ncol;
		for (int j = 1; j < ncol; j++) {
			int x = pgray[Nrow_u + j + 1] - pgray[Nrow_u + j - 1]; 
			int y = pgray[Nrow_u + j + 1] + pgray[Nrow_u + j - 1] + 2 * pgray[Nrow_u + j]; 
			x += (2 * pgray[Nrow + j + 1] - 2 * pgray[Nrow + j - 1]);
			x += (pgray[Nrow_d + j + 1]-pgray[Nrow_d + j - 1]);
			y -= (2 * pgray[Nrow_d + j] + pgray[Nrow_d + j + 1] + pgray[Nrow_d + j - 1]);
			((float*)px)[Nrow + j] = x;
			((float*)py)[Nrow + j] = y;
		}
	}

#ifdef IMG_SHOW
	Mat gradImg_x_8U(nrow, ncol, CV_8UC1);
	Mat gradImg_y_8U(nrow, ncol, CV_8UC1);
	for (int row_i = 0; row_i < nrow; row_i++)
	{
		for (int col_j = 0; col_j < ncol; col_j += 1)
		{
			int val = ((float*)gradImg_x.data)[row_i * ncol + col_j];
			gradImg_x_8U.data[row_i * ncol + col_j] = abs(val);
            val= ((float*)gradImg_y.data)[row_i * ncol + col_j];
			gradImg_y_8U.data[row_i * ncol + col_j] = abs(val);
		}
	}

	namedWindow("gradImg_x_8U", 0);
	namedWindow("gradImg_y_8U", 0);
	imshow("gradImg_x_8U", gradImg_x_8U);
	imshow("gradImg_y_8U", gradImg_y_8U);
	waitKey();

#endif


	return SUB_IMAGE_MATCH_OK;
}

//func3
int ustc_CalcAngleMag(Mat gradImg_x, Mat gradImg_y, Mat& angleImg, Mat& magImg) {
	if ((!angleImg.data) || (!gradImg_x.data) || (!gradImg_y.data)|| (!magImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (angleImg.channels() != 1 || gradImg_x.channels() != 1 || gradImg_y.channels() != 1|| magImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int nrow = gradImg_x.rows;
	int ncol = gradImg_x.cols;
	if (gradImg_y.rows != nrow || gradImg_y.cols != ncol || angleImg.rows != nrow || angleImg.cols != ncol|| 
		magImg.rows != nrow || magImg.cols != ncol) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}

	uchar* px = gradImg_x.data,
		*py = gradImg_y.data,
		*ang = angleImg.data,
		*mag = magImg.data;
	for (int i = 1; i < nrow - 1; i++) {
		int Nrow = ncol*i;
		for (int j = 1; j < ncol - 1; j++)
		{   
			int num = Nrow + j;
			float mag2;
			float gradx = ((float*)px)[num];
			float grady = ((float*)py)[num];
			//求幅值
			mag2 = gradx*gradx + grady*grady;
			((float*)mag)[num] = SqrtByNewton(mag2);
			((float*)ang)[num] = calc_phase(grady, gradx)+180;
			//((float*)mag)[num] = atan2(grady, gradx)*180.0/Pi+180;
		}
	}
	
#ifdef IMG_SHOW
	Mat angleImg_8U(nrow, ncol, CV_8UC1);
	Mat magImg_8U(nrow, ncol, CV_8UC1);
	for (int row_i = 0; row_i < nrow; row_i++)
	{
		for (int col_j = 0; col_j < ncol; col_j += 1)
		{
			float angle = ((float*)angleImg.data)[row_i * ncol + col_j];
			magImg_8U.data[row_i * ncol + col_j]  = ((float*)magImg.data)[row_i * ncol + col_j];
			//为了能在8U上显示，缩小到0-180之间
			angle /= 2;
			angleImg_8U.data[row_i * ncol + col_j] = angle;
		}
	}

	namedWindow("angleImg_8U", 0);
	namedWindow("magImg_8U", 0);
	imshow("angleImg_8U", angleImg_8U);
	imshow("magImg_8U", magImg_8U);
	//imwrite("ang.jpg", angleImg_8U);
	//imwrite("mag.jpg", magImg_8U);
	waitKey();
#endif
	
	return SUB_IMAGE_MATCH_OK;
}

//func4
int ustc_Threshold(Mat grayImg, Mat& binaryImg, int th) {
	if ((!grayImg.data) || (!binaryImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || binaryImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (binaryImg.rows != grayImg.rows || binaryImg.cols != grayImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int tem;
	int nrow = grayImg.rows;
	int ncol = grayImg.cols;
	uchar *gray = grayImg.data;
	for (int i = 0; i < nrow; i++) {
		int Nrow = ncol*i;
		for (int j = 0; j < ncol; j++) {
			tem = gray[Nrow + j] - th-1;
			tem = tem >> 31;
			binaryImg.data[Nrow + j] = bin[tem];
		}
	}

#ifdef IMG_SHOW
	namedWindow("binaryImg", 0);
	imshow("binaryImg", binaryImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func5
int ustc_CalcHist(Mat grayImg, int* hist, int hist_len) {
	if ((!grayImg.data) || (!hist)|| hist_len!=256) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 ) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}

	//直方图清零
	for (int i = 0; i < hist_len; i++)
	{
		hist[i] = 0;
	}

	int nrow = grayImg.rows;
	int ncol = grayImg.cols;
	
	for (int i = nrow-1; i >= 0; i--) {
		int Nrow = ncol*i;
		for (int j = ncol-1; j >=0; j--) {
			int mag= grayImg.data[Nrow + j];
			hist[mag]++;
		}
	}

#ifdef IMG_SHOW
	for (int i = 0; i < hist_len; i++)
		cout << hist[i] << endl;
	getchar();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func6
int ustc_SubImgMatch_gray(Mat grayImg, Mat subImg, int* x, int* y) {
	if ((!grayImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() !=1 || subImg.channels() !=1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.rows < subImg.rows || grayImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = grayImg.cols;
	int nrow = grayImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;
	
	//该图用于记录每一个像素位置的匹配误差
	Mat searchImg(nrow,ncol, CV_32FC1);
	searchImg.setTo(FLT_MAX);

	int imax = nrow - sub_nrow;
	int jmax = ncol - sub_ncol;
	for (int i = 0; i <imax; i++)
	{
		for (int j = 0; j < jmax; j++)
		{
			int total_diff = 0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < sub_nrow;y++)
			{
				int sub_rowindex = y*sub_ncol;
				int row_index = (i + y)* ncol;
				for (int x = 0;x < sub_ncol; x++)
				{
					//大图上的像素位置
					
					int col_index = j + x;
					int bigImg_pix = grayImg.data[row_index + col_index];
					//模板图上的像素
					int template_pix = subImg.data[sub_rowindex +x];
					int dif = abs(bigImg_pix - template_pix);
					total_diff += dif;
				}
			}
			//存储当前像素位置的匹配误差
			((float*)searchImg.data)[i * ncol + j] = total_diff;
		}
	}
	
	float mindiff = FLT_MAX,diff;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			diff = ((float*)searchImg.data)[Nrow + j];
			if (diff < mindiff) {
				*x = j, *y = i;
				mindiff = diff;
			}
		}
	}

#ifdef IMG_SHOW
	rectangle(grayImg, Rect(*x,*y, sub_ncol, sub_nrow), Scalar(0,0, 255), 10, 10, 0);
	namedWindow("grayImg", 0);
	imshow("grayImg", grayImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func7
int ustc_SubImgMatch_bgr(Mat colorImg, Mat subImg, int* x, int* y) {
	if ((!colorImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (colorImg.channels() != 3 || subImg.channels() != 3) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (colorImg.rows < subImg.rows || colorImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = colorImg.cols;
	int nrow = colorImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;

	Mat searchImg(nrow, ncol, CV_32FC1);
	searchImg.setTo(FLT_MAX);
	
	int imax= nrow - sub_nrow;
	int jmax= ncol - sub_ncol;
	for (int i = 0; i < imax; i++)
	{
		for (int j = 0; j < jmax; j++)
		{
			int total_diff = 0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < sub_nrow; y++)
			{
				int sub_rowindex = y*sub_ncol*3;
				int row_index = (i + y)* ncol*3;
				for (int x = 0; x < sub_ncol; x++)
				{
					//大图上的像素位置
					int col_index = (j + x)*3;
					int xindex = x * 3;
					int bigImg_pix_b = colorImg.data[row_index  + col_index];
					int bigImg_pix_g = colorImg.data[row_index  + col_index+1];
					int bigImg_pix_r = colorImg.data[row_index  + col_index+2];
					//模板图上的像素
					int template_pix_b = subImg.data[sub_rowindex  + xindex];
					int template_pix_g = subImg.data[sub_rowindex  + xindex +1];
					int template_pix_r = subImg.data[sub_rowindex  + xindex +2];

					int bdf = abs(bigImg_pix_b - template_pix_b);
					int gdf = abs(bigImg_pix_g - template_pix_g);
					int rdf = abs(bigImg_pix_r - template_pix_r);

		

					total_diff +=( bdf+rdf+gdf);
				}
			}
			//存储当前像素位置的匹配误差
			((float*)searchImg.data)[i * ncol + j] = total_diff;
		}
	}
	float mindiff = FLT_MAX, diff;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			diff = ((float*)searchImg.data)[Nrow + j];
			if (diff < mindiff) {
				*x = j, *y = i;
				mindiff = diff;
			}
		}
	}

#ifdef IMG_SHOW
	rectangle(colorImg, Rect(*x, *y, sub_ncol, sub_nrow), Scalar(0, 0, 255), 10, 10, 0);
	namedWindow("colorImg", 0);
	imshow("colorImg", colorImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func8
int ustc_SubImgMatch_corr(Mat grayImg, Mat subImg, int* x, int* y) {
	if ((!grayImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.rows < subImg.rows || grayImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = grayImg.cols;
	int nrow = grayImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;

	//该图用于记录每一个像素位置的匹配误差
	Mat searchImg(nrow, ncol, CV_32FC1);
	searchImg.setTo(0);

	int imax = nrow - sub_nrow;
	int jmax= ncol - sub_ncol;
	for (int i = 0; i < imax; i++)
	{
		for (int j = 0; j < jmax; j++)
		{
			float corr = 0;
			int up = 0;
			int downgray = 0;
			int downsub = 0;
			int down=0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < sub_nrow; y++)
			{
				int sub_rowindex = y*sub_ncol;
				int row_index = (i + y)* ncol;
				for (int x = 0; x < sub_ncol; x++)
				{

					int col_index = j + x;
					int bigImg_pix = grayImg.data[row_index + col_index];
					int template_pix = subImg.data[sub_rowindex + x];
					up += bigImg_pix*template_pix;
					downgray += bigImg_pix*bigImg_pix;
					downsub += template_pix*template_pix;
				}
			}
			
			downgray = SqrtByNewton(downgray);
			downsub = SqrtByNewton(downsub);
			down = downgray*downsub;
			corr = up*1.0 / down;
			((float*)searchImg.data)[i*ncol + j] = corr;
		}
	}

	float maxcorr = 0, corr;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			corr = ((float*)searchImg.data)[Nrow + j];
			if (corr > maxcorr) {
				*x = j, *y = i;
				maxcorr = corr;
			}
		}
	}
	cout << maxcorr;
#ifdef IMG_SHOW
	rectangle(grayImg, Rect(*x, *y, sub_ncol, sub_nrow), Scalar(0, 0, 255), 10, 10, 0);
	namedWindow("grayImg", 0);
	imshow("grayImg", grayImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func9
int ustc_SubImgMatch_angle(Mat grayImg, Mat subImg, int* x, int* y) {
	if ((!grayImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.rows < subImg.rows || grayImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = grayImg.cols;
	int nrow = grayImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;

	Mat searchImg(nrow, ncol, CV_32FC1);
	searchImg.setTo(FLT_MAX);

	int imax= nrow - sub_nrow - 1;
	int jmax = ncol - sub_ncol - 1;
	int ymax = sub_nrow ;
	int xmax = sub_ncol;
	char sig;
	for (int i = 1; i < imax; i++)
	{
		for (int j = 1; j <jmax; j++)
		{
			int total_diff = 0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < ymax; y++)
			{
				int sub_rowindex = y*sub_ncol;
				int row_index = (i + y)* ncol;
				for (int x =0; x<xmax; x++)
				{
					//大图上的像素位置

					int col_index = j + x;
					int bigImg_pix = ((float*)grayImg.data)[row_index + col_index];
					//模板图上的像素
					int template_pix = ((float*)subImg.data)[sub_rowindex + x];
					int dif = bigImg_pix - template_pix;
					if (dif < 0) dif = -dif;
					if (dif > 180) dif = 360 - dif;
					//sig = (dif - 180) >> 31;
					//dif = sign[sig] * dif;
					//dif += bi[sig];
					total_diff += dif;
					
				}
			}
			//存储当前像素位置的匹配误差
			((float*)searchImg.data)[i * ncol + j] = total_diff;
		}
	}

	float mindiff = FLT_MAX, diff;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			diff = ((float*)searchImg.data)[Nrow + j];
			if (diff < mindiff) {
				*x = j, *y = i;
				mindiff = diff;
			}
		}
	}

#ifdef IMG_SHOW
	rectangle(grayImg, Rect(*x, *y, sub_ncol, sub_nrow), Scalar(0, 0, 255), 10, 10, 0);
	namedWindow("grayImg", 0);
	imshow("grayImg", grayImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}

//func10
int ustc_SubImgMatch_mag(Mat grayImg, Mat subImg, int* x, int* y) {
	if ((!grayImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.rows < subImg.rows || grayImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = grayImg.cols;
	int nrow = grayImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;

	Mat searchImg(nrow, ncol, CV_32FC1);
	searchImg.setTo(FLT_MAX);

	int imax = nrow - sub_nrow ;
	int jmax = ncol - sub_ncol ;
	int ymax = sub_nrow;
	int xmax = sub_ncol;
	for (int i = 0; i < imax; i++)
	{
		for (int j = 0; j <jmax; j++)
		{
			int total_diff = 0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < ymax; y++)
			{
				int sub_rowindex = y*sub_ncol;
				int row_index = (i + y)* ncol;
				for (int x = 0; x <xmax; x++)
				{
					//大图上的像素位置

					int col_index = j + x;
					int bigImg_pix = ((float*)grayImg.data)[row_index + col_index];
					//模板图上的像素
					int template_pix = ((float*)subImg.data)[sub_rowindex + x];
					int dif = abs(bigImg_pix - template_pix);
					total_diff += dif;

				}
			}
			//存储当前像素位置的匹配误差
			((float*)searchImg.data)[i * ncol + j] = total_diff;
		}
	}

	float mindiff = FLT_MAX, diff;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			diff = ((float*)searchImg.data)[Nrow + j];
			if (diff < mindiff) {
				*x = j, *y = i;
				mindiff = diff;
			}
		}
	}

#ifdef IMG_SHOW
	rectangle(grayImg, Rect(*x, *y, sub_ncol, sub_nrow), Scalar(0, 0, 255), 10, 10, 0);
	namedWindow("grayImg", 0);
	imshow("grayImg", grayImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;

}

//func11
int ustc_SubImgMatch_hist(Mat grayImg, Mat subImg, int* x, int* y) {
	if ((!grayImg.data) || (!subImg.data)) {
		cout << "fail";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1) {
		cout << "channel error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	if (grayImg.rows < subImg.rows || grayImg.cols < subImg.cols) {
		cout << "size error";
		return SUB_IMAGE_MATCH_FAIL;
	}
	int ncol = grayImg.cols;
	int nrow = grayImg.rows;
	int sub_ncol = subImg.cols;
	int sub_nrow = subImg.rows;
	int histg[256];
	int hists[256];
	ustc_CalcHist( subImg,  hists, 256);
	Mat tem(subImg.rows, subImg.cols, CV_8UC1);

	Mat searchImg(nrow, ncol, CV_32FC1);
	searchImg.setTo(FLT_MAX);

	int imax = nrow - sub_nrow;
	int jmax = ncol - sub_ncol;
	int ymax = sub_nrow;
	int xmax = sub_ncol;
	for (int i = 0; i < imax; i++)
	{
		for (int j = 0; j < jmax; j++)
		{
			int total_diff = 0;
			//遍历模板图上的每一个像素
			for (int y = 0; y < ymax; y++)
			{
				int sub_rowindex = y*sub_ncol;
				int row_index = (i + y)* ncol;
				for (int x = 0; x < xmax; x++)
				{
					int col_index = j + x;
					tem.data[sub_rowindex + x] = grayImg.data[row_index + col_index];
				}
			}
			ustc_CalcHist(tem, histg, 256);
			for (int t = 0; t < 256; t++) {
				total_diff += abs(hists[t] - histg[t]);
			}
			((float*)searchImg.data)[i * ncol + j] = total_diff;
		}
	}

	float mindiff = FLT_MAX, diff;
	//寻找匹配点
	for (int i = 0; i < nrow - sub_nrow; i++)
	{
		int Nrow = ncol*i;
		for (int j = 0; j < ncol - sub_ncol; j++)
		{
			diff = ((float*)searchImg.data)[Nrow + j];
			if (diff < mindiff) {
				*x = j, *y = i;
				mindiff = diff;
			}
		}
	}

#ifdef IMG_SHOW_MH
	rectangle(grayImg, Rect(*x, *y, sub_ncol, sub_nrow), Scalar(0, 0, 255), 10, 10, 0);
	namedWindow("grayImg", 0);
	imshow("grayImg", grayImg);
	waitKey();
#endif

	return SUB_IMAGE_MATCH_OK;
}


