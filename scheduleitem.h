#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

#include <QDate>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QTime>

/**
 * @brief 애플리케이션에서 단일 일정을 표현하는 데이터 구조체이다.
 *
 * 메인 윈도우의 일정 저장소, 달력 하이라이트/렌더링, 일정 목록 위젯, 추가/수정 다이얼로그
 * 사이에서 공통으로 전달되는 기본 모델 역할을 담당한다.
 *
 * 주요 변수 설명:
 * - id: 일정의 고유 식별자이며 수정/삭제 대상 식별에 사용된다.
 * - date: 일정 시작 날짜를 저장한다.
 * - time: 일정 시작 시간을 저장한다.
 * - endDateTime: 일정 종료 일시를 저장하며 목록 표시와 편집 시 사용된다.
 * - title: 일정의 제목을 저장한다.
 * - description: 일정의 상세 메모를 저장한다.
 */
struct ScheduleItem {
    /// 일정 수정/삭제 시 대상을 식별하는 내부 고유 번호이다.
    int id = -1;
    /// 일정 시작 날짜이다.
    QDate date;
    /// 일정 시작 시간이다.
    QTime time;
    /// 일정 종료 일시이며 "yyyy-MM-dd HH:mm" 형식으로 직렬화된다.
    QDateTime endDateTime;
    /// 목록과 검색 결과에 노출되는 일정 제목이다.
    QString title;
    /// 일정의 추가 설명 또는 메모이다.
    QString description;
};

Q_DECLARE_METATYPE(ScheduleItem)
Q_DECLARE_METATYPE(QList<ScheduleItem>)

#endif // SCHEDULEITEM_H
