// VwPrjgb1.cpp : Support file for vwprjgbx.cpp
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"
#include    "DlgGboxp.h"
#include    "DlgMkbrd.h"
#include    "DlgMakts.h"
#include    "DlgNtile.h"
#include    "DlgTsetp.h"
#include    "DlgBrdp.h"
#include    "DlgPcep.h"
#include    "DlgMrkp.h"
#include    "DlgPgrpn.h"
#include    "DlgMgrpn.h"
#include    "DlgPnew.h"
#include    "DlgPedt.h"
#include    "DlgMnew.h"
#include    "DlgMedt.h"
#include    "DlgPEditMulti.h"
#include    "DlgMEditMulti.h"
#include    "VwPrjgbx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Game Box info routine

void CGbxProjView::DoGbxProperty()
{
    GetDocument()->DoGbxProperties();
}

void CGbxProjView::DoUpdateGbxInfo()
{
    CGamDoc* pDoc = GetDocument();
    CString str;
    if (!pDoc->m_strTitle.IsEmpty())
        str += "TITLE: " + pDoc->m_strTitle + "\r\n\r\n";
    if (!pDoc->m_strAuthor.IsEmpty())
        str += "AUTHOR: " + pDoc->m_strAuthor + "\r\n\r\n";
    if (!pDoc->m_strDescr.IsEmpty())
        str += "DESCRIPTION:\r\n\r\n" + pDoc->m_strDescr;
    m_editInfo.SetWindowText(str);
}

/////////////////////////////////////////////////////////////////////
// Playing Board Support Methods

void CGbxProjView::DoBoardCreate()
{
    GetDocument()->DoCreateBoard();
}

void CGbxProjView::DoBoardProperty()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoard* pBoard = pDoc->GetBoardManager()->GetBoard(nBrd);
    ASSERT(pBoard);

    if (pDoc->DoBoardPropertyDialog(pBoard))
    {
        pDoc->SetModifiedFlag();
        DoUpdateProjectList();
    }
}

void CGbxProjView::DoBoardDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEBOARD, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        CGmBoxHint hint;
        hint.m_pBoard = pDoc->GetBoardManager()->GetBoard(nBrd);

        pDoc->GetBoardManager()->DeleteBoard(nBrd);

        // Make sure any views on the board are closed.
        pDoc->UpdateAllViews(NULL, HINT_BOARDDELETED, &hint);

        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoBoardClone()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoardManager* pBMgr = pDoc->GetBoardManager();

    ASSERT(nBrd < pBMgr->GetSize());
    CBoard* pOrigBoard = pBMgr->GetBoard(nBrd);
    if (pOrigBoard == NULL)
        return;

    CBoard* pNewBoard = NULL;
    TRY
    {
        CMemFile file;
        CArchive arSave(&file, CArchive::store);
        arSave.m_pDocument = pDoc;
        pOrigBoard->Serialize(arSave);      // Make a copy of the board
        arSave.Close();

        file.SeekToBegin();
        CArchive arRestore(&file, CArchive::load);
        arRestore.m_pDocument = pDoc;
        pNewBoard = new CBoard();
        pNewBoard->Serialize(arRestore);

        // We have a copy of the board. Now just change it's name
        // and give it a new serial number.
        CString strNewName;
        strNewName.LoadString(IDS_COPY_OF);
        strNewName += pNewBoard->GetName();
        pNewBoard->SetName(strNewName);
        pNewBoard->SetSerialNumber(pBMgr->IssueSerialNumber());

        // Add it to the board list
        pBMgr->Add(pNewBoard);
    }
    CATCH_ALL(e)
    {
        if (pNewBoard != NULL)
            delete pNewBoard;
        return;
    }
    END_CATCH_ALL

    pDoc->UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoBoardEdit()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoard* pBoard = pDoc->GetBoardManager()->GetBoard(nBrd);
    CView* pView = pDoc->FindBoardEditorView(pBoard);
    if (pView != NULL)
    {
        // This board already has an editor. Activate that view.
        CFrameWnd* pFrm = pView->GetParentFrame();
        ASSERT(pFrm);
        pFrm->ActivateFrame();
    }
    else
    {
        CString strTitle;
        m_listProj.GetItemText(nSel, strTitle);
        pDoc->CreateNewFrame(GetApp()->m_pMapViewTmpl, strTitle, pBoard);
    }
}

