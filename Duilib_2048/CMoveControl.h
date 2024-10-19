#pragma once

class CData2048
{
public:
	int _matrix[4][4];
	int current[4];
	int _count;
	int score;
	CTextUI::ptr scores;
	std::vector<CButtonUI::ptr> buttons;
};


class CMoveControl
{
	std::unique_ptr<CData2048> m_pData2048;
public:
	CMoveControl();
	~CMoveControl();

	void Init();
	void AddButton(CButtonUI::ptr bt);
	void AddTextScore(CTextUI::ptr bt);

	bool ButtonUp();
	bool ButtonLeft();
	bool ButtonDown();
	bool ButtonRight();

	void KeyUp();
	void KeyDown();
	void KeyLeft();
	void KeyRight();

	bool RandomDataOne();
	void Print();
	void ShowButton(CButtonUI::ptr bt, int num);
};
