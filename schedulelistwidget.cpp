#include "schedulelistwidget.h"

#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <utility>

namespace {

class ScheduleEditorDialog : public QDialog
{
public:
    explicit ScheduleEditorDialog(const ScheduleItem &initialItem, QWidget *parent = nullptr)
        : QDialog(parent)
        , m_item(initialItem)
    {
        setWindowTitle(m_item.id < 0 ? tr("일정 추가") : tr("일정 수정"));
        resize(420, 280);

        auto *layout = new QVBoxLayout(this);

        auto *formLayout = new QFormLayout();
        formLayout->setLabelAlignment(Qt::AlignLeft);
        formLayout->setFormAlignment(Qt::AlignTop);

        m_dateEdit = new QDateEdit(m_item.date.isValid() ? m_item.date : QDate::currentDate(), this);
        m_dateEdit->setCalendarPopup(true);
        m_dateEdit->setDisplayFormat("yyyy-MM-dd");

        m_timeEdit = new QTimeEdit(m_item.time.isValid() ? m_item.time : QTime::currentTime(), this);
        m_timeEdit->setDisplayFormat("HH:mm");

        m_titleEdit = new QLineEdit(m_item.title, this);
        m_descriptionEdit = new QPlainTextEdit(m_item.description, this);
        m_descriptionEdit->setPlaceholderText(tr("Optional notes"));
        m_descriptionEdit->setFixedHeight(96);

        formLayout->addRow(tr("날짜"), m_dateEdit);
        formLayout->addRow(tr("시작 시각"), m_timeEdit);
        formLayout->addRow(tr("제목"), m_titleEdit);
        formLayout->addRow(tr("메모"), m_descriptionEdit);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Save)->setText(tr("저장"));
        buttons->button(QDialogButtonBox::Cancel)->setText(tr("취소"));


        layout->addLayout(formLayout);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            if (m_titleEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, tr("제목이 없습니다."), tr("일정 제목을 입력해주세요."));
                return;
            }

            m_item.date = m_dateEdit->date();
            m_item.time = m_timeEdit->time();
            m_item.title = m_titleEdit->text().trimmed();
            m_item.description = m_descriptionEdit->toPlainText().trimmed();
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    ScheduleItem item() const
    {
        return m_item;
    }

private:
    ScheduleItem m_item;
    QDateEdit *m_dateEdit;
    QTimeEdit *m_timeEdit;
    QLineEdit *m_titleEdit;
    QPlainTextEdit *m_descriptionEdit;
};

} // namespace

ScheduleListWidget::ScheduleListWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_listContainer(nullptr)
    , m_listLayout(nullptr)
    , m_addButton(nullptr)
    , m_currentDate(QDate::currentDate())
{
    setupUi();
    setDate(m_currentDate);
}

void ScheduleListWidget::setListTitle(const QString &title)
{
    m_customTitle = title;
    m_titleLabel->setText(m_customTitle.isEmpty() ? defaultDateTitle() : m_customTitle);
}

void ScheduleListWidget::setDate(const QDate &date)
{
    m_currentDate = date;
    if (m_customTitle.isEmpty()) {
        m_titleLabel->setText(defaultDateTitle());
    }
}

void ScheduleListWidget::updateScheduleList(const QList<ScheduleItem> &items)
{
    m_items = items;
    rebuildScheduleItems();
}

void ScheduleListWidget::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(this);
    card->setProperty("card", true);
    outerLayout->addWidget(card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);

    m_titleLabel = new QLabel(card);
    m_titleLabel->setObjectName("SectionTitle");


    auto *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);

    m_listContainer = new QWidget(scrollArea);
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(10);
    m_listLayout->addStretch();

    scrollArea->setWidget(m_listContainer);

    m_addButton = new QPushButton(tr("+ 일정 추가"), card);
    m_addButton->setObjectName("PrimaryButton");

    layout->addWidget(m_titleLabel);
    layout->addWidget(scrollArea, 1);
    layout->addWidget(m_addButton);

    connect(m_addButton, &QPushButton::clicked, this, &ScheduleListWidget::openCreateDialog);
}

void ScheduleListWidget::rebuildScheduleItems()
{
    clearListLayout();

    if (m_items.isEmpty()) {
        auto *emptyState = new QLabel(tr("현재 일정이 없습니다."), m_listContainer);
        emptyState->setObjectName("SectionSubtitle");
        emptyState->setWordWrap(true);
        m_listLayout->addWidget(emptyState);
        m_listLayout->addStretch();
        return;
    }

    for (const ScheduleItem &item : std::as_const(m_items)) {
        auto *card = new QWidget(m_listContainer);
        card->setObjectName("ScheduleRowCard");

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(10);

        auto *topRow = new QHBoxLayout();
        topRow->setSpacing(8);

        auto *metaLabel = new QLabel(
            QString("%1  %2").arg(item.date.toString("yyyy-MM-dd"), item.time.toString("HH:mm")),
            card);
        metaLabel->setObjectName("ScheduleMeta");

        auto *editButton = new QPushButton(tr("수정"), card);
        auto *deleteButton = new QPushButton(tr("삭제"), card);

        topRow->addWidget(metaLabel, 1);
        topRow->addWidget(editButton);
        topRow->addWidget(deleteButton);

        auto *titleLabel = new QLabel(item.title, card);
        titleLabel->setObjectName("ScheduleTitle");
        titleLabel->setWordWrap(true);

        auto *descriptionLabel = new QLabel(
            item.description.isEmpty() ? tr("메모 없음") : item.description,
            card);
        descriptionLabel->setObjectName("ScheduleDescription");
        descriptionLabel->setWordWrap(true);

        cardLayout->addLayout(topRow);
        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(descriptionLabel);

        connect(editButton, &QPushButton::clicked, this, [this, item]() {
            openEditDialog(item);
        });
        connect(deleteButton, &QPushButton::clicked, this, [this, item]() {
            const auto answer = QMessageBox::question(
                this,
                tr("일정 삭제"),
                tr("%1 %2에 있는 \"%3\" 일정을 삭제하시겠습니까?").arg(item.date.toString("yyyy-MM-dd"), item.time.toString("HH:mm"), item.title));
            if (answer == QMessageBox::Yes) {
                emit scheduleDeleted(item.id);
            }
        });

        m_listLayout->addWidget(card);
    }

    m_listLayout->addStretch();
}

void ScheduleListWidget::openCreateDialog()
{
    ScheduleItem draft;
    draft.date = m_currentDate.isValid() ? m_currentDate : QDate::currentDate();
    draft.time = QTime::currentTime();

    ScheduleEditorDialog dialog(draft, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit scheduleAdded(dialog.item());
    }
}

void ScheduleListWidget::openEditDialog(const ScheduleItem &item)
{
    ScheduleEditorDialog dialog(item, this);
    if (dialog.exec() == QDialog::Accepted) {
        emit scheduleUpdated(dialog.item());
    }
}

void ScheduleListWidget::clearListLayout()
{
    while (QLayoutItem *item = m_listLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

QString ScheduleListWidget::defaultDateTitle() const
{
    return m_currentDate.isValid()
        ? tr("%1 일정").arg(QLocale().toString(m_currentDate, "M월 d일 dddd"))
        : tr("Schedules");
}
