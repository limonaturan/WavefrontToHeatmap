#include "wavefront.h"

Coordinate3D::Coordinate3D()
{
    x = y = z = 0.;
    isValid = false;
}

Coordinate3D::Coordinate3D(double X, double Y, double Z)
{
    x = X;
    y = Y;
    z = Z;
    isValid = true;
}

Coordinate3D::Coordinate3D(double X, double Y)
{
    x = X;
    y = Y;
    z = 0.;
    isValid = false;
}

Coordinate2D::Coordinate2D()
{
    x = y = 0.;
}

Coordinate2D::Coordinate2D(double X, double Y)
{
    x = X;
    y = Y;
}

WavefrontFace::WavefrontFace()
{
    normal = Coordinate3D();
}

Coordinate2D WavefrontFace::getMinimum()
{
    double maxDouble = 9999999.;
    Coordinate2D minimum = Coordinate2D(maxDouble,maxDouble);
    for(int i=0; i<vertices.size(); i++) {
        if(minimum.x > vertices[i].x)
            minimum.x = vertices[i].x;
        if(minimum.y > vertices[i].y)
            minimum.y = vertices[i].y;
    }
    return minimum;
}

Coordinate2D WavefrontFace::getMaximum()
{
    double minDouble = -9999999.;
    Coordinate2D maximum = Coordinate2D(minDouble,minDouble);
    for(int i=0; i<vertices.size(); i++) {
        if(maximum.x < vertices[i].x)
            maximum.x = vertices[i].x;
        if(maximum.y < vertices[i].y)
            maximum.y = vertices[i].y;
    }
    return maximum;
}

bool WavefrontFace::isPointInPolygon(Coordinate2D Point)
{
    int i, j, nvert = vertices.size();
    bool c = false;

    for(i = 0, j = nvert - 1; i < nvert; j = i++) {
        if( ( (vertices[i].y >= Point.y ) != (vertices[j].y >= Point.y) ) &&
            (Point.x <= (vertices[j].x - vertices[i].x) * (Point.y - vertices[i].y) / (vertices[j].y - vertices[i].y) + vertices[i].x)
          )
          c = !c;
    }

    return c;
}

Coordinate3D WavefrontFace::getHeight(Coordinate2D Point)
{
    Coordinate3D vertex = Coordinate3D(Point.x, Point.y, 0);
    if(normal.z == 0.) {
        vertex.isValid = false;
        return vertex;
    }
    else {
        double X = (vertices[0].x-vertex.x)*normal.x;
        double Y = (vertices[0].y-vertex.y)*normal.y;
        vertex.z = vertices[0].z+(X + Y)/normal.z;
        vertex.isValid = true;
        return vertex;
    }
}

WavefrontObject::WavefrontObject()
{
    name = "";
    isActive = true;
}

Wavefront::Wavefront(QString Filename, bool commandLineOutput)
{
    QCoreApplication::processEvents();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(outputProgress()));
    this->commandLineOutput = commandLineOutput;
    filename = Filename;
    getObjects(Filename);
}

Wavefront::~Wavefront()
{
    timer->stop();
}

Grid Wavefront::getHeatmap(double Precision)
{
    bool isPointInPolygon;
    WavefrontFace face;
    Coordinate2D faceMinimumPoint, faceMaximumPoint;
    Pixel2D faceMinimumPixel, faceMaximumPixel;
    Coordinate3D oldVertex, newVertex;
    QDateTime timeEnd, timeLastUpdate;
    timeStart = QDateTime::currentDateTime();
    timeLastUpdate = timeStart;

    Grid grid = Grid(getMinimum(), getMaximum(), Precision);

    totalNumberOfIterations = getNumberOfIterations(grid);
    iteration = 0;
    timer->start(1000);

    for(int i=0; i<objects.size(); i++) {
        for(int j=0; j<objects[i].faces.size(); j++) {
            face = objects[i].faces[j];

            faceMinimumPoint = face.getMinimum();
            faceMinimumPixel = grid.getFloorPixel(faceMinimumPoint);

            faceMaximumPoint = face.getMaximum();
            faceMaximumPixel = grid.getCeilPixel(faceMaximumPoint);

            for(int k=faceMinimumPixel.i; k<=faceMaximumPixel.i; k++) {
                for(int l=faceMinimumPixel.j; l<=faceMaximumPixel.j; l++) {
                    QCoreApplication::processEvents();
                    iteration += 1;
                    oldVertex = grid.getCoordinate3D(Pixel2D(k,l));
                    isPointInPolygon = face.isPointInPolygon(Coordinate2D(oldVertex.x,oldVertex.y));
                    if(isPointInPolygon) {
                        newVertex = face.getHeight(Coordinate2D(oldVertex.x,oldVertex.y));
                        if(newVertex.isValid && (newVertex.z > oldVertex.z || !oldVertex.isValid)) {
                            grid.setValue(Pixel2D(k,l), newVertex.z);
                        }
                    }
                }
            }
        }
    }
    timer->stop();
    return grid;
}

