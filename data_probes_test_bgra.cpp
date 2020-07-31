//
// Created by biba_bo on 2020-07-31.
//

#include <gst/gst.h>
#include <opencv2/opencv.hpp>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


#define WIDTH 800 //384
#define HEIGHT 600 //288

static GstPadProbeReturn cb_have_data (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    gint x, y;
    GstMapInfo map;
    guint16 *ptr, t;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER (info);

    buffer = gst_buffer_make_writable (buffer);

    /* Making a buffer writable can fail (for example if it
     * cannot be copied and is used more than once)
     */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {
        //*
        cv::Mat frame_image;
        cv::Size frame_size(WIDTH, HEIGHT);

        frame_image = cv::Mat(frame_size, CV_8UC4, (char*)(map.data), cv::Mat::AUTO_STEP);
        int cv_x = 300;
        int cv_y = 200;
        int cv_width = 200;
        int cv_height = 200;

        cv::Point cv_pt1(cv_x, cv_y);
        cv::Point cv_pt2(cv_x + cv_width, cv_y + cv_height);


        cv::Scalar my_color(0, 255, 0, 255);
        cv::rectangle(frame_image, cv_pt1, cv_pt2, my_color);

        cv::Scalar circle_color(0, 0, 255);
        cv::Point pt_center(300, 200);
        cv::circle(frame_image, pt_center, 150, circle_color);

        gst_buffer_unmap (buffer, &map);
    }

    GST_PAD_PROBE_INFO_DATA (info) = buffer;

    return GST_PAD_PROBE_OK;
}

gint main (gint argc, gchar *argv[]) {
    GMainLoop *loop;
    GstElement *pipeline, *src, *sink, *filter, *csp, *csp_bgra, *filter_bgra;
    GstCaps *filtercaps, *filtercapt_bgra;
    GstPad *pad;

    /* init GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* build */
    pipeline = gst_pipeline_new ("my-pipeline");
    src = gst_element_factory_make ("v4l2src", "src");
    if (src == NULL)
        g_error ("Could not create 'v4l2src' element");

    filter = gst_element_factory_make ("capsfilter", "filter");
    g_assert (filter != NULL); /* should always exist */

    csp_bgra = gst_element_factory_make("videoconvert", "csp_bgra");
    if (csp_bgra == NULL)
        g_error("Could not create 'videoconvert' element");

    filter_bgra = gst_element_factory_make("capsfilter", "filter_bgra");
    g_assert (filter_bgra != NULL); /* should always exist */

    csp = gst_element_factory_make ("videoconvert", "csp");
    if (csp == NULL)
        g_error ("Could not create 'videoconvert' element");

    sink = gst_element_factory_make ("xvimagesink", "sink");
    if (sink == NULL) {
        sink = gst_element_factory_make ("ximagesink", "sink");
        if (sink == NULL)
            g_error ("Could not create neither 'xvimagesink' nor 'ximagesink' element");
    }

    gst_bin_add_many (GST_BIN (pipeline), src, filter, csp_bgra, filter_bgra, csp, sink, NULL);
    gst_element_link_many (src, filter, csp_bgra, filter_bgra, csp, sink, NULL);
    g_object_set(src, "device", "/dev/video0", NULL);
    filtercaps = gst_caps_new_simple ("video/x-raw",
                                      "width", G_TYPE_INT, WIDTH,
                                      "height", G_TYPE_INT, HEIGHT,
                                      "framerate", GST_TYPE_FRACTION, 15, 1,
                                      NULL);
    g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
    gst_caps_unref (filtercaps);

    filtercapt_bgra = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRA", NULL);
    g_object_set(G_OBJECT(filter_bgra), "caps", filtercapt_bgra, NULL);
    gst_caps_unref(filtercapt_bgra);

    pad = gst_element_get_static_pad (csp_bgra, "src");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
                       (GstPadProbeCallback) cb_have_data, NULL, NULL);
    gst_object_unref (pad);

    /* run */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* wait until it's up and running or failed */
    if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
        g_error ("Failed to go into PLAYING state");
    }

    g_print ("Running ...\n");
    g_main_loop_run (loop);

    /* exit */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}

