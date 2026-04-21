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
        resize(450, 350);

        auto *layout = new QVBoxLayout(this);
        auto *formLayout = new QFormLayout();

        // --- 시작 일시 (날짜 + 시간) ---
        // 기존 item.date와 item.time을 합쳐서 QDateTime 생성
        QDateTime startDT(m_item.date, m_item.time);
        if (!startDT.isValid()) startDT = QDateTime::currentDateTime();

        m_startDateTimeEdit = new QDateTimeEdit(startDT, this);
        m_startDateTimeEdit->setCalendarPopup(true); // 달력 팝업 사용
        m_startDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        // --- 종료 일시 (날짜 + 시간) ---
        // 기본값으로 시작 일시로부터 1시간 후 설정
        m_endDateTimeEdit = new QDateTimeEdit(startDT.addSecs(3600), this);
        m_endDateTimeEdit->setCalendarPopup(true);
        m_endDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        m_titleEdit = new QLineEdit(m_item.title, this);
        m_descriptionEdit = new QPlainTextEdit(m_item.description, this);
        m_descriptionEdit->setFixedHeight(80);

        // 폼 레이아웃에 추가
        formLayout->addRow(tr("시작 일시"), m_startDateTimeEdit);
        formLayout->addRow(tr("종료 일시"), m_endDateTimeEdit);
        formLayout->addRow(tr("제목"), m_titleEdit);
        formLayout->addRow(tr("메모"), m_descriptionEdit);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        layout->addLayout(formLayout);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            if (m_titleEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, tr("알림"), tr("제목을 입력해주세요."));
                return;
            }

            // 1. 시작 일시 저장
            m_item.date = m_startDateTimeEdit->date();
            m_item.time = m_startDateTimeEdit->time();

            // 2. 종료 일시 저장 (추가된 부분)
            m_item.endDateTime = m_endDateTimeEdit->dateTime();

            // 3. 기타 정보 저장
            m_item.title = m_titleEdit->text().trimmed();
            m_item.description = m_descriptionEdit->toPlainText().trimmed();

            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    ScheduleItem item() const { return m_item; }

private:
    ScheduleItem m_item;
    QDateTimeEdit *m_startDateTimeEdit;
    QDateTimeEdit *m_endDateTimeEdit;
    QLineEdit *m_titleEdit;
    QPlainTextEdit *m_descriptionEdit;
};

} // namespace

ScheduleListWidget::ScheduleListWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_subtitleLabel(nullptr)
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

    m_subtitleLabel = new QLabel(tr("Your day list updates immediately when plans change."), card);
    m_subtitleLabel->setObjectName("SectionSubtitle");

    auto *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);

    m_listContainer = new QWidget(scrollArea);
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(10);
    m_listLayout->addStretch();

    scrollArea->setWidget(m_listContainer);

    m_addButton = new QPushButton(tr("+ Manual Add"), card);
    m_addButton->setObjectName("PrimaryButton");

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_subtitleLabel);
    layout->addWidget(scrollArea, 1);
    layout->addWidget(m_addButton);

    connect(m_addButton, &QPushButton::clicked, this, &ScheduleListWidget::openCreateDialog);
}

void ScheduleListWidget::rebuildScheduleItems()
{
    clearListLayout();

    for (const ScheduleItem &item : std::as_const(m_items)) {
        auto *card = new QWidget(m_listContainer);
        card->setObjectName("ScheduleRowCard");

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 12, 12, 12);
        cardLayout->setSpacing(8); // 요소 간 간격

        // 1. 첫 번째 줄: 일시 정보 (가로로 가득 채움)
        QString startStr = QDateTime(item.date, item.time).toString("yyyy-MM-dd HH:mm");

        // 고정된 1시간 추가 대신, 저장된 종료 시간을 사용
        QString endStr = item.endDateTime.isValid()
                             ? item.endDateTime.toString("HH:mm")
                             : QDateTime(item.date, item.time).addSecs(3600).toString("HH:mm");

        auto *metaLabel = new QLabel(QString("%1 ~ %2").arg(startStr, endStr), card);
        metaLabel->setText(QString("%1 ~ %2").arg(startStr, endStr));
        metaLabel->setObjectName("ScheduleMeta");
        metaLabel->setStyleSheet("color: #666666; font-size: 11px;");

        // 2. 두 번째 줄: 제목
        auto *titleLabel = new QLabel(item.title, card);
        titleLabel->setObjectName("ScheduleTitle");
        titleLabel->setWordWrap(true);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

        // 3. 세 번째 줄: 버튼 전용 레이아웃 (우측 정렬)
        auto *buttonRow = new QHBoxLayout();
        buttonRow->setContentsMargins(0, 4, 0, 0); // 제목과의 간격

        // rebuildScheduleItems 함수 내 버튼 설정 부분 수정
        auto *editButton = new QPushButton(tr("Edit"), card);
        auto *deleteButton = new QPushButton(tr("Delete"), card);

        // 크기를 고정하는 대신 최소 크기를 지정하고, 사라지지 않게 정책 설정
        editButton->setMinimumSize(60, 26);
        deleteButton->setMinimumSize(70, 26); // Delete는 글자가 더 길어서 조금 더 넓게

        editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        buttonRow->addStretch(); // 왼쪽을 밀어서 버튼들을 오른쪽으로 정렬
        buttonRow->addWidget(editButton);
        buttonRow->addWidget(deleteButton);

        // --- 카드에 순서대로 쌓기 ---
        cardLayout->addWidget(metaLabel);
        cardLayout->addWidget(titleLabel);
        cardLayout->addLayout(buttonRow); // 버튼들을 제목 아래에 배치

        // --- Connect 로직 ---
        connect(editButton, &QPushButton::clicked, this, [this, item]() { openEditDialog(item); });
        connect(deleteButton, &QPushButton::clicked, this, [this, item]() {
            if (QMessageBox::question(this, tr("삭제"), tr("정말 삭제하시겠습니까?")) == QMessageBox::Yes) {
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
        ? tr("Schedules for %1").arg(m_currentDate.toString("dddd, MMM d yyyy"))
        : tr("Schedules");
}
