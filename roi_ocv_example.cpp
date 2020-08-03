//
// Created by biba_bo on 2020-08-03.
//

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
    cv::Mat loaded_image = cv::imread("o-96.jpg", cv::IMREAD_COLOR);
    cv::namedWindow("Step 1 image loaded", cv::WINDOW_AUTOSIZE);
    cv::imshow("Step 1 image loaded", loaded_image);
    cv::waitKey(3000);

    cv::Rect my_rect(200, 200, 600, 400);
    cv::rectangle(loaded_image, my_rect, cv::Scalar(255), 1, 8, 0);
    cv::namedWindow("Step 2 draw Rectangle", cv::WINDOW_AUTOSIZE);
    cv::imshow("Step 2 draw Rectangle", loaded_image);
    cv::waitKey(3000);

    cv::Mat roi_mat = loaded_image(my_rect);
    cv::namedWindow("Step 3 Draw selected ROI", cv::WINDOW_AUTOSIZE);
    cv::imshow("Step 3 Draw selected ROI", roi_mat);
    cv::waitKey(3000);

    cv::Rect where_rect(0, 0, roi_mat.cols, roi_mat.rows);
    roi_mat.copyTo(loaded_image(where_rect));
    cv::namedWindow("Step 4 Final result", cv::WINDOW_AUTOSIZE);
    cv::imshow("Step 4 Final result", loaded_image);
    cv::waitKey(0);
    return 0;
}