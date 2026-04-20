#include "mainwindow.h"
#include "scheduleitem.h"

#include <QApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLocale koreanLocale(QLocale::Korean, QLocale::SouthKorea);
    QLocale::setDefault(koreanLocale);
    qRegisterMetaType<ScheduleItem>("ScheduleItem");
    qRegisterMetaType<QList<ScheduleItem>>("QList<ScheduleItem>");

    MainWindow window;
    window.show();

    return app.exec();
}
