#include "mainwindow.h"

#include "calendarwidget.h"
#include "schedulelistwidget.h"
#include "toptoolbarwidget.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSaveFile>
#include <QSet>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>

namespace {

QString csvEscaped(const QString &value)
{
    QString escaped = value;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
}

bool scheduleLessThan(const ScheduleItem &left, const ScheduleItem &right)
{
    if (left.date != right.date) {
        return left.date < right.date;
    }

    if (left.time != right.time) {
        return left.time < right.time;
    }

    return left.id < right.id;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_topToolbarWidget(nullptr)
    , m_scheduleListWidget(nullptr)
    , m_calendarWidget(nullptr)
    , m_selectedDate(QDate::currentDate())
    , m_nextScheduleId(1)
{
    setupUi();
    setupConnections();
    applyStyles();
    refreshScheduleList();
    refreshCalendarHighlights();
}

void MainWindow::handleDateSelected(const QDate &date)
{
    m_selectedDate = date;
    refreshScheduleList();
}

void MainWindow::handleScheduleAdded(const ScheduleItem &item)
{
    ScheduleItem schedule = item;
    schedule.id = generateScheduleId();
    m_schedules.append(schedule);

    refreshScheduleList();
    refreshCalendarHighlights();
}

void MainWindow::handleScheduleUpdated(const ScheduleItem &item)
{
    for (ScheduleItem &schedule : m_schedules) {
        if (schedule.id == item.id) {
            schedule = item;
            refreshScheduleList();
            refreshCalendarHighlights();
            return;
        }
    }
}

void MainWindow::handleScheduleDeleted(int id)
{
    const auto newEnd = std::remove_if(m_schedules.begin(), m_schedules.end(),
                                       [id](const ScheduleItem &item) { return item.id == id; });
    if (newEnd == m_schedules.end()) {
        return;
    }

    m_schedules.erase(newEnd, m_schedules.end());
    refreshScheduleList();
    refreshCalendarHighlights();
}

void MainWindow::handleSearchRequested(const QString &keyword)
{
    m_searchKeyword = keyword.trimmed();
    refreshScheduleList();
}

void MainWindow::handleExportRequested(const QString &format)
{
    if (format.compare("csv", Qt::CaseInsensitive) == 0) {
        const QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("Export schedules"),
            QDir::homePath() + "/schedules.csv",
            tr("CSV Files (*.csv)"));

        if (filePath.isEmpty()) {
            return;
        }

        if (exportAsCsv(filePath)) {
            QMessageBox::information(this, tr("Export complete"), tr("Schedules were exported as CSV."));
        } else {
            QMessageBox::warning(this, tr("Export failed"), tr("Could not write the CSV file."));
        }
        return;
    }

    QMessageBox::information(
        this,
        tr("Export option"),
        tr("%1 export is reserved in the structure and can be implemented next.")
            .arg(format.toUpper()));
}

void MainWindow::handleImportRequested()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import schedules"),
        QDir::homePath(),
        tr("JSON Files (*.json)"));

    if (filePath.isEmpty()) {
        return;
    }

    if (importFromJson(filePath)) {
        refreshScheduleList();
        refreshCalendarHighlights();
        QMessageBox::information(this, tr("Import complete"), tr("Schedules were imported from JSON."));
    } else {
        QMessageBox::warning(this, tr("Import failed"), tr("The selected JSON file could not be imported."));
    }
}

void MainWindow::handleSettingsRequested()
{
    QMessageBox::information(
        this,
        tr("Settings"),
        tr("Settings UI is intentionally left as a dedicated extension point."));
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("Schedule Management"));
    resize(1240, 760);

    auto *central = new QWidget(this);
    central->setObjectName("AppBackground");
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    m_topToolbarWidget = new TopToolbarWidget(central);
    m_scheduleListWidget = new ScheduleListWidget(central);
    m_calendarWidget = new CalendarWidget(central);

    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(18);
    contentLayout->addWidget(m_scheduleListWidget, 0);
    contentLayout->addWidget(m_calendarWidget, 1);

    m_scheduleListWidget->setFixedWidth(300);
    m_calendarWidget->setMinimumWidth(500);

    mainLayout->addWidget(m_topToolbarWidget);
    mainLayout->addLayout(contentLayout, 1);
}

