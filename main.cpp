#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <iostream>
#include <string>
#include <math.h>

#include "wavefront.h"

struct Input {
    bool wavefrontFilenameSupplied = false;
    bool wavefrontFilenameValid = false;
    QString wavefrontFilename = QString();

    bool outputFilenameSupplied = false;
    bool outputFilenameValid = false;
    QString outputFilename = QString();

    bool precisionSupplied = false;
    bool precisionValid = false;
    double precision = 0.;

    bool pixelCountSupplied = false;
    bool pixelCountValid = false;
    double pixelCount = 0.;

    bool helpRequested = false;

    bool commandLine = true;
};

Input getInputParameters(int argc, char *argv[]) {
    Input input;
    for(int i=0; i<argc; i++) {
        std::string argument = argv[i];
        if(argument == "-i") { // Input file
            input.wavefrontFilenameSupplied = true;
            if(i+1<argc) {
                i++;
                input.wavefrontFilename = argv[i];
                if(QFileInfo(input.wavefrontFilename).isFile() || QFileInfo(input.wavefrontFilename).suffix() == ".obj") {
                    input.wavefrontFilenameValid = true;
                }
            }
        }
        else if(argument == "-o") { // Output file
            input.outputFilenameSupplied = true;
            if(i+1<argc) {
                i++;
                input.outputFilename = argv[i];
                if(QFileInfo(input.outputFilename).isFile() || QFileInfo(input.outputFilename).suffix() == ".png") {
                    input.wavefrontFilenameValid = true;
                }
            }
        }
        else if(argument == "-p") {
            input.precisionSupplied = true;
            if(i+1<argc) {
                i++;
                bool validConversion;
                input.precision = QString(argv[i]).toDouble(&validConversion);
                input.precisionValid = validConversion;
            }
        }
        else if(argument == "-c") {
            input.pixelCountSupplied = true;
            if(i+1<argc) {
                i++;
                bool validConversion;
                input.pixelCount = QString(argv[i]).toInt(&validConversion);
                input.pixelCountValid = validConversion;
            }
        }
        else if(argument == "-h") {
            input.helpRequested = true;
        }
    }

    if(!input.pixelCountSupplied && !input.precisionSupplied) {
        input.pixelCount = 500000;
        input.pixelCountValid = true;
    }

    if(!input.outputFilenameSupplied && input.wavefrontFilenameValid) {
        input.outputFilenameValid = true;
        input.outputFilename = QFileInfo(input.wavefrontFilename).absoluteFilePath().replace(".obj", "") + ".png";
    }

    return input;
}

bool checkInput(Input input) {
    if(!input.wavefrontFilenameSupplied) {
        std::cout << "No input file supplied. Type -h for further help." << std::endl;
        return false;
    }
    if(!input.wavefrontFilenameValid) {
        std::cout << "Invalid input file. Type -h for further help." << std::endl;
        return false;
    }
    if(input.precisionSupplied && !input.precisionValid) {
        std::cout << "Invalid precision. Type -h for further help." << std::endl;
        return false;
    }
    if(input.pixelCountSupplied && !input.pixelCountValid) {
        std::cout << "Invalid pixel count. Type -h for further help." << std::endl;
        return false;
    }
    if(input.pixelCountSupplied && input.precisionSupplied) {
        std::cout << "Both pixel count and precision are supplied. Type -h for further help." << std::endl;
        return false;
    }
    return true;
}

void saveImageByPrecision(Input input) {
    Wavefront* obj = new Wavefront(input.wavefrontFilename, input.commandLine);
    Grid heatmap = obj->getHeatmap(input.precision);
    QImage image = obj->getImage(heatmap);
    obj->saveImage(image, input.outputFilename);
}

void saveImageByPixelCount(Input input) {
    Wavefront* obj = new Wavefront(input.wavefrontFilename, input.commandLine);
    double width = obj->getWidth();
    double height = obj->getHeight();
    double precision = std::sqrt((width*height)/double(input.pixelCount));
    Grid heatmap = obj->getHeatmap(precision);
    QImage image = obj->getImage(heatmap);
    obj->saveImage(image, input.outputFilename);
}

void displayHelp() {
    std::cout << "Usage of WavefrontToHeatmap:" << std::endl;
    std::cout << "-h" << "\t" << "Display this help. " << std::endl;
    std::cout << std::endl;
    std::cout << "IO settings:" << std::endl;
    std::cout << "-i <InputFile>" << "\t" << "Specify an input Wavefront obj file. (MANDATORY)" << std::endl;
    std::cout << "-o <OutputFile>" << "\t" << "Specify an output png file." << std::endl;
    std::cout << std::endl;
    std::cout << "Resolution settings:" << std::endl;
    std::cout << "You can either specify the pixel count or the precision." <<
                 "Only specify one of the following two parameters. If none is specified the default value -c 500000 will be used." << std::endl;
    std::cout << "-c <PixelCount>" << "\t" << "Specify the output file's pixel count." << std::endl;
    std::cout << "-p <Precision>" << "\t" << "Specify the output file's precision." << std::endl;
}



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Input input = getInputParameters(argc, argv);

    input.wavefrontFilename = "/home/peter/Qt/WavefrontToHeatmap/y.obj";
    input.wavefrontFilenameSupplied = input.wavefrontFilenameValid = true;

    input.pixelCount = 50000;
    input.pixelCountSupplied = input.precisionValid = true;

    if(input.helpRequested) { // Display help
        displayHelp();
        return 0;
    }

    if(!checkInput(input)) // Stop
        return 0;

    if(input.pixelCountSupplied) {
        saveImageByPixelCount(input);
    }
    else if(input.precisionSupplied) {
        saveImageByPrecision(input);
    }
    else {
        std::cout << "Setting default value -c 500000." << std::endl;
        saveImageByPixelCount(input);
    }
    std::cout << "Finished." << std::endl;



    return 0;
}
