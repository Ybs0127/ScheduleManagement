#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>

/**
 * @brief 일정 관리 애플리케이션의 사용자 설정값을 편집하는 다이얼로그이다.
 *
 * 메인 윈도우의 환경설정 메뉴에서 호출되며, 달력 셀에 표시할 최대 일정 수와
 * 비어 있는 일정 파일을 불러올 때의 경고 여부를 사용자가 조정할 수 있게 한다.
 *
 * 주요 변수 설명:
 * - m_maxVisibleSpin: 달력 셀당 노출할 일정 수 제한을 보관하는 스핀 박스이다.
 * - m_warnOnEmptyCheck: 빈 데이터 불러오기 시 경고 표시 여부를 보관하는 체크 박스이다.
 *
 * 주요 함수 설명:
 * - maxVisible()/warnOnEmpty(): 현재 다이얼로그에서 선택된 설정값을 읽어온다.
 * - setMaxVisible()/setWarnOnEmpty(): 저장된 설정값을 다이얼로그 초기 상태에 반영한다.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    /// 환경설정 입력 위젯과 확인/취소 버튼을 초기화한다.
    explicit SettingsDialog(QWidget *parent = nullptr);

    /// 달력 셀당 최대 표시 일정 수 설정값을 반환한다.
    int maxVisible() const { return m_maxVisibleSpin->value(); }
    /// 빈 일정 파일 불러오기 경고 사용 여부를 반환한다.
    bool warnOnEmpty() const { return m_warnOnEmptyCheck->isChecked(); }

    /// 저장된 최대 표시 개수 설정값을 다이얼로그에 반영한다.
    void setMaxVisible(int val) { m_maxVisibleSpin->setValue(val); }
    /// 저장된 경고 표시 설정값을 다이얼로그에 반영한다.
    void setWarnOnEmpty(bool val) { m_warnOnEmptyCheck->setChecked(val); }

private:
    /// 달력 셀당 최대 일정 표시 개수를 입력받는 스핀 박스이다.
    QSpinBox *m_maxVisibleSpin;
    /// 빈 일정 데이터 경고 여부를 설정하는 체크 박스이다.
    QCheckBox *m_warnOnEmptyCheck;
};

#endif
