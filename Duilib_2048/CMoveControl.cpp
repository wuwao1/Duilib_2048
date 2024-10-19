#include "Main2048.h"

#define random_2_4 (rand()%5==4 ? 4:2)
#define random_x(x) (rand()%x)

CMoveControl::CMoveControl()
{
	m_pData2048 = std::make_unique<CData2048>();
}

CMoveControl::~CMoveControl()
{
}

void CMoveControl::Init()
{
	m_pData2048->_count = 0;
	m_pData2048->score = 0;
	memset(m_pData2048->_matrix, 0, sizeof(m_pData2048->_matrix));
	srand((int)time(0));
	for (int i = 0; i < 2; i++)
		RandomDataOne();
}

void CMoveControl::AddButton(CButtonUI::ptr bt)
{
	m_pData2048->buttons.push_back(bt);
}

void CMoveControl::AddTextScore(CTextUI::ptr bt)
{
	m_pData2048->scores = bt;
}

bool CMoveControl::ButtonUp()
{
	for (int k = 0; k < 4; k++)
	{
		bool flag = false;
		for (int i = 0; i < 4 - 1; i++)
		{
			if (m_pData2048->_matrix[i][k] == 0)
				flag = true;
			else
			{
				int j = i + 1;
				while (j < 4)
				{
					if (m_pData2048->_matrix[j][k])
					{
						if (m_pData2048->_matrix[i][k] == m_pData2048->_matrix[j][k])
							return true;
						else
							break;
					}
					else {
						++j;
					}
				}
			}
		}
		if (flag)
		{
			int i = 0, j = 4 - 1;
			while (i < 4)
			{
				if (m_pData2048->_matrix[i][k])
					++i;
				else
					break;
			}
			while (j >= 0)
			{
				if (m_pData2048->_matrix[j][k] == 0)
					--j;
				else
					break;
			}
			if (i < j)
				return true;
		}
	}
	return false;
}

bool CMoveControl::ButtonLeft()
{
	for (int k = 0; k < 4; k++)
	{
		bool flag = false;
		for (int i = 0; i < 4 - 1; i++)
		{
			if (m_pData2048->_matrix[k][i] == 0)
				flag = true;
			else
			{
				int j = i + 1;
				while (j < 4)
				{
					if (m_pData2048->_matrix[k][j])
					{
						if (m_pData2048->_matrix[k][i] == m_pData2048->_matrix[k][j])
							return true;
						else
							break;
					}
					else {
						++j;
					}
				}
			}
		}
		if (flag)
		{
			int i = 0, j = 4 - 1;
			while (i < 4)
			{
				if (m_pData2048->_matrix[k][i])
					++i;
				else
					break;
			}
			while (j >= 0)
			{
				if (m_pData2048->_matrix[k][j] == 0)
					--j;
				else
					break;
			}
			if (i < j)
				return true;
		}
	}
	return false;
}

bool CMoveControl::ButtonDown()
{
	for (int k = 0; k < 4; k++)
	{
		bool flag = false;
		for (int i = 4 - 1; i > 0; i--)
		{
			if (m_pData2048->_matrix[i][k] == 0)
				flag = true;
			else
			{
				int j = i - 1;
				while (j >= 0)
				{
					if (m_pData2048->_matrix[j][k])
					{
						if (m_pData2048->_matrix[i][k] == m_pData2048->_matrix[j][k])
							return true;
						else
							break;
					}
					else {
						--j;
					}
				}
			}
		}
		if (flag)
		{
			int i = 0, j = 4 - 1;
			while (i < 4)
			{
				if (m_pData2048->_matrix[i][k] == 0)
					++i;
				else
					break;
			}
			while (j >= 0)
			{
				if (m_pData2048->_matrix[j][k])
					--j;
				else
					break;
			}
			if (i < j)
				return true;
		}
	}
	return false;
}

bool CMoveControl::ButtonRight()
{
	for (int k = 0; k < 4; k++)
	{
		bool flag = false;
		for (int i = 4 - 1; i > 0; i--)
		{
			if (m_pData2048->_matrix[k][i] == 0)
				flag = true;
			else
			{
				int j = i - 1;
				while (j >= 0)
				{
					if (m_pData2048->_matrix[k][j])
					{
						if (m_pData2048->_matrix[k][i] == m_pData2048->_matrix[k][j])
							return true;
						else
							break;
					}
					else {
						--j;
					}
				}
			}
		}
		if (flag)
		{
			int i = 0, j = 4 - 1;
			while (i < 4)
			{
				if (m_pData2048->_matrix[k][i] == 0)
					++i;
				else
					break;
			}
			while (j >= 0)
			{
				if (m_pData2048->_matrix[k][j])
					--j;
				else
					break;
			}
			if (i < j)
				return true;
		}
	}
	return false;
}

void CMoveControl::KeyUp()
{
	if (ButtonUp())
	{
		for (int i = 0; i < 4; i++)
		{
			memset(m_pData2048->current, 0, sizeof(int) * 4);
			int ii = 0;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->_matrix[j][i])
					m_pData2048->current[ii++] = m_pData2048->_matrix[j][i];
			}
			for (int k = 0; k < ii - 1; k++)
			{
				if (m_pData2048->current[k] == m_pData2048->current[k + 1])
				{
					m_pData2048->current[k] *= 2;
					//m_pData2048->score += m_pData2048->current[k] / 2;
					m_pData2048->current[k + 1] = 0;
					++k;
					--m_pData2048->_count;
				}
			}
			ii = 0;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->current[j])
					m_pData2048->_matrix[ii++][i] = m_pData2048->current[j];
			}
			for (; ii < 4; ii++)
				m_pData2048->_matrix[ii][i] = 0;
		}
		RandomDataOne();
	}
}

