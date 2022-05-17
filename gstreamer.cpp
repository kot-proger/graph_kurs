#include "gstreamer.h"
#include <gst/gst.h>
#include <QString>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <qthread.h>

struct CustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *convert;
    GstElement *resample;
    GstElement *tee;
    GstElement *audio_queue;
    GstElement *wavenc;
    GstElement *sink;
    GstElement *wave_queue;
    GstElement *visual;
    GstElement *wave_convert;
    GstElement *wave_sink;
};

GStreamer::GStreamer(int argc, char *argv[])
{
    gst_init(&argc, &argv);
}

GStreamer::GStreamer(){

}

Video GStreamer::Analyse(QString filePath)
{
    QString tmp = filePath;
    for (int i = 0; i < filePath.length() - 1; i++)
    {
        if (filePath.at(i) == ' ' || filePath.at(i) == ')' || (filePath.at(i) == '(' && filePath.at(i-1) == ' '))
        {
            filePath.insert(i, (char)92);
            i++;
        }
    }
    QString command("gst-discoverer-1.0 " + filePath + " > analyse.txt");
    system(qPrintable(command));

    std::ifstream fin ("analyse.txt", std::fstream::in);
    std::string temp;
    int Width = 240;
    int Heigth = 320;
    int FPS = 25;

    while (temp != "Interlaced:")
    {
        fin>>temp;

        if (temp == "Width:")
        {
            fin>>temp;
            Width = std::stoi(temp);
        } else if (temp == "Height:")
        {
            fin>>temp;
            Heigth = std::stoi(temp);
        } else if (temp == "Frame")
        {
            fin>>temp;
            fin>>temp;
            std::string tmp;
            for(int i = 0; i < temp.length(); i++)
            {
                if (temp[i] == '/') break;
                tmp += temp[i];
            }
            FPS = std::stoi(temp);
        }

    }
    fin.close();

    QString aspect = "1:1";
    double asp = Heigth / Width;
    if (asp == 16 / 9)
    {
        aspect = "16:9";
    } else if (asp == 4 / 3)
    {
        aspect = "4:3";
    } else if (asp == 5 / 4)
    {
        aspect = "5:4";
    } else if (asp == 3 / 2)
    {
        aspect = "3:2";
    } else if (asp == 16 / 10)
    {
        aspect = "16:10";
    } else if (asp == 4 / 1)
    {
        aspect = "4:1";
    }

    return Video(filePath, FPS, Heigth, Width, aspect);
}

static void pad_data_handler(GstElement *src, GstPad *pad, CustomData *data);

Video GStreamer::Process(Video video, int FPS, int Heigth, int Width, QString aspectRatio)
{
    CustomData data{};
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    gboolean terminate;
    GstPad *tee_audio_pad;
    GstPad *tee_wave_pad;
    GstPad *queue_audio_pad;
    GstPad *queue_wave_pad;

    /*Create the elements*/
    data.source = gst_element_factory_make("uridecodebin", "source");
    data.tee = gst_element_factory_make("tee", "tee");
    data.audio_queue = gst_element_factory_make("queue", "audio_queue");
    data.convert = gst_element_factory_make("audiocinvert", "convert");
    data.resample = gst_element_factory_make("audioresample", "resample");
    data.wavenc = gst_element_factory_make("wavenc", "wavenc");
    data.sink = gst_element_factory_make("filesinc", "sink");
    data.wave_queue = gst_element_factory_make("queue", "wave_queue");
    data.visual = gst_element_factory_make("wavescope", "visual");
    data.wave_convert = gst_element_factory_make("videoconvert", "csp");
    data.wave_sink = gst_element_factory_make("autovideosink", "wave_sink");

    /*Create new empty pipline*/
    data.pipeline = gst_pipeline_new("gstreamer-pipline");

    if(data.pipeline || !data.source || !data.tee || !data.audio_queue || !data.convert || !data.resample || !data.wavenc ||
            !data.sink || !data.wave_queue || !data.visual ||!data.wave_convert || !data.wave_sink) {
        g_printerr("Not elements could be created\n");
    }

    /*Build the pipline. Note that we are NOT linking source at this point. We will do it later.*/
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.tee, data.audio_queue, data.convert, data.resample,
                     data.wavenc, data.sink, data.wave_queue,data.visual, data.wave_convert, data.wave_sink, NULL);
    if(!gst_element_link_many(data.convert, data.resample, data.tee, NULL) ||
            !gst_element_link_many(data.audio_queue, data.wavenc, data.sink, NULL) ||
            !gst_element_link_many(data.wave_queue, data.visual, data.wave_convert, data.wave_sink, NULL)) {
        g_printerr("Elements could be linked");
        gst_object_unref(data.pipeline);
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    tee_audio_pad = gst_element_get_request_pad(data.tee, "src_%u");
    g_print("Obtained request pad %s for audio branch.\n", gst_pad_get_name (tee_audio_pad));
    tee_wave_pad = gst_element_get_request_pad(data.tee, "src_%u");
    g_print("Obtained request pad %s for audio branch.\n", gst_pad_get_name (tee_wave_pad));
    queue_audio_pad = gst_element_get_static_pad(data.audio_queue, "sink");
    queue_wave_pad = gst_element_get_static_pad(data.wave_queue, "sink");

    /*Set the URL to play*/
    g_object_set(data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

    /*Set the location to save audio file*/
    g_object_set(data.source, "location", "aud.wav", NULL);

    /*Set the audio visualaize style*/
    g_object_set(data.visual, "shader", 0, "style", 1, NULL);

    /*Connect to the pad-added signal*/
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

    /*Start playing*/
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if(GST_STATE_CHANGE_FAILURE == ret) {
        g_printerr("unable to set the pipline to playing state");
        gst_object_unref(data.pipeline);
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    /*Listen to the bus*/
    bus = gst_element_get_bus(data.pipeline);
    do {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        /*parse message*/
        if (msg != nullptr) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
                    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                    g_clear_error(&err);
                    g_free(debug_info);
                    terminate = TRUE;
                    break;
               case GST_MESSAGE_EOS:
                    g_print("End-Of-Stream reached.\n");
                    terminate = TRUE;
                    break;
               case GST_MESSAGE_STATE_CHANGED:
                    /* We are only interested in state-changed messages from the pipeline */
                    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
                        GstState old_state, new_state, pending_state;
                        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                        g_print("Pipeline state changed from %s to %s:\n",
                                gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

                    }
                    break;
                default:
                    /* We should not reach here */
                    g_printerr("Unexpected message received.\n");
                    break;
            }
        }
    } while (!terminate);
    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipline");

    /*Release the request pads from the Tee, and unref them*/

    return Video("Error, restart, please", 1, 1, 1, "1");
}