double Wavefront::getWidth()
{
    Coordinate2D max = getMaximum();
    Coordinate2D min = getMinimum();

    return max.x - min.x;
}

double Wavefront::getHeight()
{
    Coordinate2D max = getMaximum();
    Coordinate2D min = getMinimum();

    return max.y - min.y;
}


QImage Wavefront::getImage(Grid grid)
{
    QImage image = QImage(grid.getWidth(), grid.getHeight(), QImage::Format_Grayscale8);
    image.setColorCount(255);

    for(int i=0; i<255; i++) {
        image.setColor(i,QColor(i,i,i, 0).rgb());
    }

    grid.normalize();

    int height = grid.getHeight();
    int width  = grid.getWidth();

    for(int i=0; i<width; i++) {
        for(int j=0; j<height; j++) {
            Coordinate3D coordinate3D = grid.getCoordinate3D(Pixel2D(i,j));
            if(coordinate3D.isValid) {
                int value = coordinate3D.z*255.;
                image.setPixel(i, height-1-j, qRgb(value, value, value));
            }
            else {
                image.setPixel(i, height-1-j, 0);
            }
        }
    }
    return image;
}

void Wavefront::saveImage(QImage Image, QString Filename)
{
    Image.save(Filename, "PNG", -1);
}

void Wavefront::outputProgress()
{
    double progress = double(iteration)/double(totalNumberOfIterations)*100.;
    int elapsed = (QDateTime::currentDateTime().toMSecsSinceEpoch()-timeStart.toMSecsSinceEpoch())/1000;
    int remaining = double(elapsed)/double(progress)*(100.-double(progress));
    if(commandLineOutput) {
        if(elapsed == 0) {
            std::cout << "Progress: " << progress << " %. " <<
                         "Elapsed: " << elapsed << " s. " << std::endl;
        }
        else {
                std::cout << "Progress: " << progress << " %. " <<
                             "Elapsed: " << elapsed << " s. " <<
                             "Remaining: " << remaining << " s." << std::endl;
        }
    }
    else {
        // TODO: implement timed signal that informs about progress
    }
}

QVector<WavefrontObject> Wavefront::getObjects(QString Filename)
{
    if(!objects.isEmpty())
        return objects;

    QFile file(Filename);
    if(!file.exists())
        return objects;

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString contents = in.readAll();
    file.close();

    QStringList lines = contents.split("\n");
    QVector<Coordinate3D> vertices;
    QVector<Coordinate3D> normals;
    WavefrontObject object = WavefrontObject();

    for(int i=0; i<lines.size(); i++) {
        QStringList line = lines[i].split(" ");
        QString identifier;
        if(!line.isEmpty())
            identifier = line[0];

        if(identifier == "o") {
            if(!object.name.isEmpty()) {
                objects.append(object);
                object = WavefrontObject();
            }
            object.name = line[1];
        }
        else if(!object.name.isEmpty() && identifier == "v") {
            vertices.append(Coordinate3D(line[1].toDouble(), line[2].toDouble(), line[3].toDouble()));
        }
        else if(!object.name.isEmpty() && identifier == "vn") {
            normals.append(Coordinate3D(line[1].toDouble(), line[2].toDouble(), line[3].toDouble()));
        }
        else if(!object.name.isEmpty() && identifier == "f") {
            WavefrontFace face = WavefrontFace();
            int numberOfVertices = line.size() - 1;
            for(int k=1; k<numberOfVertices+1; k++) {
                QStringList item = line[k].split("//");
                if(k==1) {
                    int u = item[1].toInt()-1;
                    face.normal = normals[u];
                }
                face.vertices.append(vertices[item[0].toInt()-1]);
            }
            object.faces.append(face);
        }
        if(!object.name.isEmpty() && lines.size()-1==i) {
            objects.append(object);
        }
    }
    return objects;
}

Coordinate2D Wavefront::getMinimum()
{
    double maxDouble = 9999999.;
    Coordinate2D minimum = Coordinate2D(maxDouble,maxDouble);
    for(int i=0; i<objects.size(); i++) {
        if(!objects[i].isActive)
            continue;
        for(int j=0; j<objects[i].faces.size(); j++) {
            for(int k=0; k<objects[i].faces[j].vertices.size(); k++) {
                if(minimum.x > objects[i].faces[j].vertices[k].x)
                    minimum.x = objects[i].faces[j].vertices[k].x;
                if(minimum.y > objects[i].faces[j].vertices[k].y)
                    minimum.y = objects[i].faces[j].vertices[k].y;
            }
        }
    }
    return minimum;
}

