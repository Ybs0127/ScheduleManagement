#ifndef SCHEDULELISTWIDGET_H
#define SCHEDULELISTWIDGET_H

#include "scheduleitem.h"

#include <QDate>
#include <QWidget>

class QLabel;
class QPushButton;
class QVBoxLayout;

/**
 * @brief 선택된 날짜의 일정 목록을 보여주고 일정 추가/수정/삭제를 중계하는 위젯이다.
 *
 * 메인 윈도우의 좌측 패널에서 사용되며, 현재 날짜 또는 검색 결과에 맞는 일정 카드를 렌더링하고
 * 편집 다이얼로그와 상세 보기 다이얼로그를 통해 사용자 상호작용을 처리한다.
 *
 * 주요 변수 설명:
 * - m_titleLabel: 현재 목록의 기준 날짜 또는 검색 결과 제목을 표시한다.
 * - m_listContainer/m_listLayout: 일정 카드 위젯들을 동적으로 생성해 담는 컨테이너이다.
 * - m_addButton: 새 일정 등록 다이얼로그를 여는 버튼이다.
 * - m_currentDate: 일정 생성 기본값과 제목 생성에 사용되는 현재 기준 날짜이다.
 * - m_items: 현재 목록에 표시 중인 일정 데이터이다.
 * - m_customTitle: 검색 결과 등 날짜 제목 대신 사용할 사용자 지정 제목이다.
 *
 * 주요 함수 설명:
 * - setDate(): 목록의 기준 날짜를 변경하고 기본 제목을 갱신한다.
 * - updateScheduleList(): 외부에서 전달된 일정 목록으로 카드 목록을 다시 구성한다.
 * - setListTitle(): 날짜 제목 대신 표시할 커스텀 제목을 설정한다.
 * - openCreateDialog()/openEditDialog(): 일정 추가/수정 다이얼로그를 열고 결과를 시그널로 전달한다.
 * - rebuildScheduleItems(): 현재 일정 데이터를 기반으로 카드 UI를 다시 생성한다.
 */
class ScheduleListWidget : public QWidget
{
    Q_OBJECT

public:
    /// 일정 목록 패널 UI를 구성하고 현재 날짜 기준 상태를 초기화한다.
    explicit ScheduleListWidget(QWidget *parent = nullptr);
    /// 목록 상단 제목을 날짜 기반 제목 또는 검색 결과 제목으로 설정한다.
    void setListTitle(const QString &title);

signals:
    /// 새 일정이 생성되었을 때 상위에 저장 요청을 전달한다.
    void scheduleAdded(const ScheduleItem &item);
    /// 기존 일정이 수정되었을 때 상위에 반영 요청을 전달한다.
    void scheduleUpdated(const ScheduleItem &item);
    /// 일정 삭제가 확정되었을 때 식별자를 상위에 전달한다.
    void scheduleDeleted(int id);
protected:
    /// 일정 카드 더블클릭을 감지해 상세 보기 다이얼로그를 표시한다.
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    /// 현재 표시 기준 날짜를 설정하고 제목을 날짜 기준으로 갱신한다.
    void setDate(const QDate &date);
    /// 전달받은 일정 목록으로 카드 영역을 재구성한다.
    void updateScheduleList(const QList<ScheduleItem> &items);

private:
    /// 외곽 카드, 스크롤 영역, 추가 버튼 등 목록 패널 UI를 구성한다.
    void setupUi();
    /// m_items 내용을 바탕으로 일정 카드 목록을 새로 생성한다.
    void rebuildScheduleItems();
    /// 새 일정 초안으로 편집 다이얼로그를 열어 추가 요청을 발생시킨다.
    void openCreateDialog();
    /// 기존 일정 데이터를 편집 다이얼로그에 로드해 수정 요청을 발생시킨다.
    void openEditDialog(const ScheduleItem &item);
    /// 기존 카드 위젯과 레이아웃 아이템을 모두 제거한다.
    void clearListLayout();
    /// 현재 날짜를 사람이 읽기 쉬운 목록 제목 문자열로 변환한다.
    QString defaultDateTitle() const;

    /// 목록 섹션 상단 제목 라벨이다.
    QLabel *m_titleLabel;
    /// 스크롤 영역 내부에서 일정 카드들을 담는 컨테이너 위젯이다.
    QWidget *m_listContainer;
    /// 일정 카드들을 수직으로 배치하는 레이아웃이다.
    QVBoxLayout *m_listLayout;
    /// 일정 추가 다이얼로그를 여는 버튼이다.
    QPushButton *m_addButton;
    /// 목록 기준 날짜이자 신규 일정 기본 날짜이다.
    QDate m_currentDate;
    /// 현재 화면에 렌더링할 일정 데이터 목록이다.
    QList<ScheduleItem> m_items;
    /// 날짜 기반 기본 제목 대신 사용할 커스텀 제목이다.
    QString m_customTitle;
};

#endif // SCHEDULELISTWIDGET_H
