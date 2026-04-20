#include "mainwindow.h"
#include "scheduleitem.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qRegisterMetaType<ScheduleItem>("ScheduleItem");
    qRegisterMetaType<QList<ScheduleItem>>("QList<ScheduleItem>");

    MainWindow window;
    window.show();

    return app.exec();
}
