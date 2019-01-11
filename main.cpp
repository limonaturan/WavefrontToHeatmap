#include <QCoreApplication>
#include <QString>

#include "wavefront.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    Wavefront obj = Wavefront("/home/peter/Qt/WavefrontToHeatmap/untitled.obj");
    obj.getHeatmap(0.1);

    return a.exec();
}
