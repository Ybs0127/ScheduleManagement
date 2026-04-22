#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "scheduleitem.h"

#include <QDate>
#include <QString>
#include <QWidget>

class QCalendarWidget;
class QLineEdit;

/**
 * @brief 달력 탐색과 일정 검색 UI를 함께 제공하는 위젯이다.
 *
 * 메인 윈도우의 우측 패널에서 사용되며, 날짜 선택 이벤트를 전달하고
 * 등록된 일정이 있는 날짜를 강조 표시하며 검색어 입력을 상위 계층으로 전달한다.
 *
 * 주요 변수 설명:
 * - m_calendar: 날짜 선택과 일정 하이라이트를 표시하는 실제 달력 컨트롤이다.
 * - m_searchEdit: 제목/메모 검색어를 입력받는 검색창이다.
 * - m_highlightedDates: 현재 강조 표시 중인 날짜 목록을 보관해 포맷 갱신에 사용한다.
 *
 * 주요 함수 설명:
 * - setSelectedDate(): 외부에서 선택 날짜를 동기화하고 달력 페이지를 해당 월로 이동한다.
 * - highlightDates(): 일정이 존재하는 날짜 목록을 받아 달력 셀 스타일을 갱신한다.
 * - setSchedules(): 달력 셀 내부에 표시할 일정 목록을 전달한다.
 * - setupUi(): 달력과 검색창 레이아웃 및 시그널 연결을 초기화한다.
 */
class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    /// 달력/검색 UI를 구성하고 초기 상태를 준비한다.
    explicit CalendarWidget(QWidget *parent = nullptr);
    /// 외부에서 선택한 날짜를 달력 표시 상태와 동기화한다.
    void setSelectedDate(const QDate &date);

signals:
    /// 사용자가 달력에서 날짜를 선택했을 때 선택 결과를 상위에 전달한다.
    void dateSelected(const QDate &date);
    /// 검색창의 텍스트가 변경될 때 현재 검색어를 상위에 전달한다.
    void searchRequested(const QString &keyword);

public slots:
    /// 일정이 있는 날짜 목록을 받아 달력 셀 강조 상태를 갱신한다.
    void highlightDates(const QList<QDate> &dates);
    /// 달력 셀에 요약 표시할 전체 일정 목록을 전달한다.
    void setSchedules(const QList<ScheduleItem> &items);

private:
    /// 내부 레이아웃과 위젯 생성, 이벤트 연결을 수행한다.
    void setupUi();

    /// 일정 셀 렌더링과 선택 이벤트를 담당하는 달력 컨트롤이다.
    QCalendarWidget *m_calendar;
    /// 키워드 검색 입력을 담당하는 라인 에디트이다.
    QLineEdit *m_searchEdit;
    /// 현재 하이라이트가 적용된 날짜 목록을 저장한다.
    QList<QDate> m_highlightedDates;
};

#endif // CALENDARWIDGET_H
