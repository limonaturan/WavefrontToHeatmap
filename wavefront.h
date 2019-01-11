#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDebug>


class Coordinate3D
{
public:
    Coordinate3D();
    Coordinate3D(double, double, double);

    double x;
    double y;
    double z;
};


class Coordinate2D
{
public:
    Coordinate2D();
    Coordinate2D(double, double);

    double x;
    double y;
};


class WavefrontFace
{
public:
    WavefrontFace();

    Coordinate2D getMinimum();
    Coordinate2D getMaximum();

    bool isPointInPolygon(Coordinate2D);
    double getHeight(Coordinate2D Point);

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

    void setValue(Coordinate2D Pixel, double Value);
    double getValue(Coordinate2D Pixel);

    Coordinate2D getCoordinate2D(Coordinate2D Pixel);


    Coordinate2D getFloorPixel(Coordinate2D Point);
    Coordinate2D getCeilPixel(Coordinate2D Point);

    int getHeight();
    int getWidth();

private:
    QVector<QVector<Coordinate3D>> pixels;
    int width;
    int height;
};


class Wavefront
{
public:
    Wavefront(QString Filename);

    void getHeatmap(double Precision);

private:
    QString filename;

    QVector<WavefrontObject> objects;


    // Methods
    QVector<WavefrontObject> getObjects(QString Filename);
    Coordinate2D getMinimum();
    Coordinate2D getMaximum();
};

#endif // WAVEFRONT_H
