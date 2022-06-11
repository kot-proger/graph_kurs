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
    GstElement *sink;
    GstElement *qtmux;
    GstElement *find;
    GstElement *demux;
    GstElement *vp8dec;
    GstElement *vorbisDec;
    GstElement *audioConvert;
    GstElement *videoConvert;
    GstElement *audioResample;
    GstElement *video_queue;
    GstElement *audio_queue;
    GstElement *voaacenc;
    GstElement *x264enc;
    GstElement *premuxVideoQueue;
    GstElement *premuxAudioQueue;
    GstElement *avimux;
    GstElement *videorate;
    GstElement *videoscale;
    GstElement *aspectratiocrop;
    GstElement *parser;
    GstElement *parser1;
    GstElement *capssetter;
    GstElement *capssetter1;
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

    return Video(tmp, FPS, Heigth, Width, aspect);
}

static void pad_added_handler(GstElement *src, GstPad *pad, CustomData *data);

Video GStreamer::Process(Video video, int FPS, int Heigth, int Width, QString aspectRatio)
{
    CustomData data{};
    /*Create the elements*/
    data.source = gst_element_factory_make("filesrc", "source");
    data.find = gst_element_factory_make("typefind", "typeFindElement");
    data.demux = gst_element_factory_make("qtdemux", "demux0");

    data.aspectratiocrop = gst_element_factory_make("aspectratiocrop", "aspectratiocrop");
    data.videoscale = gst_element_factory_make("videoscale", "videoscale");
    data.videorate = gst_element_factory_make("videorate", "videorate");
    data.capssetter = gst_element_factory_make("capssetter", "capssetter");
    data.capssetter1 = gst_element_factory_make("capssetter", "capssetter1");

    data.vorbisDec = gst_element_factory_make("avdec_aac", "vorbisdec0");
    data.vp8dec = gst_element_factory_make("avdec_h264", "nvh264dec_0");
    data.parser = gst_element_factory_make("h264parse", "h264parse");
    data.parser1 = gst_element_factory_make("h264parse", "h264parse1");

    data.audioConvert = gst_element_factory_make("audioconvert", "AudioConvert");
    data.videoConvert = gst_element_factory_make("videoconvert", "VideoConvert");

    data.audioResample = gst_element_factory_make("audioresample", "resample");

    data.audio_queue = gst_element_factory_make("queue", "audio_queue");
    data.video_queue = gst_element_factory_make("queue", "video_queue");

    data.voaacenc = gst_element_factory_make("voaacenc", "AudioEncoder");
    data.x264enc = gst_element_factory_make("nvh264enc","VideoEncoder");

    data.premuxAudioQueue = gst_element_factory_make("queue", "premuxAudioQueue");
    data.premuxVideoQueue = gst_element_factory_make("queue", "premuxVideoQueue");

    data.avimux =  gst_element_factory_make("avimux","Muxer");

    data.sink = gst_element_factory_make("filesink", "sink");
    data.pipeline = gst_pipeline_new("pipeline");

    /*Create new empty pipline*/
    data.pipeline = gst_pipeline_new("gstreamer-pipline");

    if (!data.pipeline || !data.find || !data.source || !data.sink) {
        if (!data.pipeline) {
            g_printerr(" Pipeline could not be created\n");
        }

        if (!data.find) {
            g_printerr(" typefind could not be created\n");
        }

        if (!data.source) {
            g_printerr(" filesrc could not be created\n");
        }

        if (!data.sink) {
            g_printerr(" filesink could not be created\n");
        }
        return Video("Error, restart, please", 1, 1, 1, "1");
    }


    if(!data.aspectratiocrop || !data.videoscale || !data.videorate || !data.capssetter || !data.capssetter1) {
        if(!data.aspectratiocrop) {
            g_printerr("aspectratiocrop could not be created\n");
        }

        if(!data.videoscale) {
            g_printerr("videoscale could not be created\n");
        }

        if(!data.videorate) {
            g_printerr("videorate could not be created\n");
        }

        if (!data.capssetter) {
            g_printerr("capssetter could not be created");
        }

        if (!data.capssetter1) {
            g_printerr("capssetter1 could not be created");
        }
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    if (!data.audioConvert || !data.videoConvert) {
        if (!data.audioConvert) {
            g_printerr(" audioConvert could not be created\n");
        }

        if (!data.videoConvert) {
            g_printerr(" videoConvert could not be created\n");
        }
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    if (!data.vorbisDec || !data.vp8dec || !data.voaacenc || !data.x264enc || !data.parser || !data.parser1)
    {
        if (!data.vorbisDec) {
            g_printerr(" vorbisDec could not be created\n");
        }

        if (!data.vp8dec) {
            g_printerr(" vp8dec could not be created\n");
        }

        if (!data.x264enc) {
            g_printerr(" x264enc could not be created\n");
        }

        if (!data.voaacenc) {
            g_printerr(" voaacenc could not be created\n");
        }

        if (!data.parser) {
            g_printerr("parser could not be cxreated\n");
        }

        if (!data.parser1) {
            g_printerr("parser1 could not be cxreated\n");
        }
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    if (!data.audioResample) {
        if (!data.audioResample) {
            g_printerr(" audioResample could not be created\n");
        }

        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    if (!data.demux || !data.avimux) {
        if (!data.demux) {
            g_printerr(" demux could not be created\n");
        }

        if (!data.avimux) {
            g_printerr(" avimux could not be created\n");
        }
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    /*Build the pipline. Note that we are NOT linking source at this point. We will do it later.*/

    gst_bin_add_many
    (
        GST_BIN(data.pipeline),
        data.source,
        data.find,
        data.demux,
        data.vorbisDec,
        data.vp8dec,
        data.audioConvert,
        data.videoConvert,
        data.audioResample,
        data.videorate,
        data.videoscale,
        data.aspectratiocrop,
        data.audio_queue,
        data.video_queue,
        data.voaacenc,
        data.x264enc,
        data.premuxAudioQueue,
        data.premuxVideoQueue,
        data.avimux,
        data.parser,
        data.parser1,
        data.capssetter,
        data.capssetter1,
        data.sink, NULL
    );

    g_object_set(data.audio_queue, "max-size-time", 100000000000,NULL);
    g_object_set(data.video_queue, "max-size-time", 100000000000,NULL);

    if( !gst_element_link_many(data.source, data.find, data.demux, NULL) ||
        !gst_element_link_many(data.audio_queue, data.vorbisDec, data.audioConvert, data.audioResample, data.voaacenc, data.premuxAudioQueue, NULL) ||
        !gst_element_link_many(data.video_queue, data.parser, data.vp8dec, data.videoConvert, data.aspectratiocrop, data.videoscale, data.capssetter, data.videorate, data.capssetter1, data.x264enc, data.parser1, data.premuxVideoQueue, NULL) ||
        !gst_element_link_many(data.avimux, data.sink, NULL)) {
        g_printerr("Elements could be linked\n");
        gst_object_unref(data.pipeline);
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    gst_debug_bin_to_dot_file(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipline3.dot");
    GstPad* audioPad;
    GstPad* videoPad;
    GstPad* audioSrc;
    GstPad* videoSrc;

    audioPad = gst_element_get_request_pad(data.avimux, "audio_%u");
    videoPad = gst_element_get_request_pad(data.avimux, "video_%u");

    audioSrc = gst_element_get_static_pad(data.premuxAudioQueue, "src");
    videoSrc = gst_element_get_static_pad(data.premuxVideoQueue, "src");

    if (gst_pad_link(audioSrc, audioPad) != GST_PAD_LINK_OK ||
        gst_pad_link(videoSrc, videoPad) != GST_PAD_LINK_OK) {
        g_printerr(" avimux could not be linked.\n");
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "LinkRequestPad");
        gst_object_unref(data.pipeline);
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    gchar tmp[255];
    sprintf(tmp, "%s", video.getFilePath().toStdString().c_str());
    g_object_set(data.source, "location", tmp, NULL);

    /*Set the location to save file*/
    sprintf(tmp, "%s_result.mp4", video.getFilePath().toStdString().c_str());
    g_object_set(data.sink, "location", tmp, NULL);

    sprintf(tmp, "video/x-raw,framerate=%d/1", FPS);
    g_object_set(data.capssetter1, "caps", gst_caps_from_string(tmp), NULL);

    sprintf(tmp, "video/x-raw,width=%d,height=%d", Width, Heigth);
    g_object_set(data.capssetter, "caps", gst_caps_from_string("video/x-raw,width=1280,height=1024"), NULL);
    g_object_set(data.videoscale, "method", 9, NULL);

    /*Connect to the pad-added signal*/
    if (g_signal_connect(data.demux, "pad-added", G_CALLBACK(pad_added_handler), data.audio_queue) &&
        g_signal_connect(data.demux, "pad-added", G_CALLBACK(pad_added_handler), data.video_queue)) {
    } else {
        g_print("signal connect failed\n");
        return Video("Error, restart, please", 1, 1, 1, "1");
    }

    /*Start playing*/
    GstStateChangeReturn ret;

    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

    if(GST_STATE_CHANGE_FAILURE == ret) {
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "FailPlay");
        g_printerr("unable to set the pipline to playing state\n");
        gst_object_unref(data.pipeline);
        return Video("Error, restart, please", 1, 1, 1, "1");
    }
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipline2.dot");

    /*Listen to the bus*/

    GstBus *bus;
    GstMessage *msg;
    gboolean terminate = false;

    bus = gst_element_get_bus(data.pipeline);

    do {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "playing.dot");
        // Parse message
        if (msg != nullptr)
        {
            GError* err;
            gchar* debug_info;

            switch (GST_MESSAGE_TYPE(msg))
            {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_print("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_print("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                terminate = TRUE;
                break;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                terminate = TRUE;
                break;
            case GST_MESSAGE_STATE_CHANGED:
                // We are only interested in state-changed messages from the pipeline
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline))
                {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                    g_print("Pipeline state changed from %s to %s:\n",
                        gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

                }
                break;
            default:
                //We should not reach here
                g_print("Unexpected message received.\n");
                break;
            }
        }
    } while (!terminate);
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data.pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipline");

    /* Free resources */
    if (msg != NULL)
        gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (data.pipeline, GST_STATE_NULL);

    gst_object_unref (data.pipeline);
    g_print("finished\n");

    return Video("Error, restart, please", 1, 1, 1, "1");
}

static void pad_added_handler(GstElement *element, GstPad *new_pad, CustomData *data) {
    GstPad* sinkpad;
    GstElement* decoder = (GstElement*)data;
    g_print("Dynamic pad created, linking demuxer/decoder\n");
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(new_pad, sinkpad);
    gst_object_unref(sinkpad);
}
