#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QDate>
#include <QWidget>

class QCalendarWidget;
class QComboBox;

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

private:
    void setupUi();
    void syncSelectorsWithCalendar();
    void updateCalendarPage();

    QComboBox *m_monthCombo;
    QComboBox *m_yearCombo;
    QCalendarWidget *m_calendar;
    QList<QDate> m_highlightedDates;
};

#endif // CALENDARWIDGET_H
