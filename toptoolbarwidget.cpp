#include "toptoolbarwidget.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
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

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    auto *titleLabel = new QLabel(tr("Planner Console"), card);
    titleLabel->setObjectName("SectionTitle");

    auto *subtitleLabel = new QLabel(tr("Search, import, export, and manage your schedule flow."), card);
    subtitleLabel->setObjectName("SectionSubtitle");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    layout->addLayout(titleLayout, 1);

    m_searchEdit = new QLineEdit(card);
    m_searchEdit->setPlaceholderText(tr("Search schedules by title or description"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(280);

    m_settingsButton = new QPushButton(tr("Settings"), card);
    m_importButton = new QPushButton(tr("Import Config"), card);
    m_exportButton = new QPushButton(tr("Export"), card);

    auto *exportMenu = new QMenu(m_exportButton);
    const QList<QPair<QString, QString>> exportOptions = {
        {tr("Export CSV"), QStringLiteral("csv")},
        {tr("Export PDF"), QStringLiteral("pdf")},
        {tr("Export Image"), QStringLiteral("image")},
    };

    for (const auto &option : exportOptions) {
        QAction *action = exportMenu->addAction(option.first);
        action->setData(option.second);
        connect(action, &QAction::triggered, this, [this, action]() {
            emit exportRequested(action->data().toString());
        });
    }

    m_exportButton->setMenu(exportMenu);

    layout->addWidget(m_searchEdit);
    layout->addWidget(m_importButton);
    layout->addWidget(m_exportButton);
    layout->addWidget(m_settingsButton);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &TopToolbarWidget::searchRequested);
    connect(m_importButton, &QPushButton::clicked, this, &TopToolbarWidget::importRequested);
    connect(m_settingsButton, &QPushButton::clicked, this, &TopToolbarWidget::settingsRequested);
}