Coordinate2D Wavefront::getMaximum()
{
    double minDouble = -9999999.;
    Coordinate2D maximum = Coordinate2D(minDouble,minDouble);
    for(int i=0; i<objects.size(); i++) {
        if(!objects[i].isActive)
            continue;
        for(int j=0; j<objects[i].faces.size(); j++) {
            for(int k=0; k<objects[i].faces[j].vertices.size(); k++) {
                if(maximum.x < objects[i].faces[j].vertices[k].x)
                    maximum.x = objects[i].faces[j].vertices[k].x;
                if(maximum.y < objects[i].faces[j].vertices[k].y)
                    maximum.y = objects[i].faces[j].vertices[k].y;
            }
        }
    }
    return maximum;
}

int Wavefront::getNumberOfIterations(Grid grid)
{
    WavefrontFace face;
    Coordinate2D faceMinimumPoint, faceMaximumPoint;
    Pixel2D faceMinimumPixel, faceMaximumPixel;
    int number = 0;
    for(int i=0; i<objects.size(); i++) {
        for(int j=0; j<objects[i].faces.size(); j++) {
            face = objects[i].faces[j];

            faceMinimumPoint = face.getMinimum();
            faceMinimumPixel = grid.getFloorPixel(faceMinimumPoint);

            faceMaximumPoint = face.getMaximum();
            faceMaximumPixel = grid.getCeilPixel(faceMaximumPoint);

            number += (faceMaximumPixel.i - faceMinimumPixel.i + 1)*(faceMaximumPixel.j - faceMinimumPixel.j + 1);
        }
    }
    return number;
}

Grid::Grid(Coordinate2D Minimum, Coordinate2D Maximum, double Precision)
{
    double lengthX = Maximum.x - Minimum.x;
    double lengthY = Maximum.y - Minimum.y;
    height = int(lengthY/Precision);
    width = int(lengthX/Precision);
    minimum = Minimum;
    maximum = Maximum;


    double slopeX, slopeY;
    if(width == 1)
        slopeX = 0.;
    else
        slopeX = lengthX/double(width-1);

    if(height == 1)
        slopeY = 0.;
    else
        slopeY = lengthY/double(height-1);

    pixels.resize(width);
    for(int i=0; i<width; i++) {
        pixels[i].resize(height);
        for(int j=0; j<height; j++) {
            pixels[i][j] = Coordinate3D(Minimum.x+double(i)*slopeX, Minimum.y+double(j)*slopeY);
        }
    }
}

void Grid::setValue(Pixel2D Pixel, double Value)
{
    pixels[Pixel.i][Pixel.j].z = Value;
    pixels[Pixel.i][Pixel.j].isValid = true;
}

Coordinate3D Grid::getCoordinate3D(Pixel2D Pixel)
{
    return pixels[Pixel.i][Pixel.j];
}

Coordinate2D Grid::getCoordinate2D(Pixel2D Pixel)
{
    Coordinate3D c = pixels[Pixel.i][Pixel.j];
    return Coordinate2D(c.x,c.y);
}

Pixel2D Grid::getFloorPixel(Coordinate2D Point)
{
    Pixel2D minimumPixel = Pixel2D(0, 0);
    for(int j=0; j<height; j++) {
        if(pixels[0][j].y > Point.y)
            break;
        minimumPixel.j = j;
    }

    for(int i=0; i<width; i++) {
        if(pixels[i][0].x > Point.x)
            break;
        minimumPixel.i = i;
    }

    return minimumPixel;
}

Pixel2D Grid::getCeilPixel(Coordinate2D Point)
{
    Pixel2D maximumPixel = Pixel2D(width-1, height-1);
    for(int j=0; j<height; j++) {
        if(pixels[0][j].y < Point.y)
            break;
        maximumPixel.j = j;
    }

    for(int i=0; i<width; i++) {
        if(pixels[i][0].x < Point.x)
            break;
        maximumPixel.i = i;
    }

    return maximumPixel;
}

int Grid::getHeight()
{
    return height;
}

int Grid::getWidth()
{
    return width;
}

void Grid::normalize()
{
    double globalMax = -99999.;
    double globalMin = 99999.;
    for(int i=0; i<width; i++) {
        for(int j=0; j<height; j++) {
            if(!pixels[i][j].isValid)
                continue;
            Coordinate3D p = pixels[i][j];
            if(globalMax < p.z)
                globalMax = p.z;
            if(globalMin > p.z)
                globalMin = p.z;
        }
    }

    double delta = globalMax - globalMin;

    for(int i=0; i<width; i++) {
        for(int j=0; j<height; j++) {
            if(pixels[i][j].isValid)
                pixels[i][j].z = (pixels[i][j].z - globalMin)/delta;
        }
    }
}

Pixel2D::Pixel2D()
{
    i = j = 0;
    isValid = false;
}

Pixel2D::Pixel2D(int I, int J)
{
    i = I;
    j = J;
    isValid = true;
}

Pixel3D::Pixel3D()
{
    i = j = 0;
    value = 0.;
    isValid = false;
}
