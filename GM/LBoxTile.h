// LBoxTile.h
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

#ifndef _LBOXTILE_H
#define _LBOXTILE_H

#ifndef     _LBOXGRFX_H
#include    "LBoxGrfx.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

class CTileListBox : public CGrafixListBox
{
// Construction
public:
    CTileListBox();

// Attributes
public:
    void SetDrawAllScales(BOOL bDrawAll) { m_bDrawAllScales = bDrawAll; }
    BOOL GetDrawAllScaled() { return m_bDrawAllScales; }

// Operations
public:
    void SetDocument(CGamDoc* pDoc) { m_pDoc = pDoc; }

// Implementation
protected:
    CGamDoc*    m_pDoc;
    BOOL        m_bDrawAllScales;
    int         m_bDisplayIDs;     // Set to prop [Settings]:DisplayIDs

    // Overrides
    virtual int OnItemHeight(int nIndex);
    virtual void OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
        CRect rctItem);
    virtual BOOL OnDragSetup(DragInfo* pDI);

    //{{AFX_MSG(CTileListBox)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

#endif

