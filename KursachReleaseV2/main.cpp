#include "widget.h"

#include <QApplication>
#include <Q3DSurface>
#include <QFile>
#include <QTextStream>
#include <QValue3DAxis>
#include <QObject>
#include <Q3DTheme>
#include <QAbstract3DGraph>
#include <QCustom3DItem>
#include <QHBoxLayout>
#include <QHeightMapSurfaceDataProxy>
#include <QMessageBox>

using namespace QtDataVisualization;

//Путь к файлу с данными карты высот
QString pathMap = "D:/QTProjects/PIZDEC/heightmap.dat";
//Путь к файлу с данными для траектории
QString pathData = "D:/QTProjects/PIZDEC/output.dat";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //Инициализация экземпляра класса отрисовки
    Q3DSurface *surface = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(surface);

    //Для отображения графиков, библиотека использует OpenGL. Тут идет проверка, есть ли он.
    if (!surface->hasContext()) {
        QMessageBox::critical(nullptr, "Error", "Couldn't initialize the OpenGL context.");
        return -1;
    }

    //Создание виджетов для размещения контейнера
    //Если проще, то создание самого окна для отображения всего
    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(container);
    container->setMinimumSize(1000, 800);
    container->setMaximumSize(1000, 800);
    surface->setShadowQuality(QtDataVisualization::QAbstract3DGraph::ShadowQualityNone);

    //Инициализация экземпляра класса для работы с данными для траектории
    QSurfaceDataProxy *proxy = new QSurfaceDataProxy();
    QSurface3DSeries *series = new QSurface3DSeries(proxy);

    //Надстройки отображения данных
    series->setBaseColor(Qt::black);
    series->setMeshSmooth(true);
    series->setDrawMode(QSurface3DSeries::DrawSurface);

    //Создание массива данных
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;

    // Открытие файла
    QFile file(pathData);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file.");
        return -1;
    }

    QSurfaceDataRow *dataRow1 = new QSurfaceDataRow;
    QSurfaceDataRow *dataRow2 = new QSurfaceDataRow;

    //Чтение файла и запись данных в массив
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList values = in.readLine().split("\t");
        double height = values[10].toDouble();
        double longitude = values[18].toDouble();
        double latitude = values[17].toDouble();

        QVector3D position1(longitude, height, latitude);
        (*dataRow1) << position1;

        QVector3D position2(longitude, height, latitude + 0.01);
        (*dataRow2) << position2;
    }

    // Добавление строк данных в массив данных
    dataArray->append(dataRow1);
    dataArray->append(dataRow2);

    //Инициализация экземпляра класса для работы с данными для карты высот
    QSurfaceDataProxy *proxy2 = new QSurfaceDataProxy();
    QSurface3DSeries *series2 = new QSurface3DSeries(proxy2);
    series2->setMeshSmooth(true);
    //series2->setDrawMode(QSurface3DSeries::DrawSurface);

    // Открытие файла
    QFile file2(pathMap);
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file.");
        return -1;
    }

    // Чтение значений из файла
    QTextStream in2(&file2);
    QStringList latitudeValues = in2.readLine().split(" ");
    QStringList longitudeValues = in2.readLine().split(" ");
    double minLatitude = latitudeValues[0].toDouble();
    double maxLatitude = latitudeValues[1].toDouble();
    int latitudeSteps = latitudeValues[2].toInt();
    double minLongitude = longitudeValues[0].toDouble();
    double maxLongitude = longitudeValues[1].toDouble();
    int longitudeSteps = longitudeValues[2].toInt();

        //Создание массива данных
        QSurfaceDataArray *dataArray2 = new QSurfaceDataArray;
        dataArray2->reserve(latitudeSteps * longitudeSteps);

        // Чтение высот из файла и заполнение массива данных
        for (int i = 0; i < latitudeSteps; ++i) {
            QSurfaceDataRow *row3 = new QSurfaceDataRow(longitudeSteps);
            for (int j = 0; j < longitudeSteps; ++j) {
                double height;
                in2 >> height;
                double x = minLatitude + i * (maxLatitude - minLatitude) / (latitudeSteps - 1);
                double z = minLongitude + j * (maxLongitude - minLongitude) / (longitudeSteps - 1);
                (*row3)[j].setPosition(QVector3D(z, height, x));
            }
            dataArray2->append(row3);
        }

    proxy->resetArray(dataArray);
    proxy2->resetArray(dataArray2);

    //Отображение трехмерного графика с данными
    surface->addSeries(series);
    surface->addSeries(series2);

     //Отображение окна
     widget->show();

    return app.exec();
}
