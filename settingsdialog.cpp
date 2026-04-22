#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("환경설정"));
    auto *layout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    m_maxVisibleSpin = new QSpinBox(this);
    m_maxVisibleSpin->setRange(1, 10);

    m_warnOnEmptyCheck = new QCheckBox(tr("빈 일정 파일 로드 시 경고 표시"), this);

    formLayout->addRow(tr("셀당 최대 일정 표시 개수:"), m_maxVisibleSpin);
    formLayout->addRow(m_warnOnEmptyCheck);

    layout->addLayout(formLayout);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}