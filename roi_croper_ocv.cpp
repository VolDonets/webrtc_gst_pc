//
// Created by biba_bo on 2020-08-03.
//

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

int main(int argc, char** argv) {
    cv::Mat img = cv::imread("o-96.jpg");
    cv::Rect2d r = cv::selectROI(img);
    cv::Mat img_crop = img(r);

    cv::imshow("Image", img_crop);
    cv::waitKey(0);
    return 0;
}