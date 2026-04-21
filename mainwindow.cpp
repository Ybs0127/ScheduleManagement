#include "mainwindow.h"

#include "calendarwidget.h"
#include "schedulelistwidget.h"

#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
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

QString defaultSchedulesFilePath()
{
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataPath.isEmpty()) {
        return QDir::home().filePath(".schedulemanagement/schedules.json");
    }
    return QDir(appDataPath).filePath("schedules.json");
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_scheduleListWidget(nullptr)
    , m_calendarWidget(nullptr)
    , m_selectedDate(QDate::currentDate())
    , m_nextScheduleId(1)
{
    setupUi();
    setupMenuBar();
    setupConnections();
    applyStyles();
    loadFromDefaultStorage();
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
    saveToDefaultStorage();

    refreshScheduleList();
    refreshCalendarHighlights();
}

void MainWindow::handleScheduleUpdated(const ScheduleItem &item)
{
    for (ScheduleItem &schedule : m_schedules) {
        if (schedule.id == item.id) {
            schedule = item;
            saveToDefaultStorage();
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
    saveToDefaultStorage();
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
            tr("일정 내보내기"),
            QDir::homePath() + "/schedules.csv",
            tr("CSV Files (*.csv)"));

        if (filePath.isEmpty()) {
            return;
        }

        if (exportAsCsv(filePath)) {
            QMessageBox::information(this, tr("내보내기 완료!"), tr("일정을 CSV 파일로 내보냈습니다."));
        } else {
            QMessageBox::warning(this, tr("내보내기 실패ㅠㅠ"), tr("CSV 파일 생성에 실패했습니다."));
        }
        return;
    }
    else if (format.compare("json", Qt::CaseInsensitive) == 0) {
        const QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("일정 내보내기"),
            QDir::homePath() + "/schedules.json",
            tr("JSON Files (*.json)"));

        if (filePath.isEmpty()) {
            return;
        }

        if(exportAsJson(filePath)){
            QMessageBox::information(this, tr("내보내기 완료!"), tr("일정을 JSON 파일로 내보냈습니다."));
        } else {
            QMessageBox::warning(this, tr("내보내기 실패ㅠㅠ"), tr("JSON 파일 생성에 실패했습니다."));
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
        saveToDefaultStorage();
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

    m_scheduleListWidget = new ScheduleListWidget(central);
    m_calendarWidget = new CalendarWidget(central);

    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(18);
    contentLayout->addWidget(m_scheduleListWidget, 0);
    contentLayout->addWidget(m_calendarWidget, 1);

    m_scheduleListWidget->setFixedWidth(300);
    m_calendarWidget->setMinimumWidth(500);

    mainLayout->addLayout(contentLayout, 1);
}

void MainWindow::setupMenuBar()
{
    menuBar()->clear();

    QMenu *settingsMenu = menuBar()->addMenu(tr("설정"));
    QMenu *importMenu = menuBar()->addMenu(tr("불러오기"));
    QMenu *exportMenu = menuBar()->addMenu(tr("내보내기"));

    QAction *settingsAction = settingsMenu->addAction(tr("환경설정"));
    QAction *importAction = importMenu->addAction(tr("JSON 불러오기"));
    QAction *exportCsvAction = exportMenu->addAction(tr("CSV로 내보내기"));
    QAction *exportJsonAction = exportMenu->addAction(tr("json으로 내보내기"));

    connect(settingsAction, &QAction::triggered, this, &MainWindow::handleSettingsRequested);
    connect(importAction, &QAction::triggered, this, &MainWindow::handleImportRequested);
    connect(exportCsvAction, &QAction::triggered, this, [this]() {
        handleExportRequested(QStringLiteral("csv"));
    });
    connect(exportJsonAction, &QAction::triggered, this, [this]() {
        handleExportRequested(QStringLiteral("json"));
    });
}

void MainWindow::setupConnections()
{
    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleAdded,
            this, &MainWindow::handleScheduleAdded);
    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleUpdated,
            this, &MainWindow::handleScheduleUpdated);
    connect(m_scheduleListWidget, &ScheduleListWidget::scheduleDeleted,
            this, &MainWindow::handleScheduleDeleted);

    connect(m_calendarWidget, &CalendarWidget::dateSelected,
            this, &MainWindow::handleDateSelected);
    connect(m_calendarWidget, &CalendarWidget::searchRequested,
            this, &MainWindow::handleSearchRequested);
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
            background: transparent;
            border: 1px solid transparent;
            border-radius: 8px;
            color: #223046;
            padding: 6px 10px;
            margin: 2px 12px;
        }
        QCalendarWidget QToolButton:hover {
            background: #dbe9ff;
            border: 1px solid #bfd4f8;
        }
        QCalendarWidget QToolButton:menu-indicator {
            image: none;
            background: transparent;
            subcontrol-position: right center;
            subcontrol-origin: padding;
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
            tr("\"%1에 대한 검색 결과").arg(m_searchKeyword));
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
    stream << "id,date,time,endDateTime,title,description\n";

    QList<ScheduleItem> schedules = m_schedules;
    std::sort(schedules.begin(), schedules.end(), scheduleLessThan);

    for (const ScheduleItem &item : schedules) {
        stream << item.id << ','
               << csvEscaped(item.date.toString(Qt::ISODate)) << ','
               << csvEscaped(item.time.toString("HH:mm")) << ','
               << csvEscaped(item.endDateTime.toString("yyyy-MM-dd HH:mm")) << ','
               << csvEscaped(item.title) << ','
               << csvEscaped(item.description) << '\n';
    }

    return file.commit();
}

