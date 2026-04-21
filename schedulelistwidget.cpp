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
class ScheduleViewerDialog : public QDialog
{
public:
    explicit ScheduleViewerDialog(const ScheduleItem &item, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(tr("일정 상세 확인"));
        resize(400, 300);

        auto *layout = new QVBoxLayout(this);
        auto *formLayout = new QFormLayout();

        // 1. 시간 표시
        QString timeStr = QString("%1 %2 ~ %3")
                              .arg(item.date.toString("yyyy-MM-dd"),
                                   item.time.toString("HH:mm"),
                                   item.endDateTime.isValid() ? item.endDateTime.toString("MM-dd HH:mm") : "---");

        auto *timeLabel = new QLabel(timeStr, this);
        timeLabel->setStyleSheet("font-weight: bold;");

        // 2. 제목 표시
        auto *titleLabel = new QLabel(item.title, this);
        titleLabel->setWordWrap(true);
        titleLabel->setStyleSheet("font-size: 16px; color: #2c3e50; font-weight: bold;");

        // 3. 메모 표시
        auto *descText = new QPlainTextEdit(this);
        descText->setPlainText(item.description.isEmpty() ? tr("(메모 없음)") : item.description);
        descText->setReadOnly(true); // 수정 불가 설정
        descText->setStyleSheet("background-color: #f9f9f9; border: 1px solid #ddd;");

        formLayout->addRow(tr("일시:"), timeLabel);
        formLayout->addRow(tr("제목:"), titleLabel);
        layout->addLayout(formLayout);
        layout->addWidget(new QLabel(tr("메모:"), this));
        layout->addWidget(descText);

        // 4. 닫기 버튼
        auto *closeBtn = new QPushButton(tr("확인"), this);
        closeBtn->setDefault(true);
        layout->addWidget(closeBtn);

        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    }
};

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

        // --- 시작 일시 ---
        QDateTime startDT(m_item.date, m_item.time);
        if (!startDT.isValid()) startDT = QDateTime::currentDateTime();

        m_startDateTimeEdit = new QDateTimeEdit(startDT, this);
        m_startDateTimeEdit->setCalendarPopup(true);
        m_startDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        // --- 종료 일시 (여기서 초기화!) ---
        QDateTime endDT = m_item.endDateTime;
        if (!endDT.isValid()) {
            endDT = startDT.addSecs(3600); // 값이 없으면 1시간 후로
        }

        // --- 종료 일시 (날짜 + 시간) ---
        // 기본값으로 시작 일시로부터 1시간 후 설정
        m_endDateTimeEdit = new QDateTimeEdit(endDT, this);
        m_endDateTimeEdit->setCalendarPopup(true);
        m_endDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        m_titleEdit = new QLineEdit(m_item.title, this);
        m_descriptionEdit = new QPlainTextEdit(m_item.description, this);
        m_descriptionEdit->setPlaceholderText(tr("메모"));
        m_descriptionEdit->setFixedHeight(96);

        // 폼 레이아웃에 추가
        formLayout->addRow(tr("시작 일시"), m_startDateTimeEdit);
        formLayout->addRow(tr("종료 일시"), m_endDateTimeEdit);
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

    ScheduleItem item() const
    {
        return m_item;
    }

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

        //이 카드에 마우스 이벤트를 이 클래스(ScheduleListWidget)가 감시하도록 설정
        card->installEventFilter(this);
        //이벤트 필터에서 어떤 아이템인지 식별하기 위해 데이터를 저장해둠
        card->setProperty("item_id", item.id);

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(10);

        // 1. 첫 번째 줄: 일시 정보 (가로로 가득 채움)
        QDateTime startDT(item.date, item.time);
        QDateTime endDT = item.endDateTime.isValid()
                              ? item.endDateTime
                              : startDT.addSecs(3600);
        QString startStr;
        QString endStr;

        if (startDT.date() == endDT.date()) {
            startStr = startDT.toString("MM-dd HH:mm");
            endStr = endDT.toString("HH:mm");
        } else {
            startStr = startDT.toString("MM-dd HH:mm");
            endStr = endDT.toString("MM-dd HH:mm");
        }

        auto *metaLabel = new QLabel(QString("%1 ~ %2").arg(startStr, endStr), card);
        metaLabel->setText(QString("%1 ~ %2").arg(startStr, endStr));
        metaLabel->setObjectName("ScheduleMeta");

        auto *editButton = new QPushButton(tr("수정"), card);
        auto *deleteButton = new QPushButton(tr("삭제"), card);

        auto *titleLabel = new QLabel(item.title, card);
        titleLabel->setObjectName("ScheduleTitle");
        titleLabel->setWordWrap(true);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

        //QHBoxLayout *buttonRow = new QHBoxLayout();


        // 3. 세 번째 줄: 버튼 전용 레이아웃 (우측 정렬)
        auto *buttonRow = new QHBoxLayout();
        buttonRow->setContentsMargins(0, 4, 0, 0); // 제목과의 간격

        // 크기를 고정하는 대신 최소 크기를 지정하고, 사라지지 않게 정책 설정
        editButton->setMinimumSize(60, 26);
        deleteButton->setMinimumSize(70, 26); // Delete는 글자가 더 길어서 조금 더 넓게

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

bool ScheduleListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (obj->objectName() == "ScheduleRowCard") {
            int itemId = obj->property("item_id").toInt();

            for (const ScheduleItem &item : m_items) {
                if (item.id == itemId) {
                    // --- 여기를 수정: 수정창 대신 확인창 생성 ---
                    ScheduleViewerDialog viewer(item, this);
                    viewer.exec();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
