// DragDrop.h
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

#ifndef _DRAGDROP_H
#define _DRAGDROP_H

////////////////////////////////////////////////////////////////
// The WM_DRAGDROP message is sent the underlying window
// to see if a drop can occur. The underlying window should
// return a HCURSOR to signify the state of the drop. If zero
// is returned, the "No Drop" cursor is displayed.
//
// The message format is:
//      WPARAM = drag phase code.
//      LPARAM = Pointer to DragInfo structure.
// Return:
//      HCURSOR to display (if phaseDragOver). Or zero for no drop cursor.

extern UINT WM_DRAGDROP;

// Drag phase codes...

const   WORD    phaseDragEnter = 0;
const   WORD    phaseDragExit  = 1;
const   WORD    phaseDragOver  = 2;
const   WORD    phaseDragDrop  = 3;

// ----------------------------------------------- //

enum DragType
{
    DRAG_TILE,              // pObj = CTileManager*, dwItem = TileID
    DRAG_TILELIST,          // pObj = CTileManager*, dwItem = CWordArray*
    DRAG_MGRTILE,           // pObj = CTileManager*, dwItem = TileID
    DRAG_PIECE,             // pObj = CGamDoc*, dwItem = PieceID
    DRAG_PIECELIST,         // pObj = CGamDoc*, dwItem = CWordArray*
    DRAG_MARKER,            // pObj = CMarkManager*, dwItem = MarkID
    DRAG_SELECTLIST,        // pObj = CGamDoc*, dwItem = CSelList*
    DRAG_SELECTVIEW         // pObj = CGamDoc*, dwItem = CWordArray*
};

struct DragInfo
{
    DragType    m_dragType;
//  BOOL        m_bDropAccepted;// Set by droppee so source knows what happened
    CPoint      m_point;        // Loc of mouse in window (dragOver&dragDrop)
    CPoint      m_pointClient;  // list box relative
    HCURSOR     m_hcsrSuggest;  // Suggested cursor to return if can drop
    // Drag specific info...
    DWORD       m_dwVal;        // Depends on dragType
    void*       m_pObj;         // Depends on dragType
};

#endif