void MainWindow::setupConnections()
{
    connect(m_topToolbarWidget, &TopToolbarWidget::searchRequested,
            this, &MainWindow::handleSearchRequested);
    connect(m_topToolbarWidget, &TopToolbarWidget::exportRequested,
            this, &MainWindow::handleExportRequested);
    connect(m_topToolbarWidget, &TopToolbarWidget::importRequested,
            this, &MainWindow::handleImportRequested);
    connect(m_topToolbarWidget, &TopToolbarWidget::settingsRequested,
            this, &MainWindow::handleSettingsRequested);

    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleAdded,
            this, &MainWindow::handleScheduleAdded);
    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleUpdated,
            this, &MainWindow::handleScheduleUpdated);
    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleDeleted,
            this, &MainWindow::handleScheduleDeleted);

    connect(m_calendarWidget, &CalendarWidget::dateSelected,
            this, &MainWindow::handleDateSelected);
}

void MainWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget#AppBackground {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #f5f7fb, stop:1 #edf2f9);
        }
        QFrame[card="true"] {
            background-color: #ffffff;
            border: 1px solid #dbe3ee;
            border-radius: 18px;
        }
        QLabel#SectionTitle {
            color: #223046;
            font-size: 20px;
            font-weight: 700;
        }
        QLabel#SectionSubtitle {
            color: #6f7f95;
            font-size: 12px;
            font-weight: 600;
        }
        QLineEdit,
        QComboBox,
        QDateEdit,
        QTimeEdit,
        QPlainTextEdit {
            background: #f8fafc;
            border: 1px solid #d4deea;
            border-radius: 12px;
            padding: 10px 12px;
            color: #223046;
        }
        QLineEdit:focus,
        QComboBox:focus,
        QDateEdit:focus,
        QTimeEdit:focus,
        QPlainTextEdit:focus {
            border: 1px solid #4e89ff;
            background: #ffffff;
        }
        QPushButton {
            background: #eef4ff;
            border: 1px solid #d4deea;
            border-radius: 12px;
            color: #1f3d66;
            padding: 10px 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: #dbe9ff;
            border-color: #bfd4f8;
        }
        QPushButton#PrimaryButton {
            background: #2d6cf6;
            color: white;
            border-color: #2d6cf6;
        }
        QPushButton#PrimaryButton:hover {
            background: #1f5ce0;
            border-color: #1f5ce0;
        }
        QScrollArea {
            border: none;
            background: transparent;
        }
        QWidget#ScheduleRowCard {
            background: #f9fbfe;
            border: 1px solid #dce5f1;
            border-radius: 16px;
        }
        QWidget#ScheduleRowCard:hover {
            background: #f1f6ff;
            border-color: #bfd4f8;
        }
        QLabel#ScheduleTitle {
            color: #223046;
            font-size: 15px;
            font-weight: 700;
        }
        QLabel#ScheduleMeta,
        QLabel#ScheduleDescription {
            color: #6f7f95;
            font-size: 12px;
        }
        QCalendarWidget QWidget {
            alternate-background-color: #ffffff;
        }
        QCalendarWidget QToolButton {
            background: #eef4ff;
            border: 1px solid #d4deea;
            border-radius: 10px;
            color: #223046;
            padding: 6px 10px;
            margin: 2px;
        }
        QCalendarWidget QToolButton:hover {
            background: #dbe9ff;
        }
        QCalendarWidget QMenu {
            background: #ffffff;
            border: 1px solid #d4deea;
        }
        QCalendarWidget QSpinBox {
            background: #f8fafc;
            border: 1px solid #d4deea;
            border-radius: 8px;
            padding: 4px 8px;
        }
        QCalendarWidget QAbstractItemView {
            background: #ffffff;
            selection-background-color: #2d6cf6;
            selection-color: #ffffff;
            outline: 0;
            border: none;
        }
    )");
}

