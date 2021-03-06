// LBoxTileBase.h - class used to handle a variety of tile
//      oriented listbox functions
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

#ifndef _LBOXTILEBASE2_H
#define _LBOXTILEBASE2_H

#ifndef     _LBOXGFX2_H
#include    "LBoxGfx2.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

///////////////////////////////////////////////////////////////////////

class CTileBaseListBox2 : public CGrafixListBox2
{
public:
    CTileBaseListBox2();

    void SetTipMarkVisibility(BOOL bShow = TRUE) { m_bTipMarkItems = bShow; }
    BOOL GetTipMarkVisibility() { return m_bTipMarkItems; }

// Vars...
protected:
    int         m_bDisplayIDs;              // Set by property [Settings]:DisplayIDs

    BOOL        m_bTipMarkItems;
    CString     m_strTipMark;
    CSize       m_sizeTipMark;

// Helpers...
protected:
    void DrawTileImage(CDC* pDC, CRect rctItem, BOOL bDrawIt, int& x, TileID tid);
    void DrawItemDebugIDCode(CDC* pDC, int nItem, CRect rctItem, BOOL bDrawIt, int& x);

    void SetupTipMarkerIfRequired();
    void DrawTipMarker(CDC* pDC, CRect rctItem, BOOL bVisible, int& x);

    int  DoOnItemHeight(TileID tid1, TileID tid2);
    void DoOnDrawItem(CDC *pDC, int nItem, UINT nAction, UINT nState, CRect rctItem,
        TileID tid1, TileID tid2);

    void GetTileRectsForItem(int nItem, TileID tidLeft, TileID tidRight, CRect& rctLeft,
        CRect& rctRight);

// Overrides...
public:
    virtual CTileManager* GetTileManager() = 0;

// Overrides...
protected:
    virtual BOOL OnDoesItemHaveTipText(int nItem) { return FALSE; }

    // Subclass should only override one of these if any...
    virtual void* OnGetItemDebugIDCode(int nItem) { return MapIndexToItem(nItem); }
    virtual void  OnGetItemDebugString(int nItem, CString& str);

    //{{AFX_MSG(CTileBaseListBox)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


#endif
