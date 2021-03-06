// LBoxProj.cpp
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include    "stdafx.h"
#include    "ResTbl.h"
#include    "StrLib.h"
#include    "LBoxProj.h"
#include    "GdiTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Each item in the listbox has a five character prefix:
// 0: Group Letter ('A'...)
// 1: Style ('*'=bold, '+'=indent non bold)
// 2-4: number used to force a group to be sorted in a particular
//  sequence. default is 3 spaces.

const int prefixLen = 5;                // five characters

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CProjListBox, CGrafixListBox)
    //{{AFX_MSG_MAP(CProjListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CProjListBox::AddItem(int nGroupCode, LPCSTR pszText, int nSourceCode)
{
    CString str;
    str += (char)(nGroupCode + 'A');
    str += (char)(nSourceCode == -1 ? '*' : '+');
    str += "   ";           // Will always precede sequenced entries
    str += pszText;
    int nIdx = AddString(str);
    if (nIdx == LB_ERR)
        return nIdx;
    int nItem = SetItemData(nIdx, (DWORD)nSourceCode);
    m_nHorzWidth = max(GetItemWidth(nIdx), m_nHorzWidth);
    SetHorizontalExtent(m_nHorzWidth);
    return nItem;
}

int CProjListBox::AddSeqItem(int nGroupCode, LPCSTR pszText, int nSeqNum,
    int nSourceCode)
{
    CString str;
    str += (char)(nGroupCode + 'A');
    str += (char)(nSourceCode == -1 ? '*' : '+');

    char szNum[16];
    _itoa(nSeqNum, szNum, 10);
    StrLeadZeros(szNum, 3);
    str += szNum;

    str += pszText;
    int nIdx = AddString(str);
    if (nIdx == LB_ERR)
        return nIdx;
    int nItem = SetItemData(nIdx, (DWORD)nSourceCode);
    m_nHorzWidth = max(GetItemWidth(nIdx), m_nHorzWidth);
    SetHorizontalExtent(m_nHorzWidth);
    return nItem;
}

int CProjListBox::GetItemGroupCode(int nIndex)
{
    CString str;
    GetText(nIndex, str);
    return (int)(str[0] - 'A');
}

int CProjListBox::GetItemSourceCode(int nIndex)
{
    return (int)GetItemData(nIndex);
}

void CProjListBox::GetItemText(int nIndex, CString& str)
{
    CString strTmp;
    GetText(nIndex, strTmp);
    str = (const char*)strTmp + prefixLen;
}

/////////////////////////////////////////////////////////////////////////////

int CProjListBox::GetItemWidth(int nIndex)
{
    int nWidth = 0;

    // First character in string is sorting code. The second is the
    // style ('*' = Heading line, '+' = Item line)
    CString str;
    GetText(nIndex, str);
    BOOL bHead = str[1] == '*';
    CWindowDC scrnDC(NULL);

    CFont* pPrevFont = scrnDC.SelectObject(CFont::FromHandle(
        bHead ? g_res.h8ssb : g_res.h8ss));

    if (str[0] - 'A' == m_nMarkGrp &&
        GetItemSourceCode(nIndex) == m_nMarkSourceCode)
    {
        // Mark the line with a chevron
        nWidth = scrnDC.GetTextExtent("\xBB", 1).cx;
    }
    nWidth += scrnDC.GetTextExtent((const char*)str + prefixLen,
        lstrlen((const char*)str + prefixLen)).cx + 16; // (fudge factor)
    scrnDC.SelectObject(pPrevFont);
    return nWidth;
}

/////////////////////////////////////////////////////////////////////////////

int CProjListBox::OnItemHeight(int nIndex)
{
    return g_res.tm8ssb.tmHeight;
}

void CProjListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        COLORREF crPrevBack = pDC->SetBkColor(GetSysColor(
            nState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
        COLORREF crPrevText = pDC->SetTextColor(GetSysColor(
            nState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        // First character in string is sorting code. The second is the
        // style ('*' = Heading line, '+' = Item line)
        CString str;
        GetText(nIndex, str);
        BOOL bHead = str[1] == '*';

        CFont* pPrevFont = pDC->SelectObject(CFont::FromHandle(
            bHead ? g_res.h8ssb : g_res.h8ss));

        if (str[0] - 'A' == m_nMarkGrp &&
            GetItemSourceCode(nIndex) == m_nMarkSourceCode)
        {
            CRect rct = rctItem;
            // Mark the line with a chevron
            pDC->ExtTextOut(rct.left, rct.top, ETO_OPAQUE, rct,
                "\xBB", 1, NULL);
            rct.left += 3 * g_res.tm8ssb.tmAveCharWidth;
            pDC->ExtTextOut(rctItem.left + 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rct,
                (const char*)str + prefixLen,
                lstrlen((const char*)str + prefixLen), NULL);
        }
        else
        {
            pDC->ExtTextOut(rctItem.left +
                bHead ? 0 : 3 * g_res.tm8ssb.tmAveCharWidth,
                rctItem.top, ETO_OPAQUE, rctItem,
                (const char*)str + prefixLen,
                lstrlen((const char*)str + prefixLen), NULL);
        }

        pDC->SelectObject(pPrevFont);
        pDC->SetBkColor(crPrevBack);
        pDC->SetTextColor(crPrevText);
    }
    if (nAction & ODA_FOCUS)
        pDC->DrawFocusRect(&rctItem);
}