void MainWindow::refreshScheduleList()
{
    QList<ScheduleItem> visibleSchedules;

    if (m_searchKeyword.isEmpty()) {
        m_scheduleListWidget->setListTitle(QString());
        m_scheduleListWidget->setDate(m_selectedDate);
        visibleSchedules = schedulesForSelectedDate();
    } else {
        visibleSchedules = schedulesMatchingKeyword(m_searchKeyword);
        m_scheduleListWidget->setListTitle(
            tr("Search results for \"%1\" (%2)").arg(m_searchKeyword).arg(visibleSchedules.size()));
    }

    m_scheduleListWidget->updateScheduleList(visibleSchedules);
    m_calendarWidget->setSelectedDate(m_selectedDate);
}

void MainWindow::refreshCalendarHighlights()
{
    m_calendarWidget->setSchedules(m_schedules);
    m_calendarWidget->highlightDates(scheduledDates());
}

QList<ScheduleItem> MainWindow::schedulesForSelectedDate() const
{
    QList<ScheduleItem> matches;
    for (const ScheduleItem &item : m_schedules) {
        if (item.date == m_selectedDate) {
            matches.append(item);
        }
    }

    std::sort(matches.begin(), matches.end(), scheduleLessThan);
    return matches;
}

QList<ScheduleItem> MainWindow::schedulesMatchingKeyword(const QString &keyword) const
{
    QList<ScheduleItem> matches;
    const QString loweredKeyword = keyword.toLower();

    for (const ScheduleItem &item : m_schedules) {
        const QString haystack = QString("%1 %2")
                                     .arg(item.title, item.description)
                                     .toLower();
        if (haystack.contains(loweredKeyword)) {
            matches.append(item);
        }
    }

    std::sort(matches.begin(), matches.end(), scheduleLessThan);
    return matches;
}

QList<QDate> MainWindow::scheduledDates() const
{
    QSet<QDate> uniqueDates;
    for (const ScheduleItem &item : m_schedules) {
        uniqueDates.insert(item.date);
    }

    QList<QDate> dates = uniqueDates.values();
    std::sort(dates.begin(), dates.end());
    return dates;
}

bool MainWindow::exportAsCsv(const QString &filePath) const
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << "id,date,time,title,description\n";

    QList<ScheduleItem> schedules = m_schedules;
    std::sort(schedules.begin(), schedules.end(), scheduleLessThan);

    for (const ScheduleItem &item : schedules) {
        stream << item.id << ','
               << csvEscaped(item.date.toString(Qt::ISODate)) << ','
               << csvEscaped(item.time.toString("HH:mm")) << ','
               << csvEscaped(item.title) << ','
               << csvEscaped(item.description) << '\n';
    }

    return file.commit();
}

bool MainWindow::importFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || document.isNull()) {
        return false;
    }

    QJsonArray schedulesArray;
    if (document.isArray()) {
        schedulesArray = document.array();
    } else if (document.isObject()) {
        schedulesArray = document.object().value("schedules").toArray();
    } else {
        return false;
    }

    bool importedAny = false;

    for (const QJsonValue &value : schedulesArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        const QDate date = QDate::fromString(object.value("date").toString(), Qt::ISODate);
        QTime time = QTime::fromString(object.value("time").toString(), "HH:mm");
        if (!time.isValid()) {
            time = QTime::fromString(object.value("time").toString(), Qt::ISODate);
        }

        const QString title = object.value("title").toString().trimmed();
        if (!date.isValid() || !time.isValid() || title.isEmpty()) {
            continue;
        }

        ScheduleItem item;
        item.id = generateScheduleId();
        item.date = date;
        item.time = time;
        item.title = title;
        item.description = object.value("description").toString();

        m_schedules.append(item);
        importedAny = true;
    }

    return importedAny;
}

int MainWindow::generateScheduleId()
{
    return m_nextScheduleId++;
}