void CMoveControl::KeyDown()
{
	if (ButtonDown())
	{
		for (int i = 0; i < 4; i++)
		{
			memset(m_pData2048->current, 0, sizeof(int) * 4);
			int ii = 0;
			for (int j = 4 - 1; j >= 0; j--)
			{
				if (m_pData2048->_matrix[j][i])
					m_pData2048->current[ii++] = m_pData2048->_matrix[j][i];
			}
			for (int k = 0; k < ii - 1; k++)
			{
				if (m_pData2048->current[k] == m_pData2048->current[k + 1])
				{
					m_pData2048->current[k] *= 2;
					//m_pData2048->score += m_pData2048->current[k] / 2;
					m_pData2048->current[k + 1] = 0;
					++k;
					--m_pData2048->_count;
				}
			}
			ii = 4 - 1;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->current[j])
					m_pData2048->_matrix[ii--][i] = m_pData2048->current[j];
			}
			for (; ii >= 0; ii--)
				m_pData2048->_matrix[ii][i] = 0;
		}
		RandomDataOne();
	}
}

void CMoveControl::KeyLeft()
{
	if (ButtonLeft())
	{
		for (int i = 0; i < 4; i++)
		{
			memset(m_pData2048->current, 0, sizeof(int) * 4);
			int ii = 0;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->_matrix[i][j])
					m_pData2048->current[ii++] = m_pData2048->_matrix[i][j];
			}
			for (int k = 0; k < ii - 1; k++)
			{
				if (m_pData2048->current[k] == m_pData2048->current[k + 1])
				{
					m_pData2048->current[k] *= 2;
					//m_pData2048->score += m_pData2048->current[k] / 2;
					m_pData2048->current[k + 1] = 0;
					++k;
					--m_pData2048->_count;
				}
			}
			ii = 0;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->current[j])
					m_pData2048->_matrix[i][ii++] = m_pData2048->current[j];
			}
			for (; ii < 4; ii++)
				m_pData2048->_matrix[i][ii] = 0;
		}
		RandomDataOne();
	}
}

void CMoveControl::KeyRight()
{
	if (ButtonRight())
	{
		for (int i = 0; i < 4; i++)
		{
			memset(m_pData2048->current, 0, sizeof(int) * 4);
			int ii = 0;
			for (int j = 4 - 1; j >= 0; j--)
			{
				if (m_pData2048->_matrix[i][j])
					m_pData2048->current[ii++] = m_pData2048->_matrix[i][j];
			}
			for (int k = 0; k < ii - 1; k++)
			{
				if (m_pData2048->current[k] == m_pData2048->current[k + 1])
				{
					m_pData2048->current[k] *= 2;
					//m_pData2048->score += m_pData2048->current[k] / 2;
					m_pData2048->current[k + 1] = 0;
					++k;
					--m_pData2048->_count;
				}
			}
			ii = 4 - 1;
			for (int j = 0; j < 4; j++)
			{
				if (m_pData2048->current[j])
					m_pData2048->_matrix[i][ii--] = m_pData2048->current[j];
			}
			for (; ii >= 0; ii--)
				m_pData2048->_matrix[i][ii] = 0;
		}
		RandomDataOne();
	}
}

bool CMoveControl::RandomDataOne()
{
	if (m_pData2048->_count == 16)
		return false;
	int left = 16 - m_pData2048->_count;
	int tmp = random_x(left);
	int num = random_2_4;
	int k = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (m_pData2048->_matrix[i][j] == 0)
			{
				if (k++ == tmp)
				{
					m_pData2048->_matrix[i][j] = num;
					break;
				}
			}
		}
	}
	++m_pData2048->_count;
	return true;
}

void CMoveControl::Print()
{
	for (int k = 0; k < 16; k++)
	{
		m_pData2048->buttons[k]->SetText(_T(""));
		m_pData2048->buttons[k]->SetAttribute(_T("bkcolor"), _T("#FFCCC0B3"));
	}
	auto TotalScore = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (m_pData2048->_matrix[i][j] != 0)
				ShowButton(m_pData2048->buttons[i * 4 + j], m_pData2048->_matrix[i][j]);
			TotalScore += m_pData2048->_matrix[i][j];
		}
	}
	auto TextScore = std::to_wstring(TotalScore);
	m_pData2048->scores->SetText(TextScore);
}



void CMoveControl::ShowButton(CButtonUI::ptr bt, int num)
{
	TCHAR str[10];
	wsprintf(str, L"%d", num);
	if (num == 2)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#ffeee4da"));
		bt->SetAttribute(_T("textcolor"), _T("776e65"));
	}
	else if (num == 4)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#ffede0c8"));
		bt->SetAttribute(_T("textcolor"), _T("ff776e65"));
	}
	else if (num == 8)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#fff2b179"));
		bt->SetAttribute(_T("textcolor"), _T("fff9f6f2"));
	}
	else if (num == 16)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#fff59563"));
		bt->SetAttribute(_T("textcolor"), _T("fff9f6f2"));
	}
	else if (num == 32)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#fff67c5f"));
		bt->SetAttribute(_T("textcolor"), _T("fff9f6f2"));
	}
	else if (num == 64)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#fff65e3b"));
		bt->SetAttribute(_T("textcolor"), _T("fff9f6f2"));
	}
	else if (num >= 128)
	{
		bt->SetAttribute(_T("bkcolor"), _T("#ffedcf72"));
		bt->SetAttribute(_T("textcolor"), _T("fff9f6f2"));
	}
	bt->SetText(str);
}