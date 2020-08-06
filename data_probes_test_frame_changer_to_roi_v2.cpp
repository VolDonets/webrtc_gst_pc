//
// Created by biba_bo on 2020-08-04.
//

#include <gst/gst.h>
#include <opencv2/opencv.hpp>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"


#define WIDTH 800
#define HEIGHT 600

#define ROI_WIDTH 360
#define ROI_HEIGHT 240
#define ROI_X_GET 10
#define ROI_Y_GET 10

static cv::Mat cropped_img;

static GstPadProbeReturn cb_have_data (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    //GstCaps *my_pad_caps = gst_pad_get_current_caps(pad);
    //std::cout << "my pad caps: " << gst_caps_to_string(my_pad_caps) << "\n\n\n";

    GstCaps *my_new_caps;
    GstMapInfo map;
    GstBuffer *buffer, *new_buffer;
    uint64 new_buffer_size = 0;

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

        cv::Rect my_interest_region(ROI_X_GET, ROI_Y_GET, ROI_WIDTH, ROI_HEIGHT);

        cv::Mat cropped_img_0 = frame_image(my_interest_region);
        cropped_img = cropped_img_0.clone();


        gst_buffer_unmap (buffer, &map);

        new_buffer = gst_buffer_new();
        new_buffer_size = cropped_img.total() * cropped_img.elemSize();
        GstMemory* memory = gst_memory_new_wrapped(static_cast<GstMemoryFlags>(NULL), std::move(cropped_img.data), new_buffer_size, 0, new_buffer_size, NULL, NULL);
        cropped_img.data = nullptr;
        gst_buffer_insert_memory(new_buffer, -1, memory);

        //cv::namedWindow("aaaa", cv::WINDOW_AUTOSIZE);
        //cv::imshow("aaaa", cropped_img);
    }

    GST_PAD_PROBE_INFO_DATA(info) = new_buffer;
    GST_PAD_PROBE_INFO_SIZE(info) = new_buffer_size;
    my_new_caps = gst_caps_new_simple ("video/x-raw",
                                       "width", G_TYPE_INT, ROI_WIDTH,
                                       "height", G_TYPE_INT, ROI_HEIGHT,
                                       "framerate", GST_TYPE_FRACTION, 15, 1,
                                       "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                       "interlace-mode", G_TYPE_STRING, "progressive",
                                       "format", G_TYPE_STRING, "BGRA",
                                       NULL);
    gst_pad_set_caps(pad, my_new_caps);

    /*if (gst_buffer_map (new_buffer, &map, GST_MAP_READ)) {
        //*
        cv::Mat frame_image;
        cv::Size frame_size(ROI_WIDTH, ROI_HEIGHT);

        frame_image = cv::Mat(frame_size, CV_8UC4, (char*)(map.data), cv::Mat::AUTO_STEP);

        gst_buffer_unmap (new_buffer, &map);

        cv::namedWindow("bbbb", cv::WINDOW_AUTOSIZE);
        cv::imshow("bbbb", frame_image);
    }*/

    return GST_PAD_PROBE_OK;
}

static GstPadProbeReturn vsink_have_data (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstCaps *my_pad_caps = gst_pad_get_current_caps(pad);
    std::cout << "my SINK pad caps: " << gst_caps_to_string(my_pad_caps) << "\n\n\n";

    GstMapInfo map;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER (info);

    //buffer = gst_buffer_make_writable (buffer);

    /* Making a buffer writable can fail (for example if it
     * cannot be copied and is used more than once)
     */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    if (gst_buffer_map (buffer, &map, GST_MAP_READ)) {
        //*
        cv::Mat frame_image;
        cv::Size frame_size(ROI_WIDTH, ROI_HEIGHT);

        frame_image = cv::Mat(frame_size, CV_8UC4, (char*)(map.data), cv::Mat::AUTO_STEP);

        gst_buffer_unmap (buffer, &map);

        //cv::namedWindow("cccc", cv::WINDOW_AUTOSIZE);
        //cv::imshow("cccc", frame_image);

    }

    //GST_PAD_PROBE_INFO_DATA(info) = buffer;

    return GST_PAD_PROBE_OK;
}

