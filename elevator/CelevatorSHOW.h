#pragma once
// CelevatorSHOW 对话框

class CelevatorSHOW : public CDialogEx
{
	DECLARE_DYNAMIC(CelevatorSHOW)

public:
	CelevatorSHOW(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CelevatorSHOW();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ELEVATOR_SHOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
};

class Const
{
public:
	const int constNum;
	Const(int num) :constNum(num) {};
};
extern mutex g_cplock;
class ElevatorCopy
{
	Elevators elevators;
public:
	ElevatorCopy()
	{
		elevators.elevators = NULL;
	}
	ElevatorCopy(const Elevators& param)
	{
		elevators.numElevators = param.numElevators;
		elevators.elevators = new ElevatorRealTimeParam[param.numElevators];
		for (int i = 0; i < param.numElevators; ++i)
		{
			elevators.elevators[i].elevatorParam = param.elevators[i].elevatorParam;
			elevators.elevators[i].elevatorStatus = param.elevators[i].elevatorStatus;
		}
	}
	Elevators* ChangeOrGet(const Elevators& param, bool isChange)
	{
		g_cplock.lock();
		Elevators* temp = NULL;
		if (isChange)
		{
			if (elevators.elevators == NULL)
			{
				elevators.elevators = new ElevatorRealTimeParam[param.numElevators];
			}
			elevators.numElevators = param.numElevators;
			for (int i = 0; i < param.numElevators; ++i)
			{
				elevators.elevators[i].elevatorStatus = param.elevators[i].elevatorStatus;
				elevators.elevators[i].elevatorParam = param.elevators[i].elevatorParam;
			}
		}
		else
		{
			if (elevators.elevators != NULL)
			{
				temp = new Elevators;
				temp->numElevators = elevators.numElevators;
				temp->elevators = new ElevatorRealTimeParam[elevators.numElevators];
				for (int i = 0; i < elevators.numElevators; ++i)
				{
					temp->elevators[i].elevatorParam = elevators.elevators[i].elevatorParam;
					temp->elevators[i].elevatorStatus = elevators.elevators[i].elevatorStatus;
				}
			}
		}
		g_cplock.unlock();
		return temp;
	}
	Elevators& operator=(const Elevators& param)
	{
		if (elevators.elevators == NULL)
		{
			delete[]elevators.elevators;
		}
		elevators.numElevators = param.numElevators;
		elevators.elevators = new ElevatorRealTimeParam[param.numElevators];
		for (int i = 0; i < param.numElevators; ++i)
		{
			elevators.elevators[i].elevatorParam = param.elevators[i].elevatorParam;
			elevators.elevators[i].elevatorStatus = param.elevators[i].elevatorStatus;
		}
	}
	Elevators& GetElevatorsCopy()
	{
		return elevators;
	}
	~ElevatorCopy()
	{
		delete[]elevators.elevators;
	}
};
typedef struct EleRunParam
{
	float t;
	float restartTime;
	float stopTime; // 需要给定一个值
	bool isRestart;
}EleRunParam;