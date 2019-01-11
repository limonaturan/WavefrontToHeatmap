#include "wavefront.h"

Coordinate3D::Coordinate3D()
{
    x = y = z = 0.;
}

Coordinate3D::Coordinate3D(double X, double Y, double Z)
{
    x = X;
    y = Y;
    z = Z;
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

double WavefrontFace::getHeight(Coordinate2D Point)
{
    double minDouble = -999999.;
    if(normal.z == 0.)
        return minDouble;
    else
        return vertices[0].z-((vertices[0].x-Point.x)*normal.x + (vertices[0].y-Point.y)*normal.y)/normal.z;
}

WavefrontObject::WavefrontObject()
{
    name = "";
    isActive = true;
}

Wavefront::Wavefront(QString Filename)
{
    filename = Filename;
    getObjects(Filename);
}

void Wavefront::getHeatmap(double Precision)
{
    Grid grid = Grid(getMinimum(), getMaximum(), Precision);
    double zMin = 99999.;
    double zMax = -9999.;

    for(int i=0; i<objects.size(); i++) {
        for(int j=0; j<objects[i].faces.size(); j++) {
            Coordinate2D faceMinimumPixel = grid.getFloorPixel(objects[i].faces[j].getMinimum());
            Coordinate2D faceMaximumPixel = grid.getCeilPixel(objects[i].faces[j].getMaximum());

            for(int k=faceMinimumPixel.x; k<faceMaximumPixel.x; k++) {
                for(int l=faceMinimumPixel.y; l<faceMaximumPixel.y; l++) {
                    Coordinate2D point = Coordinate2D(grid.getCoordinate2D(Coordinate2D(k,l)));
                    if(objects[i].faces[j].isPointInPolygon(point)) {
                        double z = objects[i].faces[j].getHeight(point);
                        if(z > grid.getValue(Coordinate2D(k,l))) {
                            grid.setValue(Coordinate2D(k,l), z);
                            if(zMin > z)
                                zMin = z;
                            if(zMax < z)
                                zMax = z;
                        }
                    }
                }
            }
        }
    }
    qDebug() << "Fertig.";
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

Grid::Grid(Coordinate2D Minimum, Coordinate2D Maximum, double Precision)
{
    double lengthX = Maximum.x - Minimum.x;
    double lengthY = Maximum.y - Minimum.y;
    height = int(lengthY/Precision);
    width = int(lengthX/Precision);
    double minDouble = -9999999.;

    pixels.resize(width);

    for(int i=0; i<width; i++) {
        pixels[i].resize(height);
        for(int j=0; j<height; j++) {
            pixels[i][j] = Coordinate3D(Minimum.x+double(i)/double(width-1)*lengthX, Minimum.y+double(j)/double(height-1)*lengthY, minDouble);
        }
    }
}

void Grid::setValue(Coordinate2D Pixel, double Value)
{
    pixels[int(Pixel.x)][int(Pixel.y)].z = Value;
}

double Grid::getValue(Coordinate2D Pixel)
{
    return pixels[int(Pixel.x)][int(Pixel.y)].z;
}

Coordinate2D Grid::getCoordinate2D(Coordinate2D Pixel)
{
    Coordinate3D c = pixels[int(Pixel.x)][int(Pixel.y)];
    return Coordinate2D(c.x,c.y);
}

Coordinate2D Grid::getFloorPixel(Coordinate2D Point)
{
    Coordinate2D minimumPixel = Coordinate2D(0, 0);
    for(int i=0; i<height; i++) {
        if(pixels[0][i].y > Point.y)
            break;
        minimumPixel.y = i;
    }

    for(int i=0; i<width; i++) {
        if(pixels[i][0].x > Point.x)
            break;
        minimumPixel.x = i;
    }

    return minimumPixel;
}

Coordinate2D Grid::getCeilPixel(Coordinate2D Point)
{
    Coordinate2D maximumPixel = Coordinate2D(width-1, height-1);
    for(int i=0; i<height; i++) {
        if(pixels[0][i].y < Point.y)
            break;
        maximumPixel.y = i;
    }

    for(int i=0; i<width; i++) {
        if(pixels[i][0].x < Point.x)
            break;
        maximumPixel.x = i;
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
