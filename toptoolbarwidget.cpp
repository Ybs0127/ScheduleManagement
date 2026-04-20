#include "toptoolbarwidget.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

TopToolbarWidget::TopToolbarWidget(QWidget *parent)
    : QWidget(parent)
    , m_settingsButton(nullptr)
    , m_importButton(nullptr)
    , m_exportButton(nullptr)
    , m_searchEdit(nullptr)
{
    setupUi();
}

void TopToolbarWidget::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(this);
    card->setProperty("card", true);
    outerLayout->addWidget(card);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(12);

    m_searchEdit = new QLineEdit(card);
    m_searchEdit->setPlaceholderText(tr("검색"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(280);

    m_settingsButton = new QPushButton(tr("설정"), card);
    m_importButton = new QPushButton(tr("일정 불러오기"), card);
    m_exportButton = new QPushButton(tr("일정 내보내기"), card);

    auto *exportMenu = new QMenu(m_exportButton);
    const QList<QPair<QString, QString>> exportOptions = {
        {tr("CSV로 내보내기"), QStringLiteral("csv")},
        {tr("json으로 내보내기"), QStringLiteral("json")},
        {tr("Image로 내보내기"), QStringLiteral("image")},
    };

    for (const auto &option : exportOptions) {
        QAction *action = exportMenu->addAction(option.first);
        action->setData(option.second);
        connect(action, &QAction::triggered, this, [this, action]() {
            emit exportRequested(action->data().toString());
        });
    }

    m_exportButton->setMenu(exportMenu);

    layout->addWidget(m_searchEdit, 1);
    layout->addWidget(m_importButton);
    layout->addWidget(m_exportButton);
    layout->addWidget(m_settingsButton);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &TopToolbarWidget::searchRequested);
    connect(m_importButton, &QPushButton::clicked, this, &TopToolbarWidget::importRequested);
    connect(m_settingsButton, &QPushButton::clicked, this, &TopToolbarWidget::settingsRequested);
}
