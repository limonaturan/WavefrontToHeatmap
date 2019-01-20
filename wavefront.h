#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QRgb>
#include <QColor>
#include <QDateTime>
#include <iostream>
#include <QTimer>
#include <QObject>
#include <QCoreApplication>

class Coordinate3D
{
public:
    Coordinate3D();
    Coordinate3D(double, double, double);
    Coordinate3D(double, double);

    double x;
    double y;
    double z;

    bool isValid;
};


class Coordinate2D
{
public:
    Coordinate2D();
    Coordinate2D(double, double);

    double x;
    double y;
};

class Pixel3D
{
public:
    Pixel3D();

    int i;
    int j;
    double value;
    QColor color;

    bool isValid;
};

class Pixel2D
{
public:
    Pixel2D();
    Pixel2D(int I, int J);

    int i;
    int j;

    bool isValid;
};


class WavefrontFace
{
public:
    WavefrontFace();

    Coordinate2D getMinimum();
    Coordinate2D getMaximum();

    bool isPointInPolygon(Coordinate2D);
    Coordinate3D getHeight(Coordinate2D Point);

    QVector<Coordinate3D> vertices;
    Coordinate3D normal;
};


class WavefrontObject
{
public:
    WavefrontObject();
    QString name;
    QVector<WavefrontFace> faces;
    bool isActive;
};


class Grid
{
public:
    Grid(Coordinate2D Minimum, Coordinate2D Maximum, double Precision);

    void setValue(Pixel2D Pixel, double Value);

    Coordinate3D getCoordinate3D(Pixel2D Pixel);
    Coordinate2D getCoordinate2D(Pixel2D Pixel);

    Pixel2D getFloorPixel(Coordinate2D Point);
    Pixel2D getCeilPixel(Coordinate2D Point);

    int getHeight();
    int getWidth();

    void normalize();

private:
    struct GridPoint {
        Coordinate3D c;
        Pixel2D p;
    };

    QVector<QVector<Coordinate3D>> pixels;
    QVector<QVector<GridPoint>> grid;
    int width;
    int height;
    Coordinate2D minimum, maximum;
};


class Wavefront : public QObject
{
    Q_OBJECT
public:
    Wavefront(QString Filename, bool commandLineOutput);

    ~Wavefront();

    Grid getHeatmap(double Precision);
    double getWidth();
    double getHeight();

    QImage getImage(Grid grid);
    void saveImage(QImage Image, QString Filename);

public slots:
    void outputProgress();

private:
    QString filename;

    QVector<WavefrontObject> objects;


    // Methods
    QVector<WavefrontObject> getObjects(QString Filename);
    Coordinate2D getMinimum();
    Coordinate2D getMaximum();

    int getNumberOfIterations(Grid);
    bool commandLineOutput;

    QTimer *timer;
    int totalNumberOfIterations;
    int iteration;
    QDateTime timeStart;

};

#endif // WAVEFRONT_H
