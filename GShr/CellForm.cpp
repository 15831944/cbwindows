// CellForm.cpp
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
#include    "GdiTools.h"
#include    "CellForm.h"
#include    "StrLib.h"
#include    "CDib.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

inline int RoundHalf(int x) { return x / 2 + (x & 1); }

//////////////////////////////////////////////////////////////////////

CCellForm::CCellForm()
{
    m_pPoly = NULL;
    m_pMask = NULL;
    m_pWrk = NULL;
    m_nStagger = 0;
    memset(&m_bmapMask, 0, sizeof(BITMAP));
}

//////////////////////////////////////////////////////////////////////

BOOL CCellForm::CompareEqual(CCellForm& cf)
{
    return m_eType == cf.m_eType && m_nStagger == cf.m_nStagger &&
        m_rct == cf.m_rct;
}

//////////////////////////////////////////////////////////////////////

void CCellForm::CreateCell(CellFormType eType, int nParm1, int nParm2,
    int nStagger)
{
    int nWide, nFace, nLeg;

    Clear();
    m_eType = eType;
    m_nStagger = nStagger;

    if (eType == cformRect || eType == cformBrickHorz ||
        eType == cformBrickVert)
    {
        m_pPoly = new POINT[5];     // Last point repeats first
        m_pWrk  = new POINT[5];

        m_rct.SetRect(0, 0, nParm2, nParm1);
        m_pPoly[0].x = m_pPoly[3].x = m_pPoly[4].x =
            m_pPoly[0].y = m_pPoly[1].y = 0;
        m_pPoly[1].x = m_pPoly[2].x = m_rct.right;
        m_pPoly[2].y = m_pPoly[3].y = m_rct.bottom;
        m_pPoly[4] = m_pPoly[0];    // Wrap around

        return;                     // No mask is required.
    }
    else
    {
        nWide = nParm1 - 1; // Because it ends up one larger in end
        nFace = MulDiv(nParm1, 1000, 1732);
        nLeg = MulDiv(nParm1, 1000, 3464);

        m_pPoly = new POINT[7];     // Extra pnt-> last point repeats first
        m_pWrk  = new POINT[7];

        if (eType == cformHexPnt)
        {
            //         1
            //       0/ \2
            //       |   |
            //       5\ /3
            //         4
            m_pPoly[0].x = m_pPoly[5].x = 0;
            m_pPoly[1].x = m_pPoly[4].x = nWide / 2;
            m_pPoly[2].x = m_pPoly[3].x = nWide;

            m_pPoly[1].y = 0;
            m_pPoly[0].y = m_pPoly[2].y = nLeg;
            m_pPoly[3].y = m_pPoly[5].y = nLeg + nFace;
            m_pPoly[4].y = 2 * nLeg + nFace;

            m_pPoly[6] = m_pPoly[0];    // Wrap around

            m_rct.SetRect(0, 0, m_pPoly[2].x + 1, m_pPoly[4].y + 1);
        }
        else
        {
            //      1  2
            //      /--\
            //    0/    \3
            //     \    /
            //     5\--/4
            m_pPoly[0].x = 0;
            m_pPoly[1].x = m_pPoly[5].x = nLeg;
            m_pPoly[2].x = m_pPoly[4].x = nLeg + nFace;
            m_pPoly[3].x = 2 * nLeg + nFace;

            m_pPoly[1].y = m_pPoly[2].y = 0;
            m_pPoly[0].y = m_pPoly[3].y = nWide / 2;
            m_pPoly[5].y = m_pPoly[4].y = nWide;

            m_pPoly[6] = m_pPoly[0];    // Wrap around

            m_rct.SetRect(0, 0, m_pPoly[3].x + 1, m_pPoly[4].y + 1);
        }
        // Create a monochrome mask for hexes.
        CreateHexMask();
    }
}

void CCellForm::CreateHexMask()
{
    CDC dcMask;
    dcMask.CreateCompatibleDC(NULL);
    SetupPalette(&dcMask);
    m_pMask = new CBitmap;
    //  m_pMask->CreateBitmap(m_rct.right, m_rct.bottom, 1, 1, NULL);
    m_pMask->Attach(Create16BitDIBSection(dcMask.m_hDC,
        m_rct.right, m_rct.bottom));
    ResetPalette(&dcMask);

    CBitmap *prvbmMask = dcMask.SelectObject(m_pMask);

    // Make a black hex with white exterior
    dcMask.PatBlt(0, 0, m_rct.right, m_rct.bottom, WHITENESS);
    dcMask.SelectStockObject(BLACK_BRUSH);
    dcMask.SelectStockObject(BLACK_PEN);
    dcMask.Polygon(m_pPoly, 6);
    dcMask.SelectObject(prvbmMask);

    m_pMask->GetObject(sizeof(BITMAP), &m_bmapMask);
    ASSERT(m_bmapMask.bmBits != NULL);

    //TEST--Write mask file--TEST//
    //char fname[80];
    //wsprintf(fname, "MASK%02d%02d.BMP", m_rct.right, m_rct.bottom);
    //CFile mskFile(fname, CFile::modeWrite | CFile::modeCreate);
    //CDib dib;
    //dib.BitmapToDIB(m_pMask);
    //dib.WriteDIBFile(mskFile);
    //mskFile.Close();
}

