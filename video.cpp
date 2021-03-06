#include "video.h"

Video::Video(QString filePath, int FPS, int Height, int Width, QString aspectRatio)
{
    this->filePath = filePath;
    this->FPS = FPS;
    this->Heigth = Height;
    this->Width = Width;
    this->aspectRatio = aspectRatio;
}

Video::Video()
{

}

QString Video::getAspectRatio() const
{
    return aspectRatio;
}

void Video::setAspectRatio(const QString &value)
{
    aspectRatio = value;
}

int Video::getHeigth() const
{
    return Heigth;
}

void Video::setHeigth(int value)
{
    Heigth = value;
}

int Video::getWidth() const
{
    return Width;
}

void Video::setWidth(int value)
{
    Width = value;
}

int Video::getFPS() const
{
    return FPS;
}

void Video::setFPS(int value)
{
    FPS = value;
}

QString Video::getFilePath() const
{
    return filePath;
}

void Video::setFilePath(const QString &value)
{
    filePath = value;
}
