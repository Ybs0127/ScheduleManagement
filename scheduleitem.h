#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

#include <QDate>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QTime>

struct ScheduleItem {
    int id = -1;
    QDate date;
    QTime time;
    QString title;
    QString description;
};

Q_DECLARE_METATYPE(ScheduleItem)
Q_DECLARE_METATYPE(QList<ScheduleItem>)

#endif // SCHEDULEITEM_H
