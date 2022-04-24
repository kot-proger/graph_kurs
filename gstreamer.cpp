#include "gstreamer.h"
#include <gst/gst.h>
#include <QString>
#include <stdlib.h>
#include <fstream>

GStreamer::GStreamer(int argc, char *argv[])
{
    gst_init(&argc, &argv);
}

GStreamer::GStreamer(){

}

Video GStreamer::Analyse(QString filePath)
{
    const char* command = qPrintable(QString("gst-discoverer-1.0 " + filePath + " > analyse.txt"));
    system(command);
    return Video("1", 1, 1, 1, "1");
}

Video GStreamer::Process(Video video, int FPS, int Heigth, int Width, QString aspectRatio)
{

}