void CGbxProjView::DoUpdateBoardHelpInfo()
{
// m_editInfo.SetWindowText("Board Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateBoardInfo()
{
// m_editInfo.SetWindowText("Board Info Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

/////////////////////////////////////////////////////////////////////
// Tile Image Support Methods

void CGbxProjView::DoTileManagerProperty()
{
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoTileGroupCreate()
{
    GetDocument()->DoCreateTileGroup();
}

void CGbxProjView::DoTileGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CTSetPropDialog dlg;
    CTileManager* pTMgr = pDoc->GetTileManager();
    dlg.m_crTrans = pTMgr->GetTransparentColor();
    dlg.m_strName = pTMgr->GetTileSet(nGrp)->GetName();

    if (dlg.DoModal() == IDOK)
    {
        pTMgr->SetTransparentColor(dlg.m_crTrans);
        pTMgr->GetTileSet(nGrp)->SetName(dlg.m_strName);
        pDoc->UpdateAllViews(NULL, HINT_TILESETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoTileGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETETILESET, strTitle);

    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetTileManager()->DeleteTileSet(nGrp);

        // Make sure no TileID references survive
        pDoc->NotifyTileDatabaseChange();

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.m_nVal = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_TILESETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoTileNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CNewTileDialog dlg;
    dlg.m_pBMgr = pDoc->GetBoardManager();

    CTileManager* pTMgr = pDoc->GetTileManager();

    if (dlg.DoModal() == IDOK)
    {
        TileID tidNew = pTMgr->CreateTile(nGrp,
            CSize(dlg.m_nWidth, dlg.m_nHeight),
            CSize(dlg.m_nHalfWidth, dlg.m_nHalfHeight),
            RGB(255, 255, 255));
        pDoc->UpdateAllViews(NULL, MAKELPARAM(HINT_TILECREATED, tidNew), NULL);
        pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
            (LPVOID)(DWORD)tidNew);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoTileEdit()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    if (m_listTiles.GetSelCount() <= 0) return;

    CWordArray tidtbl;
    m_listTiles.GetCurMappedItemList(&tidtbl);
    CTileManager* pTMgr = pDoc->GetTileManager();

    for (int i = 0; i < tidtbl.GetSize(); i++)
    {
        TileID tid = (TileID)tidtbl[i];

        CView *pView = pDoc->FindTileEditorView(tid);
        if (pView)
        {
            // Already has an editor. Activate that view.
            CFrameWnd* pFrm = pView->GetParentFrame();
            ASSERT(pFrm);
            pFrm->ActivateFrame();
        }
        else
        {
            pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
                (LPVOID)(DWORD)tid);
        }
    }
}

void CGbxProjView::DoTileClone()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    if (m_listTiles.GetSelCount() <= 0) return;

    CWordArray tidtbl;
    m_listTiles.GetCurMappedItemList(&tidtbl);
    CTileManager* pTMgr = pDoc->GetTileManager();

    for (int i = 0; i < tidtbl.GetSize(); i++)
    {
        CTile tileFull;
        CTile tileHalf;
        CTile tileSmall;
        TileID tid = (TileID)tidtbl[i];

        pTMgr->GetTile(tid, &tileFull, fullScale);
        pTMgr->GetTile(tid, &tileHalf, halfScale);
        pTMgr->GetTile(tid, &tileSmall, smallScale);

        TileID tidNew = pTMgr->CreateTile(nGrp, tileFull.GetSize(),
            tileHalf.GetSize(), tileSmall.GetSmallColor());

        CBitmap bmap;

        tileFull.CreateBitmapOfTile(&bmap);
        pTMgr->GetTile(tidNew, &tileFull, fullScale);
        tileFull.Update(&bmap);

        tileHalf.CreateBitmapOfTile(&bmap);
        pTMgr->GetTile(tidNew, &tileHalf, halfScale);
        tileHalf.Update(&bmap);

        pDoc->UpdateAllViews(NULL, MAKELPARAM(HINT_TILECREATED, tidNew), NULL);

        pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
            (LPVOID)(DWORD)tidNew);
    }
    pDoc->SetModifiedFlag();

}

