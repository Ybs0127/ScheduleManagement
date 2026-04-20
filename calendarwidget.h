#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "scheduleitem.h"

#include <QDate>
#include <QWidget>

class QCalendarWidget;

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    void setSelectedDate(const QDate &date);

signals:
    void dateSelected(const QDate &date);

public slots:
    void highlightDates(const QList<QDate> &dates);
    void setSchedules(const QList<ScheduleItem> &items);

private:
    void setupUi();

    QCalendarWidget *m_calendar;
    QList<QDate> m_highlightedDates;
};

#endif // CALENDARWIDGET_H
