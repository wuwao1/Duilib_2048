﻿#ifndef UI_PALLET_H
#define UI_PALLET_H
#pragma once

namespace DuiLib
{
/////////////////////////////////////////////////////////////////////////////////////
//
class UILIB_API CColorPaletteUI: public CControlUI
{
    DECLARE_DUICONTROL(CColorPaletteUI)
public:
    using ptr = std::shared_ptr<CColorPaletteUI>;

    CColorPaletteUI();
    virtual ~CColorPaletteUI();

    //获取最终被选择的颜色，可以直接用于设置duilib背景色
    DWORD GetSelectColor();
    void SetSelectColor(DWORD dwColor);

    virtual faw::string_t GetClass() const;
    virtual std::shared_ptr<CControlUI> GetInterface(faw::string_t pstrName);
    virtual void SetAttribute(faw::string_t pstrName, faw::string_t pstrValue);

    //设置/获取 Pallet（调色板主界面）的高度
    void SetPalletHeight(int nHeight);
    int	GetPalletHeight() const;

    //设置/获取 下方Bar（亮度选择栏）的高度
    void SetBarHeight(int nHeight);
    int GetBarHeight() const;
    //设置/获取 选择图标的路径
    void SetThumbImage(faw::string_t pszImage);
    faw::string_t GetThumbImage() const;

    virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
    virtual void DoInit();
    virtual void DoEvent(TEventUI& event);
    virtual void PaintBkColor(HDC hDC);
    virtual void PaintPallet(HDC hDC);

protected:
    //更新数据
    void UpdatePalletData();
    void UpdateBarData();

private:
    HDC			m_MemDc;
    HBITMAP		m_hMemBitmap		= NULL;
    BITMAP		m_bmInfo			= { 0 };
    unsigned char		*m_pBits				= nullptr;
    UINT		m_uButtonState		= 0;
    bool		m_bIsInBar			= false;
    bool		m_bIsInPallet		= false;
    int			m_nCurH				= 180;
    int			m_nCurS				= 200;
    int			m_nCurB				= 100;

    int			m_nPalletHeight		= 200;
    int			m_nBarHeight		= 10;
    POINT		m_ptLastPalletMouse;
    POINT		m_ptLastBarMouse;
    faw::string_t  m_strThumbImage;
};
}

#endif // UI_PALLET_H