bool MainWindow::exportAsJson(const QString &filePath) const
{
    const QString directoryPath = QFileInfo(filePath).absolutePath();
    if (!directoryPath.isEmpty() && !QDir().mkpath(directoryPath)) {
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QList<ScheduleItem> schedules = m_schedules;
    std::sort(schedules.begin(), schedules.end(), scheduleLessThan);

    QJsonArray schedulesArray;
    for (const ScheduleItem &item : schedules) {
        QJsonObject scheduleObject;
        scheduleObject.insert("id", item.id);
        scheduleObject.insert("date", item.date.toString(Qt::ISODate));
        scheduleObject.insert("time", item.time.toString("HH:mm"));
        scheduleObject.insert("endDateTime", item.endDateTime.toString("yyyy-MM-dd HH:mm"));
        scheduleObject.insert("title", item.title);
        scheduleObject.insert("description", item.description);
        schedulesArray.append(scheduleObject);
    }

    QJsonObject rootObject;
    rootObject.insert("schedules", schedulesArray);

    const QJsonDocument document(rootObject);
    if (file.write(document.toJson(QJsonDocument::Indented)) == -1) {
        return false;
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
        const QJsonValue schedulesValue = document.object().value("schedules");
        if (!schedulesValue.isArray()) {
            return false;
        }
        schedulesArray = schedulesValue.toArray();
    } else {
        return false;
    }

    bool importedAny = false; // 비어 있는 일정 파일은 업로드가 안 됨!

    for (const QJsonValue &value : schedulesArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        const QDate date = QDate::fromString(object.value("date").toString(), Qt::ISODate);
        QTime time = QTime::fromString(object.value("time").toString(), "HH:mm");
        if (!time.isValid()) time = QTime(0, 0);

        QDateTime endDateTime = QDateTime::fromString(object.value("endDateTime").toString(), "yyyy-MM-dd HH:mm");
        if (!endDateTime.isValid()) endDateTime = QDateTime(date, time.addSecs(3600));

        const QString title = object.value("title").toString().trimmed();
        if (!date.isValid() || title.isEmpty()) {
            continue;
        }

        ScheduleItem item;
        item.id = generateScheduleId();
        item.date = date;
        item.time = time;
        item.endDateTime = endDateTime;
        item.title = title;
        item.description = object.value("description").toString();

        m_schedules.append(item);
        importedAny = true;
    }

    return importedAny || schedulesArray.isEmpty();
}

QString MainWindow::defaultStoragePath() const
{
    return defaultSchedulesFilePath();
}

bool MainWindow::saveToDefaultStorage() const
{
    return exportAsJson(defaultStoragePath());
}

bool MainWindow::loadFromDefaultStorage()
{
    const QString filePath = defaultStoragePath();
    if (!QFile::exists(filePath)) {
        return true;
    }

    return importFromJson(filePath);
}

int MainWindow::generateScheduleId()
{
    return m_nextScheduleId++;
}
