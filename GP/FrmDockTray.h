// FrmDockTray.h - container window for the tray palettes.
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

#ifndef _FRMDOCKTRAY_H
#define _FRMDOCKTRAY_H

/////////////////////////////////////////////////////////////////////////////
// CDockTrayPalette window
class CTrayPalette;

class CDockTrayPalette : public CDockablePane
{
    DECLARE_DYNAMIC(CDockTrayPalette);
// Construction
public:
    CDockTrayPalette();

// Attributes
public:
    CTrayPalette*   m_pChildWnd;

// Operations
public:
    void SetChild(CTrayPalette* pChildWnd);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDockTrayPalette)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CDockTrayPalette();

    // Generated message map functions
protected:
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif

