
// elevatorDlg.h: 头文件
//

#pragma once

// CelevatorDlg 对话框
class CelevatorDlg : public CDialogEx
{
	// 构造
public:
	CelevatorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ELEVATOR_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	unsigned int floors;        // 层数
	unsigned int numOfElevator; // 电梯数
	float storeyHeight;         // 层高
	unsigned int id;	             // 电梯ID
	unsigned int ratedCapacity;      // 额定载人量
	float timeOfCloseDoor;           // 电梯关门所用时间
	float timeOfOpenDoor;            // 电梯开门所用时间
	float timeOfWait;                // 电梯等待时间
	float timeOfBrake;               // 电梯制动时间
	float ratedSpeed;                // 额定速度
	float ratedAcceleration;         // 额定加速度
	float ratedJerk;                 // 额定加加速度

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_Combo;
	afx_msg void OnEnChangeElevatorNum();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnEnChangeElevatorId();
//	afx_msg void OnBnClickedButton1();
	afx_msg void OnClickedSet();
	afx_msg void OnAllSet();
	afx_msg void OnBnClickedUniformConfig();
//	virtual HRESULT accDoDefaultAction(VARIANT varChild);
};