static GstPadProbeReturn vsrc_have_data (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstCaps *my_pad_caps = gst_pad_get_current_caps(pad);
    std::cout << "my SRC pad caps: " << gst_caps_to_string(my_pad_caps) << "\n\n\n";

    GstMapInfo map;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER (info);

    //buffer = gst_buffer_make_writable (buffer);

    /* Making a buffer writable can fail (for example if it
     * cannot be copied and is used more than once)
     */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    if (gst_buffer_map (buffer, &map, GST_MAP_READ)) {
        //*
        cv::Mat frame_image;
        cv::Size frame_size(ROI_WIDTH, ROI_HEIGHT);

        frame_image = cv::Mat(frame_size, CV_8UC4, (char*)(map.data), cv::Mat::AUTO_STEP);

        gst_buffer_unmap (buffer, &map);

        //cv::namedWindow("dddd", cv::WINDOW_AUTOSIZE);
        //cv::imshow("dddd", frame_image);
    }

    //GST_PAD_PROBE_INFO_DATA(info) = buffer;

    return GST_PAD_PROBE_OK;
}

gint main (gint argc, gchar *argv[]) {
    GMainLoop *loop;
    GstElement *pipeline, *src, *sink, *filter, *csp, *csp_bgra, *filter_bgra;//, *filter_queue, *queue;
    GstCaps *filtercaps, *filtercaps_bgra;//, *filtercaps_queue;
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

    /*queue = gst_element_factory_make("queue", "queue");
    if (queue == NULL)
        g_error("Could NOT create 'queue' element");

    filter_queue = gst_element_factory_make("capsfilter", "filter_queue");
    g_assert(filter_queue != NULL); /* should always exist */

    sink = gst_element_factory_make ("autovideosink", "sink");
    if (sink == NULL) {
        sink = gst_element_factory_make ("ximagesink", "sink");
        if (sink == NULL)
            g_error ("Could not create neither 'xvimagesink' nor 'ximagesink' element");
    }

    gst_bin_add_many (GST_BIN (pipeline), src, filter, csp_bgra, filter_bgra,/* queue, filter_queue,*/ csp, sink, NULL);
    gst_element_link_many (src, filter, csp_bgra, filter_bgra,/* queue, filter_queue,*/ csp, sink, NULL);
    g_object_set(src, "device", "/dev/video0", NULL);
    filtercaps = gst_caps_new_simple ("video/x-raw",
                                      "width", G_TYPE_INT, WIDTH,
                                      "height", G_TYPE_INT, HEIGHT,
                                      "framerate", GST_TYPE_FRACTION, 15, 1,
                                      NULL);
    g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
    gst_caps_unref (filtercaps);

    filtercaps_bgra = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRA", NULL);
    g_object_set(G_OBJECT(filter_bgra), "caps", filtercaps_bgra, NULL);
    gst_caps_unref(filtercaps_bgra);

    /*filtercaps_queue = gst_caps_new_simple("video/x-raw",
                                         "format", G_TYPE_STRING, "BGRA",
                                         "width", G_TYPE_INT, ROI_WIDTH,
                                         "height", G_TYPE_INT, ROI_HEIGHT,
                                         NULL);
    g_object_set(G_OBJECT(filter_queue), "caps", filtercaps_queue, NULL);
    gst_caps_unref(filtercaps_queue);*/

    pad = gst_element_get_static_pad (csp_bgra, "src");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
                       (GstPadProbeCallback) cb_have_data, NULL, NULL);
    GstPadTemplate *my_template = gst_pad_get_pad_template(pad);
    std::cout << "\n=====\npad template caps: " << gst_caps_to_string(my_template->caps) << "\n++++++++\n";
    gst_object_unref (pad);




    pad = gst_element_get_static_pad(csp, "src");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) vsrc_have_data, NULL, NULL);
    gst_object_unref(pad);

    pad = gst_element_get_static_pad(csp, "sink");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) vsink_have_data, NULL, NULL);
    gst_object_unref(pad);
    // */

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