void CCellForm::FindCell(int x, int y, int& row, int& col)
{
    if (m_eType == cformHexPnt)
    {
        CRect rct;
        row = y / m_pPoly[3].y;

        int xBase = CellPhase(row) * m_pPoly[1].x;
        col = x < xBase ? -1 : (x - xBase) / m_pPoly[2].x;

        GetRect(row, col, &rct);
        g_gt.mDC1.SelectObject(m_pMask);
        COLORREF cr = g_gt.mDC1.GetPixel(x - rct.left, y - rct.top);
        g_gt.SelectSafeObjectsForDC1();
        if (cr != RGB(0, 0, 0))
        {
            if ((CellPhase(row) != 0) && (x > (rct.left + rct.right) / 2))
                col++;
            else if ((CellPhase(row) == 0) && (x < (rct.left + rct.right) / 2))
                col--;
            row--;
        }
    }
    else if (m_eType == cformHexFlat)
    {
        CRect rct;
        col = x / m_pPoly[2].x;
        int yBase = CellPhase(col) * m_pPoly[3].y;
        row = y < yBase ? -1 : (y - yBase) / m_pPoly[4].y;

        GetRect(row, col, &rct);

        g_gt.mDC1.SelectObject(m_pMask);
        COLORREF cr = g_gt.mDC1.GetPixel(x - rct.left, y - rct.top);
        g_gt.SelectSafeObjectsForDC1();
        if (cr != RGB(0, 0, 0))
        {
            if ((CellPhase(col) != 0) && (y > (rct.top + rct.bottom) / 2))
                row++;
            else if ((CellPhase(col) == 0) && (y < (rct.top + rct.bottom) / 2))
                row--;
            col--;
        }
    }
    else if (m_eType == cformRect)
    {
        row = y / m_rct.bottom;
        col = x / m_rct.right;
    }
    else if (m_eType == cformBrickHorz)
    {
        row = y / m_rct.bottom;
        int xBase = (CellPhase(row) * m_rct.right) / 2;
        col = x < xBase ? -1 : (x - xBase) / m_rct.right;
    }
    else    // m_eType == cformBrickVert
    {
        col = x / m_rct.right;
        int yBase = (CellPhase(col) * m_rct.bottom) / 2;
        row = y < yBase ? -1 : (y - yBase) / m_rct.bottom;
    }
}

// Trial calculate the size of a board to see if it exceeds
// 32k. Return FALSE if too large.

BOOL CCellForm::CalcTrialBoardSize(int nRows, int nCols)
{
    long x, y;
    if (m_eType == cformRect)
    {
        x = (long)nCols * m_rct.right;
        y = (long)nRows * m_rct.bottom;
    }
    else if (m_eType == cformBrickHorz)
    {
        x = (long)nCols * m_rct.right + m_rct.right / 2;
        y = (long)nRows * m_rct.bottom;
    }
    else if (m_eType == cformBrickVert)
    {
        x = (long)nCols * m_rct.right;
        y = (long)nRows * m_rct.bottom + m_rct.bottom / 2;
    }
    else if (m_eType == cformHexPnt)
    {
        x = (long)nCols * m_pPoly[2].x + m_pPoly[1].x;
        y = (long)nRows * m_pPoly[3].y + m_pPoly[0].y;
    }
    else
    {
        x = (long)nCols * m_pPoly[2].x + m_pPoly[1].x;
        y = (long)nRows * m_pPoly[4].y + m_pPoly[3].y;
    }
    return x < 32000 && y < 32000;
}

CSize CCellForm::CalcBoardSize(int nRows, int nCols)
{
    if (m_eType == cformRect)
        return CSize(nCols * m_rct.right, nRows * m_rct.bottom);
    else if (m_eType == cformBrickHorz)
    {
        return CSize(nCols * m_rct.right + m_rct.right / 2,
            nRows * m_rct.bottom);
    }
    else if (m_eType == cformBrickVert)
    {
        return CSize(nCols * m_rct.right,
            nRows * m_rct.bottom + m_rct.bottom / 2);
    }
    else if (m_eType == cformHexPnt)
    {
        return CSize(nCols * m_pPoly[2].x + m_pPoly[1].x,
            nRows * m_pPoly[3].y + m_pPoly[0].y);
    }
    else
    {
        return CSize(nCols * m_pPoly[2].x + m_pPoly[1].x,
            nRows * m_pPoly[4].y + m_pPoly[3].y);
    }
}

