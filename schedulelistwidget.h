#ifndef SCHEDULELISTWIDGET_H
#define SCHEDULELISTWIDGET_H

#include "scheduleitem.h"

#include <QDate>
#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

class ScheduleListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleListWidget(QWidget *parent = nullptr);
    void setListTitle(const QString &title);

signals:
    void scheduleAdded(const ScheduleItem &item);
    void scheduleUpdated(const ScheduleItem &item);
    void scheduleDeleted(int id);

public slots:
    void setDate(const QDate &date);
    void updateScheduleList(const QList<ScheduleItem> &items);

private:
    void setupUi();
    void rebuildScheduleItems();
    void openCreateDialog();
    void openEditDialog(const ScheduleItem &item);
    void clearListLayout();
    QString defaultDateTitle() const;

    QLabel *m_titleLabel;
    QWidget *m_listContainer;
    QVBoxLayout *m_listLayout;
    QPushButton *m_addButton;
    QDate m_currentDate;
    QList<ScheduleItem> m_items;
    QString m_customTitle;
};

#endif // SCHEDULELISTWIDGET_H
