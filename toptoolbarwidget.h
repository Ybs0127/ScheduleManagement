#ifndef TOPTOOLBARWIDGET_H
#define TOPTOOLBARWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class TopToolbarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopToolbarWidget(QWidget *parent = nullptr);

signals:
    void searchRequested(const QString &keyword);
    void exportRequested(const QString &format);
    void importRequested();
    void settingsRequested();

private:
    void setupUi();

    QPushButton *m_settingsButton;
    QPushButton *m_importButton;
    QPushButton *m_exportButton;
    QLineEdit *m_searchEdit;
};

#endif // TOPTOOLBARWIDGET_H
