// LBoxSlct.h
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

#ifndef _LBOXSLCT_H
#define _LBOXSLCT_H

#ifndef     _PIECES_H
#include    "Pieces.h"
#endif

#ifndef     _LBOXTILEBASE2_H
#include    "LBoxTileBase2.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

class CSelectListBox : public CTileBaseListBox2
{
// Construction
public:
    CSelectListBox()
    {
        m_pDoc = NULL;
    }

// Attributes
public:
    virtual CTileManager* GetTileManager();

// Operations
public:
    void SetDocument(CGamDoc *pDoc) { m_pDoc = pDoc; }

// Implementation
protected:
    CGamDoc*    m_pDoc;

    // Misc
    TileID CSelectListBox::GetTileID(BOOL bActiveIfApplies, int nIndex);
    void GetTileRectsForTwoSidedView(int nLBoxIndex, CRect& rctLeft, CRect& rctRight);

    // Overrides
    virtual int OnItemHeight(int nIndex);
    virtual void OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
        CRect rctItem);
    virtual BOOL OnDragSetup(DragInfo* pDI);

    virtual void OnGetItemDebugString(int nIndex, CString& str);

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled();
    virtual int  OnGetHitItemCodeAtPoint(CPoint point, CRect& rct);
    virtual void OnGetTipTextForItemCode(int nItemCode, CString& strTip, CString& strTitle);
    virtual BOOL OnDoesItemHaveTipText(int nItem);

    //{{AFX_MSG(CSelectListBox)
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif

