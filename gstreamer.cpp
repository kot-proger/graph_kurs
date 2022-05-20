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

static void pad_added_handler(GstElement *src, GstPad *pad, CustomData *data);

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
    data.convert = gst_element_factory_make("audioconvert", "convert");
    data.resample = gst_element_factory_make("audioresample", "resample");
    data.wavenc = gst_element_factory_make("wavenc", "wavenc");
    data.sink = gst_element_factory_make("filesink", "sink");
    data.wave_queue = gst_element_factory_make("queue", "wave_queue");
    data.visual = gst_element_factory_make ("wavescope", "visual");
    data.wave_convert = gst_element_factory_make ("videoconvert", "csp");
    data.wave_sink = gst_element_factory_make ("autovideosink", "wave_sink");

    /*Create new empty pipline*/
    data.pipeline = gst_pipeline_new("gstreamer-pipline");

    if(!data.pipeline || !data.source || !data.tee || !data.audio_queue || !data.convert || !data.resample || !data.wavenc ||
        !data.sink || !data.wave_queue || !data.visual ||!data.wave_convert || !data.wave_sink) {
        g_printerr("Not elements could be created\n");
        return Video("Error, restart, please", 1, 1, 1, "1");
    }
//    if(!data.pipeline) {
//        g_printerr("1Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.source) {
//        g_printerr("2Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.tee) {
//        g_printerr("3Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.audio_queue) {
//        g_printerr("4Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.convert) {
//        g_printerr("5Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.resample) {
//        g_printerr("6Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.wavenc) {
//        g_printerr("7Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.sink) {
//        g_printerr("8Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.wave_queue) {
//        g_printerr("9Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.visual) {
//        g_printerr("10Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.wave_convert) {
//        g_printerr("11Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }
//    if(!data.wave_sink) {
//        g_printerr("12Not elements could be created\n");
//        return Video("Error, restart, please", 1, 1, 1, "1");
//    }

    /*Build the pipline. Note that we are NOT linking source at this point. We will do it later.*/
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.tee, data.audio_queue, data.convert, data.resample,
                     data.wavenc, data.sink, data.wave_queue,data.visual, data.wave_convert, data.wave_sink, NULL);
    if(!gst_element_link_many(data.convert, data.resample, data.tee, NULL) ||
            !gst_element_link_many(data.audio_queue, data.wavenc, data.sink, NULL) ||
            !gst_element_link_many(data.wave_queue, data.visual, data.wave_convert,data.wave_sink, NULL)) {
        g_printerr("Elements could be linked\n");
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
    g_object_set(data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm"/*video.getFilePath().toStdString().c_str()*/, NULL);

    /*Set the location to save audio file*/
    g_object_set(data.sink, "location", "result.wav", NULL);

    /*Set the audio visualaize style*/
    g_object_set(data.visual, "shader", 0, "style", 1, NULL);

    /*Connect to the pad-added signal*/
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);
    /*Start playing*/
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if(GST_STATE_CHANGE_FAILURE == ret) {
        g_printerr("unable to set the pipline to playing state\n");
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
    gst_element_release_request_pad (data.tee, tee_audio_pad);
    gst_element_release_request_pad (data.tee, tee_wave_pad);
    gst_object_unref (tee_audio_pad);
    gst_object_unref (tee_wave_pad);

    /* Free resources */
    if (msg != NULL)
        gst_message_unref (msg);
    gst_object_unref (bus);
    gst_element_set_state (data.pipeline, GST_STATE_NULL);

    gst_object_unref (data.pipeline);
    g_print("finished");

    return Video("Error, restart, please", 1, 1, 1, "1");
}

static void pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data) {
    GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = nullptr;
    GstStructure *new_pad_struct = nullptr;
    const gchar *new_pad_type = nullptr;

    g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

    /*If our converter is already linked, we have nothing to do there*/
    if(gst_pad_is_linked(sink_pad)) {
        g_print("We are already linked. Ignoring. \n");
        /* Unref the new pad's caps, if we got them */
        if (new_pad_caps != NULL) gst_caps_unref(new_pad_caps);

       /* Unref the sink pad */
       gst_object_unref(sink_pad);
       return;
    }

    /*Check the new pad`s type*/
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    if (!g_str_has_prefix(new_pad_type, "audio/x-raw")) {
        g_print("It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
        /* Unref the new pad's caps, if we got them */
        if (new_pad_caps != NULL) gst_caps_unref(new_pad_caps);

       /* Unref the sink pad */
       gst_object_unref(sink_pad);
       return;
    }

    /*Attempt the link*/
    ret = gst_pad_link(new_pad, sink_pad);
    if(GST_PAD_LINK_FAILED(ret)) {
        g_print("Type is '%s' but link failed.\n", new_pad_type);
    } else {
        g_print("Link succeeded (type '%s').\n", new_pad_type);
    }
    /* Unref the new pad's caps, if we got them */
    if (new_pad_caps != NULL) gst_caps_unref(new_pad_caps);

   /* Unref the sink pad */
   gst_object_unref(sink_pad);
   return;
}