CRect* CCellForm::GetRect(int row, int col, CRect* pRct)
{
    *pRct = m_rct;          // Copy master rect
    if (m_eType == cformRect)
        pRct->OffsetRect(col * m_rct.right, row * m_rct.bottom);
    else if (m_eType == cformBrickHorz)
    {
        pRct->OffsetRect(col * m_rct.right + (CellPhase(row) * m_rct.right) / 2,
            row * m_rct.bottom);
    }
    else if (m_eType == cformBrickVert)
    {
        pRct->OffsetRect(col * m_rct.right,
            row * m_rct.bottom + (CellPhase(col) * m_rct.bottom) / 2);
    }
    else if (m_eType == cformHexPnt)
    {

        pRct->OffsetRect(col * m_pPoly[2].x + CellPhase(row) * m_pPoly[1].x,
            row * m_pPoly[3].y);
    }
    else
    {
        pRct->OffsetRect(col * m_pPoly[2].x,
            row * m_pPoly[4].y + CellPhase(col) * m_pPoly[3].y);
    }
    return pRct;
}

// The pen is assumed to be selected at this point.
void CCellForm::FrameCell(CDC* pDC, int xPos, int yPos)
{
    int nPts = (m_eType == cformRect || m_eType == cformBrickHorz ||
        m_eType == cformBrickVert) ? 5 : 7;
    memcpy(m_pWrk, m_pPoly, nPts * sizeof(POINT));
    OffsetPoly(m_pWrk, nPts, xPos, yPos);
    pDC->Polyline(m_pWrk, nPts);
}

void CCellForm::FillCell(CDC* pDC, int xPos, int yPos)
{
    CPen* pPrvPen = (CPen *)pDC->SelectStockObject(NULL_PEN);
    if (m_eType == cformRect || m_eType == cformBrickHorz ||
        m_eType == cformBrickVert)
    {
        CRect rct = m_rct;
        rct.OffsetRect(xPos, yPos);
        rct.right++;            // Adjust to include start of next rect
        rct.bottom++;
        pDC->Rectangle(&rct);
    }
    else
    {
        // Hexagonal shapes
        memcpy(m_pWrk, m_pPoly, 6 * sizeof(POINT));
        OffsetPoly(m_pWrk, 6, xPos, yPos);
        pDC->Polygon(m_pWrk, 6);
    }
}

void CCellForm::OffsetPoly(POINT* pPoly, int nPts, int xOff, int yOff)
{
    while (nPts--)
    {
        pPoly->x += xOff;
        pPoly->y += yOff;
        pPoly++;
    }
}

void CCellForm::Clear()
{
    if (m_pPoly) delete m_pPoly;
    m_pPoly = NULL;
    if (m_pWrk) delete m_pWrk;
    m_pWrk = NULL;
    if (m_pMask) delete m_pMask;
    m_pMask = NULL;
}

void CCellForm::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)m_eType;
        ar << (WORD)m_nStagger;
        ar << (short)m_rct.left;
        ar << (short)m_rct.top;
        ar << (short)m_rct.right;
        ar << (short)m_rct.bottom;
        WriteArchivePoints(ar, m_pPoly, (m_eType == cformHexFlat ||
            m_eType == cformHexPnt) ? 7 : 5);
    }
    else
    {
        WORD wVal;
        Clear();

        ar >> wVal; m_eType = (CellFormType)wVal;
        ar >> wVal; m_nStagger = (int)wVal;
        short sTmp;
        ar >> (short)sTmp; m_rct.left = sTmp;
        ar >> (short)sTmp; m_rct.top = sTmp;
        ar >> (short)sTmp; m_rct.right = sTmp;
        ar >> (short)sTmp; m_rct.bottom = sTmp;

        if (m_eType == cformHexFlat || m_eType == cformHexPnt)
        {
            m_pPoly = new POINT[7];
            m_pWrk  = new POINT[7];
            ReadArchivePoints(ar, m_pPoly, 7);
        }
        else
        {
            m_pPoly = new POINT[5];
            m_pWrk  = new POINT[5];
            ReadArchivePoints(ar, m_pPoly, 5);
        }
        CreateHexMask();
    }
}

void CCellForm::GetCellNumberStr(CellNumStyle eStyle, int row, int col,
    CString& str)
{
    int nTmp;
    char szNum1[20];
    char szNum2[20];
    str.Empty();
    switch (eStyle)
    {
        case cnsColRow:
            nTmp = row;
            row = col;
            col = nTmp;
            // Fall through to cnsRowCol processing.
        case cnsRowCol:
            _itoa(row, szNum1, 10);
            _itoa(col, szNum2, 10);
            str = szNum1;
            str += ", ";
            str += szNum2;
            break;
        case cns0101ByCols:
            nTmp = row;
            row = col;
            col = nTmp;
            // Fall through to cns0101ByRows processing.
        case cns0101ByRows:
            _itoa(row, szNum1, 10);
            _itoa(col, szNum2, 10);
            nTmp = max(strlen(szNum1), strlen(szNum2));
            nTmp = max(nTmp, 2);            // Make sure at least 2 digits
            StrLeadZeros(szNum1, nTmp);
            StrLeadZeros(szNum2, nTmp);
            str = szNum1;
            str += szNum2;
            break;
        case cnsAA01ByCols:
            nTmp = row;
            row = col;
            col = nTmp;
            // Fall through to cnsAA01ByRows processing.
        case cnsAA01ByRows:
            StrGetAAAFormat(szNum1, row);
            _itoa(col, szNum2, 10);
            str = szNum1;
            str += szNum2;
            break;
        default:;
    }
}

