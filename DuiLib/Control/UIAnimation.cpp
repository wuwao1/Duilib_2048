﻿#include "StdAfx.h"
#include "UIAnimation.h"
#include <vector>
#include <algorithm>

namespace DuiLib
{
struct CUIAnimation::Imp
{
    std::vector<CAnimationData*> m_arAnimations;
};

CUIAnimation::CUIAnimation() :
    m_pImp(new CUIAnimation::Imp())
{
    m_pControl = nullptr;
}

CUIAnimation:: ~CUIAnimation()
{
    if(m_pImp)
    {
        delete m_pImp;
        m_pImp = nullptr;
    }
}

void CUIAnimation::Attach(std::shared_ptr<CControlUI> pOwner)
{
    m_pControl = pOwner;
}

BOOL CUIAnimation::StartAnimation(int nElapse, int nTotalFrame, int nAnimationID /*= 0*/, BOOL bLoop/* = FALSE*/)
{
    CAnimationData* pData = GetAnimationDataByID(nAnimationID);
    if(pData || nElapse <= 0 || nTotalFrame <= 0 || !m_pControl)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    CAnimationData* pAnimation = new CAnimationData(nElapse, nTotalFrame, nAnimationID, bLoop);
    if(!pAnimation) return FALSE;

    if(m_pControl->GetManager()->SetTimer(m_pControl, nAnimationID, nElapse))
    {
        m_pImp->m_arAnimations.push_back(pAnimation);
        return TRUE;
    }
    return FALSE;
}

void CUIAnimation::StopAnimation(int nAnimationID /*= 0*/)
{
    if(!m_pControl) return;

    if(nAnimationID != 0)
    {
        CAnimationData* pData = GetAnimationDataByID(nAnimationID);
        if(nullptr != pData)
        {
            m_pControl->GetManager()->KillTimer(m_pControl, nAnimationID);
            m_pImp->m_arAnimations.erase(std::remove(m_pImp->m_arAnimations.begin(), m_pImp->m_arAnimations.end(), pData), m_pImp->m_arAnimations.end());
            if(pData)
            {
                delete pData;
                pData = nullptr;
            }
            return;
        }
    }
    else
    {
        size_t nCount = m_pImp->m_arAnimations.size();
        for(size_t i = 0; i < nCount; ++i)
        {
            CAnimationData* pData = m_pImp->m_arAnimations[i];
            if(pData)
            {
                m_pControl->GetManager()->KillTimer(m_pControl, pData->m_nAnimationID);
                if(pData)
                {
                    delete pData;
                    pData = nullptr;
                }
            }
        }
        m_pImp->m_arAnimations.clear();
    }
}

BOOL CUIAnimation::IsAnimationRunning(int nAnimationID)
{
    CAnimationData* pData = GetAnimationDataByID(nAnimationID);
    return !!pData;
}

int CUIAnimation::GetCurrentFrame(int nAnimationID/* = 0*/)
{
    CAnimationData* pData = GetAnimationDataByID(nAnimationID);
    if(!pData)
    {
        ASSERT(FALSE);
        return -1;
    }
    return pData->m_nCurFrame;
}

BOOL CUIAnimation::SetCurrentFrame(int nFrame, int nAnimationID/* = 0*/)
{
    CAnimationData* pData = GetAnimationDataByID(nAnimationID);
    if(!pData)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    if(nFrame >= 0 && nFrame <= pData->m_nTotalFrame)
    {
        pData->m_nCurFrame = nFrame;
        return TRUE;
    }
    else
    {
        ASSERT(FALSE);
    }
    return FALSE;
}

void CUIAnimation::OnAnimationElapse(int nAnimationID)
{
    if(!m_pControl) return;

    CAnimationData* pData = GetAnimationDataByID(nAnimationID);
    if(!pData) return;

    int nCurFrame = pData->m_nCurFrame;
    if(nCurFrame == 0)
    {
        OnAnimationStart(nAnimationID, pData->m_bFirstLoop);
        pData->m_bFirstLoop = FALSE;
    }

    OnAnimationStep(pData->m_nTotalFrame, nCurFrame, nAnimationID);

    if(nCurFrame >= pData->m_nTotalFrame)
    {
        OnAnimationStop(nAnimationID);
        if(pData->m_bLoop)
        {
            pData->m_nCurFrame = 0;
        }
        else
        {
            m_pControl->GetManager()->KillTimer(m_pControl, nAnimationID);
            m_pImp->m_arAnimations.erase(std::remove(m_pImp->m_arAnimations.begin(), m_pImp->m_arAnimations.end(), pData), m_pImp->m_arAnimations.end());
            delete pData;
            pData = nullptr;
        }
    }

    if(nullptr != pData)
    {
        ++(pData->m_nCurFrame);
    }
}

CAnimationData* CUIAnimation::GetAnimationDataByID(int nAnimationID)
{
    CAnimationData* pRet = nullptr;
    size_t nCount = m_pImp->m_arAnimations.size();
    for(size_t i = 0; i < nCount; ++i)
    {
        if(m_pImp->m_arAnimations[i]->m_nAnimationID == nAnimationID)
        {
            pRet = m_pImp->m_arAnimations[i];
            break;
        }
    }

    return pRet;
}

} // namespace DuiLib