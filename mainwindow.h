#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scheduleitem.h"

#include <QDate>
#include <QList>
#include <QMainWindow>

class CalendarWidget;
class ScheduleListWidget;

/**
 * @brief 일정 관리 애플리케이션의 메인 화면과 핵심 데이터 흐름을 총괄하는 창이다.
 *
 * 프로그램 시작 시 생성되는 최상위 윈도우로, 일정 데이터 저장소를 보관하고
 * 달력 위젯과 일정 목록 위젯 사이의 상호작용, 파일 입출력, 검색, 환경설정 반영을 조정한다.
 *
 * 주요 변수 설명:
 * - m_scheduleListWidget: 선택 날짜 또는 검색 결과의 일정 목록을 표시하는 좌측 패널이다.
 * - m_calendarWidget: 날짜 선택, 일정 하이라이트, 검색 입력을 담당하는 우측 패널이다.
 * - m_schedules: 메모리에 유지되는 전체 일정 목록이다.
 * - m_selectedDate: 현재 화면 기준이 되는 선택 날짜이다.
 * - m_searchKeyword: 목록을 검색 모드로 전환할 때 사용하는 현재 검색어이다.
 * - m_nextScheduleId: 신규 일정 추가 시 부여할 다음 고유 식별자이다.
 *
 * 주요 함수 설명:
 * - handleDateSelected()/handleScheduleAdded()/handleScheduleUpdated()/handleScheduleDeleted():
 *   하위 위젯 이벤트를 받아 내부 일정 저장소를 갱신한다.
 * - handleSearchRequested(): 검색어를 저장하고 목록 표시 모드를 갱신한다.
 * - handleExportRequested()/handleImportRequested(): 파일 내보내기/불러오기 흐름을 처리한다.
 * - refreshScheduleList()/refreshCalendarHighlights(): 현재 상태를 하위 UI에 다시 반영한다.
 * - exportAsCsv()/exportAsJson()/importFromJson(): 일정 데이터를 파일 포맷으로 직렬화/역직렬화한다.
 * - saveToDefaultStorage()/loadFromDefaultStorage(): 기본 저장소와의 자동 저장/로드를 담당한다.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// 메인 UI를 구성하고 기본 저장소를 불러와 초기 화면을 준비한다.
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    /// 달력에서 선택된 날짜를 현재 기준 날짜로 반영한다.
    void handleDateSelected(const QDate &date);
    /// 새 일정을 목록에 추가하고 영속 저장 및 UI 갱신을 수행한다.
    void handleScheduleAdded(const ScheduleItem &item);
    /// 기존 일정을 식별자로 찾아 교체하고 저장 및 UI 갱신을 수행한다.
    void handleScheduleUpdated(const ScheduleItem &item);
    /// 전달받은 식별자의 일정을 삭제하고 저장 및 UI 갱신을 수행한다.
    void handleScheduleDeleted(int id);
    /// 검색어를 갱신하고 검색 결과 기준으로 목록을 다시 계산한다.
    void handleSearchRequested(const QString &keyword);
    /// 지정 포맷에 맞춰 파일 저장 대화상자와 내보내기 로직을 실행한다.
    void handleExportRequested(const QString &format);
    /// JSON 파일을 읽어 일정 데이터를 덮어쓰기 또는 추가 방식으로 반영한다.
    void handleImportRequested(bool overwrite);
    /// 환경설정 다이얼로그를 열어 사용자 설정을 저장한다.
    void handleSettingsRequested();
    /// 메뉴에서 종료 요청이 들어왔을 때 애플리케이션 종료를 처리한다.
    void handleQuitRequested();
    /// 전체 일정 초기화를 확인받고 저장소와 UI를 비운다.
    void handleResetRequested();

private:
    /// 중앙 레이아웃과 주요 하위 위젯을 생성한다.
    void setupUi();
    /// 상단 메뉴와 각 액션을 생성하고 연결한다.
    void setupMenuBar();
    /// 달력/목록 위젯 간 시그널-슬롯 연결을 구성한다.
    void setupConnections();
    /// 공통 스타일시트를 적용한다.
    void applyStyles();
    /// 현재 선택 날짜 또는 검색어 기준으로 목록 위젯을 갱신한다.
    void refreshScheduleList();
    /// 달력 위젯의 일정 요약 표시와 하이라이트를 갱신한다.
    void refreshCalendarHighlights();
    /// 현재 선택 날짜에 해당하는 일정만 시간순으로 반환한다.
    QList<ScheduleItem> schedulesForSelectedDate() const;
    /// 제목/메모에 검색어가 포함된 일정을 정렬해 반환한다.
    QList<ScheduleItem> schedulesMatchingKeyword(const QString &keyword) const;
    /// 일정이 존재하는 날짜 목록을 중복 없이 추출한다.
    QList<QDate> scheduledDates() const;
    /// 전체 일정 데이터를 CSV 파일로 저장한다.
    bool exportAsCsv(const QString &filePath) const;
    /// 전체 일정 데이터를 JSON 파일로 저장한다.
    bool exportAsJson(const QString &filePath) const;
    /// JSON 파일을 읽어 내부 일정 목록으로 변환한다.
    bool importFromJson(const QString &filePath, bool overwrite = false, bool initialize = false);
    /// 애플리케이션 기본 저장 파일 경로를 계산한다.
    QString defaultStoragePath() const;
    /// 현재 일정 목록을 기본 저장소에 기록한다.
    bool saveToDefaultStorage() const;
    /// 기본 저장소에서 일정 목록을 읽어 초기 상태를 복원한다.
    bool loadFromDefaultStorage(bool initialize = false);
    /// 새 일정에 사용할 고유 식별자를 발급한다.
    int generateScheduleId();


    /// 일정 목록 표시와 편집 요청 중계를 담당하는 좌측 패널이다.
    ScheduleListWidget *m_scheduleListWidget;
    /// 달력 표시와 검색 입력을 담당하는 우측 패널이다.
    CalendarWidget *m_calendarWidget;
    /// 메모리에 유지되는 전체 일정 데이터 저장소이다.
    QList<ScheduleItem> m_schedules;
    /// 현재 화면 기준이 되는 선택 날짜이다.
    QDate m_selectedDate;
    /// 현재 적용 중인 검색 키워드이다.
    QString m_searchKeyword;
    /// 다음으로 발급할 일정 고유 번호이다.
    int m_nextScheduleId;
};

#endif // MAINWINDOW_H