void CGbxProjView::DoTileDelete()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    if (m_listTiles.GetSelCount() <= 0) return;

    CWordArray tidtbl;
    m_listTiles.GetCurMappedItemList(&tidtbl);
    BOOL bTilesInUse = pDoc->QueryAnyOfTheseTilesInUse(tidtbl);
    if (bTilesInUse)
    {
        if (AfxMessageBox(IDS_TILEINUSE,
                MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDYES)
            return;
    }
    CTileManager* pTMgr = pDoc->GetTileManager();
    for (int i = 0; i < tidtbl.GetSize(); i++)
    {
        pTMgr->DeleteTile((TileID)tidtbl[i]);
        pDoc->UpdateAllViews(NULL, MAKELPARAM(HINT_TILEDELETED, (TileID)tidtbl[i]), NULL);
    }
    pDoc->NotifyTileDatabaseChange();
    pDoc->SetModifiedFlag();
    if (bTilesInUse)                        // (don't increase if tiles weren't used)
        pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateTileHelpInfo()
{
    // m_editInfo.SetWindowText("Tile Tips Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateTileList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CTileSet* pTSet = pDoc->GetTileManager()->GetTileSet(nGrp);
    if (pTSet == NULL)
    {
        m_listTiles.SetItemMap(NULL);
        return;
    }
    CWordArray* pLstMap = pTSet->GetTileIDTable();
    m_listTiles.SetItemMap(pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Playing Piece Support Methods

void CGbxProjView::DoPieceGroupCreate()
{
    GetDocument()->DoCreatePieceGroup();
}

void CGbxProjView::DoPieceGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceManager* pPMgr = pDoc->GetPieceManager();

    CPiecePropDialog dlg;
    dlg.m_strName = pPMgr->GetPieceSet(nGrp)->GetName();

    if (dlg.DoModal() == IDOK)
    {
        pPMgr->GetPieceSet(nGrp)->SetName(dlg.m_strName);
        pDoc->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoPieceGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEPIECESET, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetPieceManager()->DeletePieceSet(nGrp, &pDoc->GetGameStringMap());

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.m_nVal = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_PIECESETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoPieceNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceNewDialog dlg;
    dlg.m_pDoc = (CGamDoc*)GetDocument();
    dlg.m_nPSet = nGrp;

    dlg.DoModal();
    GetDocument()->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoPieceEdit()
{
    CGamDoc* pDoc = GetDocument();

    if (m_listPieces.GetSelCount() == 1)
    {
        // Do full piece edit.
        int nSel;
        m_listPieces.GetSelItems(1, &nSel);
        if (nSel < 0)
            return;
        PieceID pid = (PieceID)m_listPieces.MapIndexToItem(nSel);

        CPieceEditDialog dlg;
        dlg.m_pDoc = pDoc;
        dlg.m_pid = pid;

        if (dlg.DoModal() != IDOK)
            return;
    }
    else
    {
        // Multiple selected. Do special piece edit.
        CArray<int, int> tblSel;
        int nNumSelected = m_listPieces.GetSelCount();
        tblSel.SetSize(nNumSelected);
        m_listPieces.GetSelItems(nNumSelected, tblSel.GetData());

        CPieceEditMultipleDialog dlg;
        if (dlg.DoModal() != IDOK)
            return;

        for (int i = 0; i < tblSel.GetSize(); i++)
        {
            PieceID pid = (PieceID)m_listPieces.MapIndexToItem(tblSel[i]);
            PieceDef* pDef = pDoc->GetPieceManager()->GetPiece(pid);
            // Process "top only visible" change
            if (pDef->Is2Sided() && dlg.m_bSetTopOnlyVisible)
            {
                pDef->m_flags &= ~PieceDef::flagShowOnlyVisibleSide;      // Initially clear the flag
                pDef->m_flags &= ~PieceDef::flagShowOnlyOwnersToo;
                if (dlg.m_bTopOnlyVisible)
                {
                    pDef->m_flags |= PieceDef::flagShowOnlyVisibleSide;   // Set the flag
                    if (dlg.m_bTopOnlyOwnersToo)
                        pDef->m_flags |= PieceDef::flagShowOnlyOwnersToo; // Set this flag too
                }
            }
            // Process front piece text change
            if (dlg.m_bSetFrontText)
            {
                GameElement elem = MakePieceElement(pid, 0);
                if (!dlg.m_strFront.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strFront);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
            // Process back piece text change
            if (pDef->Is2Sided() && dlg.m_bSetBackText)
            {
                GameElement elem = MakePieceElement(pid, 1);
                if (!dlg.m_strBack.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strBack);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
        }
    }
    pDoc->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoPieceDelete()
{
    CGamDoc* pDoc = GetDocument();

    CArray<int, int> tblSel;
    int nNumSelected = m_listPieces.GetSelCount();
    if (nNumSelected == 0)
        return;

    tblSel.SetSize(nNumSelected);
    m_listPieces.GetSelItems(nNumSelected, tblSel.GetData());

    for (int i = 0; i < tblSel.GetSize(); i++)                  // Map them all to piece ID's
        tblSel[i] = m_listPieces.MapIndexToItem(tblSel[i]);

    m_listPieces.SetItemMap(NULL);
    for (int i = 0; i < tblSel.GetSize(); i++)
        pDoc->GetPieceManager()->DeletePiece((PieceID)tblSel[i], &pDoc->GetGameStringMap());

    pDoc->UpdateAllViews(NULL, HINT_PIECEDELETED, NULL);
    pDoc->SetModifiedFlag();
    pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdatePieceHelpInfo()
{
    // m_editInfo.SetWindowText("Piece Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdatePieceList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceSet* pPSet = pDoc->GetPieceManager()->GetPieceSet(nGrp);
    if (pPSet == NULL)
    {
        m_listPieces.SetItemMap(NULL);
        return;
    }
    CWordArray* pLstMap = pPSet->GetPieceIDTable();
    m_listPieces.SetItemMap(pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Marker Support Methods

void CGbxProjView::DoMarkGroupCreate()
{
    GetDocument()->DoCreateMarkGroup();
}

void CGbxProjView::DoMarkGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkManager* pMMgr = pDoc->GetMarkManager();
    CMarkSet* pMSet = pMMgr->GetMarkSet(nGrp);

    CMarkerPropDialog dlg;
    dlg.m_strName = pMSet->GetName();
    dlg.m_nMarkerViz = (int)pMSet->GetMarkerTrayContentVisibility(); // zero based enum

    if (dlg.DoModal() == IDOK)
    {
        pMSet->SetName(dlg.m_strName);
        pMSet->SetMarkerTrayContentVisibility((MarkerTrayViz)dlg.m_nMarkerViz);

        pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoMarkGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEMARKSET, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetMarkManager()->DeleteMarkSet(nGrp, &pDoc->GetGameStringMap());

        CGmBoxHint hint;
        hint.m_nVal = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_MARKSETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoMarkNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkerCreateDialog dlg;
    dlg.m_pDoc = (CGamDoc*)GetDocument();
    dlg.m_nMSet = nGrp;

    dlg.DoModal();
    pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoMarkEdit()
{
    CGamDoc* pDoc = GetDocument();

    if (m_listMarks.GetSelCount() == 1)
    {
        // Do full marker edit.
        int nSel;
        m_listMarks.GetSelItems(1, &nSel);
        if (nSel < 0)
            return;
        MarkID mid = (MarkID)m_listMarks.MapIndexToItem(nSel);

        CMarkerEditDialog dlg;
        dlg.m_pDoc = (CGamDoc*)GetDocument();
        dlg.m_mid = mid;

        if (dlg.DoModal() != IDOK)
            return;
    }
    else
    {
        // Multiple selected. Do special marker edit.
        CArray<int, int> tblSel;
        int nNumSelected = m_listMarks.GetSelCount();
        tblSel.SetSize(nNumSelected);
        m_listMarks.GetSelItems(nNumSelected, tblSel.GetData());

        CMarkerEditMultipleDialog dlg;
        if (dlg.DoModal() != IDOK)
            return;

        for (int i = 0; i < tblSel.GetSize(); i++)
        {
            MarkID mid = (MarkID)m_listMarks.MapIndexToItem(tblSel[i]);
            MarkDef* pDef = pDoc->GetMarkManager()->GetMark(mid);
            // Process "prompt for text" change
            if (dlg.m_bSetPromptForText)
            {
                pDef->m_flags &= ~MarkDef::flagPromptText;          // Initially clear the flag
                if (dlg.m_bPromptForText)
                    pDef->m_flags |= MarkDef::flagPromptText;       // Set the flag
            }
            // Process marker text change
            if (dlg.m_bSetText)
            {
                GameElement elem = MakeMarkerElement(mid);
                if (!dlg.m_strText.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strText);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
        }
    }
    pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoMarkDelete()
{
    CGamDoc* pDoc = GetDocument();

    CArray<int, int> tblSel;
    int nNumSelected = m_listMarks.GetSelCount();
    if (nNumSelected == 0)
        return;

    tblSel.SetSize(nNumSelected);
    m_listMarks.GetSelItems(nNumSelected, tblSel.GetData());

    for (int i = 0; i < tblSel.GetSize(); i++)                  // Map them all to piece ID's
        tblSel[i] = m_listMarks.MapIndexToItem(tblSel[i]);

    m_listMarks.SetItemMap(NULL);
    for (int i = 0; i < tblSel.GetSize(); i++)
        pDoc->GetMarkManager()->DeleteMark((MarkID)tblSel[i], &pDoc->GetGameStringMap());

    pDoc->UpdateAllViews(NULL, HINT_MARKERDELETED, NULL);
    pDoc->SetModifiedFlag();
    pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateMarkHelpInfo()
{
    // m_editInfo.SetWindowText("Marker Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateMarkList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkSet* pMSet = pDoc->GetMarkManager()->GetMarkSet(nGrp);
    if (pMSet == NULL)
    {
        m_listMarks.SetItemMap(NULL);
        return;
    }
    CWordArray* pLstMap = pMSet->GetMarkIDTable();
    m_listMarks.SetItemMap(pLstMap);
}
