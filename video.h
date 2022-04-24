#ifndef VIDEO_H
#define VIDEO_H

#include <QString>



class Video
{
public:
    int getHeigth() const;
    void setHeigth(int value);

    int getWidth() const;
    void setWidth(int value);

    int getFPS() const;
    void setFPS(int value);

    QString getFileRath() const;
    void setFileRath(const QString &value);

    Video(QString filePath, int FPS, int Height, int Width, QString aspectRatio);
private:
    QString fileRath;
    int FPS;
    int Width;
    int Heigth;
    QString aspectRatio;
};

#endif // VIDEO_H
