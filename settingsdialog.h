#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    int maxVisible() const { return m_maxVisibleSpin->value(); }
    bool warnOnEmpty() const { return m_warnOnEmptyCheck->isChecked(); }

    void setMaxVisible(int val) { m_maxVisibleSpin->setValue(val); }
    void setWarnOnEmpty(bool val) { m_warnOnEmptyCheck->setChecked(val); }

private:
    QSpinBox *m_maxVisibleSpin;
    QCheckBox *m_warnOnEmptyCheck;
};

#endif