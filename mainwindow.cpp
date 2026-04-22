#include "mainwindow.h"
#include "calendarwidget.h"
#include "schedulelistwidget.h"
#include "settingsdialog.h"
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
#include <QSettings>
#include <QApplication>
#include <QCloseEvent>
#include <QLabel>
#include <QIcon>
#include <QMovie>

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
    loadFromDefaultStorage(true);
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
            QMessageBox::warning(this, tr("내보내기 실패"), tr("CSV 파일 생성에 실패했습니다."));
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
            QMessageBox::warning(this, tr("내보내기 실패"), tr("JSON 파일 생성에 실패했습니다."));
        }
        return;
    }

    QMessageBox::information(
        this,
        tr("Export option"),
        tr("%1 export is reserved in the structure and can be implemented next.")
            .arg(format.toUpper()));
}

void MainWindow::handleImportRequested(bool overwrite)
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import schedules"),
        QDir::homePath(),
        tr("JSON Files (*.json)"));

    if (filePath.isEmpty()) {
        return;
    }

    if (importFromJson(filePath, overwrite)) {
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
    // 1. 설정값 불러오기
    QSettings settings("MyCompany", "ScheduleManagement");
    int currentMax = settings.value("calendar/maxVisible", 4).toInt();
    bool currentWarn = settings.value("data/warnOnEmptyImport", true).toBool();

    // 2. 다이얼로그 생성 및 현재 값 세팅
    SettingsDialog dlg(this);
    dlg.setMaxVisible(currentMax);
    dlg.setWarnOnEmpty(currentWarn);

    // 3. 실행 및 결과 반영
    if (dlg.exec() == QDialog::Accepted) {
        // 값 저장
        settings.setValue("calendar/maxVisible", dlg.maxVisible());
        settings.setValue("data/warnOnEmptyImport", dlg.warnOnEmpty());

        // 즉시 반영이 필요한 경우 알림
        QMessageBox::information(this, tr("설정 저장"), tr("설정이 저장되었습니다.\n일부 설정은 재시작 후 적용될 수 있습니다."));

        // UI 즉시 갱신이 필요하다면 호출
        refreshCalendarHighlights();
    }
}

void MainWindow::setupUi()
{

    setWindowIcon(QIcon(":/resources/calander.gif"));
    setWindowTitle(tr("스케줄러"));
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
    QMenu *moreMenu = menuBar()->addMenu("더보기");

    QAction *settingsAction = settingsMenu->addAction(tr("환경설정"));
    QAction *resetAction = settingsMenu->addAction(tr("일정 초기화"));
    QAction *importOverwriteAction = importMenu->addAction(tr("일정 덮어쓰기"));
    QAction *importAppendAction = importMenu->addAction(tr("일정 추가하기"));
    QAction *exportJsonAction = exportMenu->addAction(tr("JSON으로 내보내기"));
    QAction *exportCsvAction = exportMenu->addAction(tr("CSV로 내보내기"));
    QAction *quitApp = moreMenu->addAction(tr("종료"));

    // 종료 액션 연결 (애플리케이션 종료)
    connect(quitApp, &QAction::triggered, this, &MainWindow::handleQuitRequested);
    connect(resetAction, &QAction::triggered, this, &MainWindow::handleResetRequested);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::handleSettingsRequested);
    connect(importOverwriteAction, &QAction::triggered, this, [this]() {
        handleImportRequested(true);
    });
    connect(importAppendAction, &QAction::triggered, this, [this]() {
        handleImportRequested(false);
    });
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
    QFile file(":/style.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        this->setStyleSheet(stream.readAll());
        file.close();
    } else {
        qDebug() << "스타일 시트 파일을 열 수 없습니다!";
    }
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

bool MainWindow::importFromJson(const QString &filePath, bool overwrite, bool initialize)
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

    if (schedulesArray.isEmpty()) {
        QSettings settings("MyCompany", "ScheduleManagement");
        bool shouldWarn = settings.value("data/warnOnEmptyImport", true).toBool();
        if (settings.value("data/warnOnEmptyImport", true).toBool() && !initialize) {
            QMessageBox::warning(this, tr("가져오기 경고"), tr("가져올 일정 데이터가 없습니다."));
        }
        return false;
    }

    if (overwrite) {
        m_schedules.clear();
        m_nextScheduleId = 1;
    }

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
    }

    return true;
}

QString MainWindow::defaultStoragePath() const
{
    return defaultSchedulesFilePath();
}

bool MainWindow::saveToDefaultStorage() const
{
    return exportAsJson(defaultStoragePath());
}

bool MainWindow::loadFromDefaultStorage(bool initialize)
{
    const QString filePath = defaultStoragePath();
    if (!QFile::exists(filePath)) {
        return true;
    }

    return importFromJson(filePath, false, initialize);
}

int MainWindow::generateScheduleId()
{
    return m_nextScheduleId++;
}
void MainWindow::handleQuitRequested()
{
    this->close();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("종료 확인"),
                                  tr("프로그램을 종료하시겠습니까?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}
void MainWindow::handleResetRequested()
{
    // 1. 사용자 확인 (실수 방지)
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("일정 초기화"),
                                  tr("모든 일정이 영구적으로 삭제됩니다. 계속하시겠습니까?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 2. 메모리 데이터 비우기
        m_schedules.clear();
        m_nextScheduleId = 1; // ID도 초기화

        // 3. 파일 저장 (빈 상태로 덮어쓰기)
        saveToDefaultStorage();

        // 4. UI 갱신
        refreshScheduleList();
        refreshCalendarHighlights();

        QMessageBox::information(this, tr("완료"), tr("모든 일정이 초기화되었습니다."));
    }
}
