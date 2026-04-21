#include "mainwindow.h"
#include "scheduleitem.h"

#include <QApplication>
#include <QLocale>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        app.setStyleSheet(stream.readAll());
        file.close();
    }
    QLocale koreanLocale(QLocale::Korean, QLocale::SouthKorea);
    QLocale::setDefault(koreanLocale);
    qRegisterMetaType<ScheduleItem>("ScheduleItem");
    qRegisterMetaType<QList<ScheduleItem>>("QList<ScheduleItem>");

    MainWindow window;
    window.show();

    return app.exec();
}
