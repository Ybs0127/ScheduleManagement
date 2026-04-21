#include "calendarwidget.h"

#include <QCalendarWidget>
#include <QColor>
#include <QDate>
#include <QFrame>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLayoutItem>
#include <QPainter>
#include <QSignalBlocker>
#include <QTextCharFormat>
#include <QToolButton>
#include <QVBoxLayout>
#include <QFont>
#include <algorithm>
#include <utility>

namespace {

bool scheduleItemLessThan(const ScheduleItem &left, const ScheduleItem &right)
{
    if (left.time != right.time) {
        return left.time < right.time;
    }

    return left.title.toLower() < right.title.toLower();
}

void arrangeCalendarNavigation(QCalendarWidget *calendar)
{
    if (!calendar) {
        return;
    }

    QWidget *navigationBar = calendar->findChild<QWidget *>(QStringLiteral("qt_calendar_navigationbar"));
    auto *navigationLayout = navigationBar ? qobject_cast<QHBoxLayout *>(navigationBar->layout()) : nullptr;
    auto *prevButton = calendar->findChild<QToolButton *>(QStringLiteral("qt_calendar_prevmonth"));
    auto *nextButton = calendar->findChild<QToolButton *>(QStringLiteral("qt_calendar_nextmonth"));
    auto *monthButton = calendar->findChild<QToolButton *>(QStringLiteral("qt_calendar_monthbutton"));
    auto *yearButton = calendar->findChild<QToolButton *>(QStringLiteral("qt_calendar_yearbutton"));

    if (!navigationLayout || !prevButton || !nextButton || !monthButton || !yearButton) {
        return;
    }

    yearButton->setText(QStringLiteral("%1년").arg(calendar->yearShown()));

    while (QLayoutItem *item = navigationLayout->takeAt(0)) {
        delete item;
    }

    navigationLayout->setContentsMargins(0, 0, 0, 0);
    navigationLayout->setSpacing(6);
    navigationLayout->addStretch();
    navigationLayout->addWidget(prevButton);
    navigationLayout->addWidget(yearButton);
    navigationLayout->addWidget(monthButton);
    navigationLayout->addWidget(nextButton);
    navigationLayout->addStretch();
}

// 하이라이트된 달력 셀이 어떻게 렌더링하는지
class ScheduleCalendarView : public QCalendarWidget
{
public:
    explicit ScheduleCalendarView(QWidget *parent = nullptr)
        : QCalendarWidget(parent)
    {
    }

    void setSchedules(const QList<ScheduleItem> &items)
    {
        m_schedulesByDate.clear();

        for (const ScheduleItem &item : items)
            m_schedulesByDate[item.date].append(item);

        for (auto it = m_schedulesByDate.begin(); it != m_schedulesByDate.end(); ++it)
            std::sort(it.value().begin(), it.value().end(), scheduleItemLessThan);
        updateCells();
    }

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override
    {
        QCalendarWidget::paintCell(painter, rect, date);

        const QList<ScheduleItem> schedules = m_schedulesByDate.value(date);
        if (schedules.isEmpty()) {
            return;
        }

        painter->save();
        painter->setClipRect(rect.adjusted(2, 12, -2, -2));

        QFont textFont = painter->font();
        textFont.setPointSize(7);
        painter->setFont(textFont);
        painter->setPen(date == selectedDate() ? QColor("#ffffff") : QColor("#1f3d66"));

        int y = rect.top() + 12;
        const int left = rect.left() + 4;
        const int width = rect.width() - 8;
        const int maxVisible = 4;

        for (int index = 0; index < schedules.size() && index < maxVisible; ++index) {
            const QString text = QString("%1 %2")
                                     .arg(schedules.at(index).time.toString("HH:mm"),
                                          schedules.at(index).title);
            const QString elided = painter->fontMetrics().elidedText(text, Qt::ElideRight, width);
            painter->drawText(QRect(left, y, width, 10), Qt::AlignLeft | Qt::AlignVCenter, elided);
            y += 11;
        }

        if (schedules.size() > maxVisible) {
            painter->drawText(QRect(left, y, width, 10),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              QString("+%1").arg(schedules.size() - maxVisible));
        }

        painter->restore();
    }

private:
    QHash<QDate, QList<ScheduleItem>> m_schedulesByDate;
};

} // namespace

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
    , m_calendar(nullptr)
    , m_searchEdit(nullptr)
{
    setupUi();
}

void CalendarWidget::setSelectedDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    const QSignalBlocker blocker(m_calendar);
    m_calendar->setSelectedDate(date);
    m_calendar->setCurrentPage(date.year(), date.month());
    arrangeCalendarNavigation(m_calendar);
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
void CalendarWidget::setSchedules(const QList<ScheduleItem> &items)
{
    static_cast<ScheduleCalendarView *>(m_calendar)->setSchedules(items);
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

    auto *titleLabel = new QLabel(tr("달력"), card);
    titleLabel->setObjectName("SectionTitle");

    m_searchEdit = new QLineEdit(card);
    m_searchEdit->setPlaceholderText(tr("일정 검색"));
    QAction *searchIconAction = m_searchEdit->addAction(QIcon::fromTheme("edit-find"),
                                                        QLineEdit::LeadingPosition);
    searchIconAction->setEnabled(true);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaximumWidth(600);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_searchEdit, 0, Qt::AlignRight);

    m_calendar = new ScheduleCalendarView(card);
    m_calendar->setGridVisible(false);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setSelectedDate(QDate::currentDate());
    m_calendar->setMinimumHeight(520);
    arrangeCalendarNavigation(m_calendar);
    connect(m_calendar, &QCalendarWidget::currentPageChanged, this,
            [this](int, int) { arrangeCalendarNavigation(m_calendar); });

    layout->addLayout(headerLayout);
    layout->addWidget(m_calendar, 1);

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        emit dateSelected(m_calendar->selectedDate());
    });
    connect(m_searchEdit, &QLineEdit::textChanged, this, &CalendarWidget::searchRequested);
}
