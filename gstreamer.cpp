#include "gstreamer.h"
#include <gst/gst.h>
#include <QString>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <qthread.h>

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

Video GStreamer::Process(Video video, int FPS, int Heigth, int Width, QString aspectRatio)
{
    return Video("1", 1, 1, 1, "1");
}

