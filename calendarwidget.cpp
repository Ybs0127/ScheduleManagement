#include "calendarwidget.h"

#include <QCalendarWidget>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QSignalBlocker>
#include <QTextCharFormat>
#include <QVBoxLayout>
#include <QFont>
#include <utility>

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
    , m_monthCombo(nullptr)
    , m_yearCombo(nullptr)
    , m_calendar(nullptr)
{
    setupUi();
    syncSelectorsWithCalendar();
}

void CalendarWidget::setSelectedDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    const QSignalBlocker blocker(m_calendar);
    m_calendar->setSelectedDate(date);
    m_calendar->setCurrentPage(date.year(), date.month());
    syncSelectorsWithCalendar();
}

void CalendarWidget::highlightDates(const QList<QDate> &dates)
{
    for (const QDate &date : std::as_const(m_highlightedDates)) {
        m_calendar->setDateTextFormat(date, QTextCharFormat());
    }

    m_highlightedDates = dates;

    QTextCharFormat format;
    format.setBackground(QColor("#dbe9ff"));
    format.setForeground(QColor("#16345f"));
    format.setFontWeight(QFont::DemiBold);

    for (const QDate &date : std::as_const(m_highlightedDates)) {
        m_calendar->setDateTextFormat(date, format);
    }
}

void CalendarWidget::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(this);
    card->setProperty("card", true);
    outerLayout->addWidget(card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(16);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    auto *titleBlock = new QVBoxLayout();
    titleBlock->setSpacing(2);

    auto *titleLabel = new QLabel(tr("Calendar"), card);
    titleLabel->setObjectName("SectionTitle");

    auto *subtitleLabel = new QLabel(tr("Select a date and track which days already have plans."), card);
    subtitleLabel->setObjectName("SectionSubtitle");

    titleBlock->addWidget(titleLabel);
    titleBlock->addWidget(subtitleLabel);

    m_monthCombo = new QComboBox(card);
    for (int month = 1; month <= 12; ++month) {
        m_monthCombo->addItem(QLocale().monthName(month, QLocale::LongFormat), month);
    }

    m_yearCombo = new QComboBox(card);
    for (int year = 1990; year <= 2100; ++year) {
        m_yearCombo->addItem(QString::number(year), year);
    }

    headerLayout->addLayout(titleBlock, 1);
    headerLayout->addWidget(m_monthCombo);
    headerLayout->addWidget(m_yearCombo);

    m_calendar = new QCalendarWidget(card);
    m_calendar->setGridVisible(false);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setSelectedDate(QDate::currentDate());
    m_calendar->setMinimumHeight(520);

    layout->addLayout(headerLayout);
    layout->addWidget(m_calendar, 1);

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        syncSelectorsWithCalendar();
        emit dateSelected(m_calendar->selectedDate());
    });
    connect(m_calendar, &QCalendarWidget::currentPageChanged, this, [this]() {
        syncSelectorsWithCalendar();
    });
    connect(m_monthCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateCalendarPage();
    });
    connect(m_yearCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateCalendarPage();
    });
}

void CalendarWidget::syncSelectorsWithCalendar()
{
    const QSignalBlocker monthBlocker(m_monthCombo);
    const QSignalBlocker yearBlocker(m_yearCombo);

    const int monthIndex = m_monthCombo->findData(m_calendar->monthShown());
    const int yearIndex = m_yearCombo->findData(m_calendar->yearShown());

    if (monthIndex >= 0) {
        m_monthCombo->setCurrentIndex(monthIndex);
    }

    if (yearIndex >= 0) {
        m_yearCombo->setCurrentIndex(yearIndex);
    }
}

void CalendarWidget::updateCalendarPage()
{
    const int month = m_monthCombo->currentData().toInt();
    const int year = m_yearCombo->currentData().toInt();

    if (month <= 0 || year <= 0) {
        return;
    }

    m_calendar->setCurrentPage(year, month);
}
