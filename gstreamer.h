#ifndef GSTREAMER_H
#define GSTREAMER_H

#include <video.h>



class GStreamer
{
public:
    GStreamer(int argc, char *argv[]);
    Video Analyse(QString filePath);
    Video Process(Video video, int FPS, int Heigth, int Width, QString aspectRatio);

    GStreamer();
};

#endif // GSTREAMER_H
