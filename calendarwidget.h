#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "scheduleitem.h"

#include <QDate>
#include <QString>
#include <QWidget>

class QCalendarWidget;
class QLineEdit;

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    void setSelectedDate(const QDate &date);

signals:
    void dateSelected(const QDate &date);
    void searchRequested(const QString &keyword);

public slots:
    void highlightDates(const QList<QDate> &dates);
    void setSchedules(const QList<ScheduleItem> &items);

private:
    void setupUi();

    QCalendarWidget *m_calendar;
    QLineEdit *m_searchEdit;
    QList<QDate> m_highlightedDates;
};

#endif // CALENDARWIDGET_H
