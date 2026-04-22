#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scheduleitem.h"

#include <QDate>
#include <QList>
#include <QMainWindow>

class CalendarWidget;
class ScheduleListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleDateSelected(const QDate &date);
    void handleScheduleAdded(const ScheduleItem &item);
    void handleScheduleUpdated(const ScheduleItem &item);
    void handleScheduleDeleted(int id);
    void handleSearchRequested(const QString &keyword);
    void handleExportRequested(const QString &format);
    void handleImportRequested(bool overwrite);
    void handleSettingsRequested();
    void handleQuitRequested();
    void handleResetRequested();

private:
    void setupUi();
    void setupMenuBar();
    void setupConnections();
    void applyStyles();
    void refreshScheduleList();
    void refreshCalendarHighlights();
    QList<ScheduleItem> schedulesForSelectedDate() const;
    QList<ScheduleItem> schedulesMatchingKeyword(const QString &keyword) const;
    QList<QDate> scheduledDates() const;
    bool exportAsCsv(const QString &filePath) const;
    bool exportAsJson(const QString &filePath) const;
    bool importFromJson(const QString &filePath, bool overwrite = false, bool initialize = false);
    QString defaultStoragePath() const;
    bool saveToDefaultStorage() const;
    bool loadFromDefaultStorage(bool initialize = false);
    int generateScheduleId();


    ScheduleListWidget *m_scheduleListWidget;
    CalendarWidget *m_calendarWidget;
    QList<ScheduleItem> m_schedules;
    QDate m_selectedDate;
    QString m_searchKeyword;
    int m_nextScheduleId;
};

#endif // MAINWINDOW_H
