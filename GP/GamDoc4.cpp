//  GamDoc4.cpp - various game playback support routines
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
#include    "Gp.h"
#include    "FrmMain.h"
#include    "GamDoc.h"
#include    "GameBox.h"
#include    "GMisc.h"
#include    "StrLib.h"

#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Marks.h"
#include    "Trays.h"
#include    "MoveMgr.h"
#include    "GamState.h"
#include    "MoveHist.h"
#include    "DlgState.h"
#include    "VwPbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::LoadAndActivateMoveFile(LPCSTR pszPathName)
{
    ASSERT(m_pRcdMoves == NULL);    // Should be gone at this point!

    CFile file;
    CFileException fe;

    if (!file.Open(pszPathName, CFile::modeRead | CFile::shareExclusive, &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, pszPathName);
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    CHistRecord* pHist = NULL;

    TRY
    {
        SerializeMoveSet(ar, pHist);

        ar.Close();
        file.Close();

        // The move data has loaded without problems.
        // Check that all pieces contained in moves still exist
        // in the GameBox. If not we can't use the move file.

        if (!pHist->m_pMList->ValidatePieces(this))
        {
            AfxMessageBox(IDS_ERR_MISSINGPIECES, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowUserException();
        }

        // Insert the current state of the game in front of the
        // move list. This allows us to discard the moves if desired.

        CGameState* pState = new CGameState(this);
        if (!pState->SaveState())
        {
            AfxMessageBox(IDS_ERR_FAILEDSTATESAVE, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowUserException();
        }

        CGameStateRcd* pCurRcd = new CGameStateRcd(pState);
        pCurRcd->SetSeqNum(-1);             // Special number
        pHist->m_pMList->PrependMoveRecord(pCurRcd, FALSE);

        // Check if the move file contains a state record.

        POSITION pos = pHist->m_pMList->FindIndex(1);
        ASSERT(pos != NULL);
        CGameStateRcd* pRcd = (CGameStateRcd*)pHist->m_pMList->GetAt(pos);
        BOOL bUpdatedGameState = FALSE;

        if (pRcd->GetType() != CMoveRecord::mrecState)
        {
            AfxMessageBox(IDS_MSG_NOGAMESTATE, MB_OK | MB_ICONINFORMATION);
            m_nFirstMove = 0;           // Restart location (state rec)
            m_nCurMove = 1;             // Set to start of actual moves
        }
        else
        {
            // See if the file's game state matches the current game
            // state. If not, give option to update game to file's state.
            pRcd->GetGameState()->SetDocument(this);
            if (!pRcd->GetGameState()->CompareState())
            {
                // File's positions don't match current games' positions.
                CSelectStateDialog dlg;
                dlg.m_nState = 0;           // Select first button (file state)
                if (dlg.DoModal() != IDOK)
                    AfxThrowUserException();
                if (dlg.m_nState == 0)
                {
                    // Use the current game state
                    pRcd->GetGameState()->RestoreState();
                    // Make sure we account for possible deletions
                    // of pieces in move file (for crash resistance)
                    GetPieceTable()->PurgeUndefinedPieceIDs();

                    bUpdatedGameState = TRUE;
                    // File's positions match current positions.
                    m_nFirstMove = 1;       // Restart location (state rec)
                    m_nCurMove = 2;         // Set to start of actual moves
                }
                else
                {
                    // Using current board state. Discard the move file's
                    // game state and replace with the current state of the game.
                    pHist->m_pMList->RemoveAt(pos); // Delete file game state
                    delete pRcd;
                    // Create a current state object
                    POSITION pos = pHist->m_pMList->FindIndex(0);
                    CGameState* pState = new CGameState(this);
                    pState->SaveState();
                    CGameStateRcd* pCurRcd = new CGameStateRcd(pState);
                    pCurRcd->SetSeqNum(0);
                    pHist->m_pMList->InsertAfter(pos, pCurRcd);
                    m_nFirstMove = 1;       // Restart location (state rec)
                    m_nCurMove = 2;         // Set to start of actual moves
                    // m_nFirstMove = 0;        // Restart location (state rec)
                    // m_nCurMove = 1;          // Set to start of actual moves
                }
            }
            else
            {
                // File's positions match current positions.
                m_nFirstMove = 1;           // Restart location (state rec)
                m_nCurMove = 2;             // Set to start of actual moves
            }
        }

        // Set game state and vars for playback.
        m_eState = stateMovePlay;
        m_pRcdMoves = pHist->m_pMList;
        m_pMoves = pHist->m_pMList;         // Shadow for playback
        m_pPlayHist = pHist;                // Save for storing in history table
        // Decouple the list since it's held by m_pRcdMoves. If we
        // don't do this the move list will be deleted twice.
        m_pPlayHist->m_pMList = NULL;

        // Final check if there are any possible moves. It might be that
        // a move file with no moves is being loaded.
        if (!m_pMoves->IsThisMovePossible(m_nCurMove))
        {
            m_nCurMove = -1;                // No moves in move table
            AfxMessageBox(IDS_MSG_NOMOVES, MB_OK);
        }

        MsgDialogCancel(TRUE);

        if (bUpdatedGameState)
            UpdateAllViews(NULL, HINT_GAMESTATEUSED);
        else
            UpdateAllViews(NULL, HINT_GAMPROPCHANGE);
    }
    CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
        if (pHist != NULL) delete pHist;
        return;
    }
    END_CATCH_ALL
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::LoadAndActivateHistory(int nHistRec)
{
    ASSERT(m_file.m_hFile != NULL);
    ASSERT(m_pHistTbl != NULL);

    // Dump all select lists and move plot lists.
    FlushAllIndicators();

    ASSERT(nHistRec < m_pHistTbl->GetNumHistRecords());
    CHistRecord* pHist = m_pHistTbl->GetHistRecord(nHistRec);
    ASSERT(pHist != NULL);
    CMoveList* pMoves = pHist->m_pMList;
    ASSERT(pMoves != NULL);

    TRY
    {
        // The move data has loaded without problems.
        // Check that all pieces contained in moves still exist
        // in the GameBox. If not we can't use the move file.

        if (!pMoves->ValidatePieces(this))
        {
            AfxMessageBox(IDS_ERR_MISSINGPIECES, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowUserException();
        }

        // Set game state and vars for playback.        // (moved earlier in processing)
        m_eState = stateHistPlay;
        m_nCurHist = nHistRec;                  // Set number of history playback
        // m_pHistMoves = pHist->m_pMList;      // Set pointer to history moves
        // Make a copy of the move list for playback
        m_pHistMoves = CMoveList::CloneMoveList(this, pHist->m_pMList);
        m_pMoves = m_pHistMoves;            // Shadow for playback

        // Insert the current state of the game in front of the
        // move list. This allows us to discard the moves if desired.

        CGameState* pState = new CGameState(this);
        if (!pState->SaveState())
        {
            AfxMessageBox(IDS_ERR_FAILEDSTATESAVE, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowUserException();
        }

        CGameStateRcd* pCurRcd = new CGameStateRcd(pState);
        pCurRcd->SetSeqNum(-1);             // Special number
        m_pMoves->PrependMoveRecord(pCurRcd, FALSE);

        // The history moveset always contains a state record so
        // position to it.

        POSITION pos = m_pMoves->FindIndex(1);
        ASSERT(pos != NULL);
        CGameStateRcd* pRcd = (CGameStateRcd*)m_pMoves->GetAt(pos);
        ASSERT(pRcd->GetType() == CMoveRecord::mrecState);

        // Use the history game state
        pRcd->GetGameState()->RestoreState();

        // Make sure we account for possible deletions
        // of pieces in move file (for crash resistance)
        GetPieceTable()->PurgeUndefinedPieceIDs();

        m_nFirstMove = 1;       // Restart location (state rec)
        m_nCurMove = 2;         // Set to start of actual moves

        MsgDialogCancel(TRUE);

        UpdateAllViews(NULL, HINT_GAMESTATEUSED);
    }
    CATCH_ALL(e)
    {
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::LoadVintageHistoryMoveLists(CFile& file)
{
    ASSERT(file.m_hFile != NULL);
    ASSERT(m_pHistTbl != NULL);

    for (int nHistRec = 0; nHistRec < m_pHistTbl->GetNumHistRecords(); nHistRec++)
    {
        CHistRecord* pHist = m_pHistTbl->GetHistRecord(nHistRec);
        if (!LoadVintageHistoryRecord(file, pHist))
            return FALSE;
    }
    SetLoadingVersion(NumVersion(fileGamVerMajor, fileGamVerMinor));
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::LoadVintageHistoryRecord(CFile& file, CHistRecord* pHist)
{
    ASSERT(file.m_hFile != NULL);
    ASSERT(pHist != NULL);

    CMoveList* pMoves = NULL;

    TRY
    {
        ASSERT(pHist->m_dwFilePos > 0);
        file.Seek(pHist->m_dwFilePos, CFile::begin);
        CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
        ar.m_pDocument = this;
        ar.m_bForceFlat = FALSE;

        pMoves = new CMoveList;

        // HACKO----RAMA!!!!!!! //
        SetLoadingVersion(pHist->m_nGamFileVersion);
        BOOL bTryEarlyVersion = FALSE;
        TRY
            pMoves->Serialize(ar);
        CATCH_ALL (e)
            // First exception we assume it is due to
            // a version problem in the format of the
            // saved history data. This is a really perverted
            // way to infer an archive version since I was
            // too stupid to store this information in earlier
            // versions of CyberBoard.
            bTryEarlyVersion = TRUE;
        END_CATCH_ALL

        ar.Close();
        if (bTryEarlyVersion)
        {
            file.Seek(pHist->m_dwFilePos, CFile::begin);
            CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
            ar.m_pDocument = this;
            ar.m_bForceFlat = FALSE;
            SetLoadingVersion(NumVersion(0, 57));
            pMoves->Serialize(ar);
            ar.Close();
            // A-OK set version to proper value.
            pHist->m_nGamFileVersion = NumVersion(0, 57);
        }

        // The move data has loaded without problems.
        // Check that all pieces contained in moves still exist
        // in the GameBox. If not we can't use the move file.

        if (!pMoves->ValidatePieces(this))
        {
            AfxMessageBox(IDS_ERR_MISSINGPIECES, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowUserException();
        }

        // Make sure we account for possible deletions
        // of pieces in move file (for crash resistance)
        GetPieceTable()->PurgeUndefinedPieceIDs();

        pHist->m_dwFilePos = 0;             // Just to be clean. Never used again
        pHist->m_pMList = pMoves;
    }
    CATCH_ALL(e)
    {
        if (pMoves != NULL) delete pMoves;
        SetLoadingVersion(NumVersion(fileGamVerMajor, fileGamVerMinor));
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::FinishHistoryPlayback()
{
    // Dump all select lists and move plot lists.
    FlushAllIndicators();

    // Get the first record which contains the state of the board
    // prior to playback. Restore it

    CGameStateRcd* pMove = (CGameStateRcd*)m_pHistMoves->RemoveHead();
    ASSERT(pMove != NULL);
    ASSERT(pMove->GetType() == CMoveRecord::mrecState);

    if (!pMove->GetGameState()->RestoreState())
    {
        AfxMessageBox(IDS_ERR_FAILEDRESTORE, MB_OK | MB_ICONEXCLAMATION);
    }

    delete pMove;                   // Destroy the game state record.

    // Delete the history  move list since it is a clone of the actual record
    // kept in the history database.
    delete m_pHistMoves;
    m_pHistMoves = NULL;
    m_nCurHist = -1;
    m_eState = stateRecording;
    m_pMoves = m_pRcdMoves;         // Restore recording pointer

    MsgDialogCancel(TRUE);
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::RestartMoves()
{
    ASSERT(IsPlaying());
    ASSERT(m_nFirstMove >= 0 && m_nFirstMove <= 1);
    m_nMoveInterlock++;
    m_nCurMove = m_pMoves->DoMove(this, m_nFirstMove);
    m_nMoveInterlock--;
    ASSERT(m_nCurMove != -1 && m_nCurMove != m_nFirstMove);
    m_astrMsgHist.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::TransferPlaybackToHistoryTable(BOOL bTruncateAtCurrentMove /* = FALSE */)
{
    FlushAllIndicators();

    m_nMoveInterlock++;
    if (bTruncateAtCurrentMove)
    {
        m_pMoves->PurgeAfter(m_nCurMove);
        // Append TRUNCATED! to description
        CString str;
        str.LoadString(IDS_TRUNCATED);
        m_pPlayHist->m_strTitle += str;
    }
    else
    {
        // Process to the last record.
        m_bQuietPlayback = TRUE;
        while ((m_nCurMove = m_pMoves->DoMove(this, m_nCurMove)) != -1) ;
        FlushAllIndicators();
        m_bQuietPlayback = FALSE;
    }
    m_nMoveInterlock--;
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);

    m_pPlayHist->m_timeAbsorbed = CTime::GetCurrentTime();

    m_pPlayHist->m_pMList = m_pRcdMoves;        // Take ownership of move list

    AddMovesToGameHistoryTable(m_pPlayHist);
    m_pPlayHist = NULL;
    SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::EnsureBoardLocationVisible(CPlayBoard* pPBoard, CPoint point)
{
    if (IsQuietPlayback()) return;
    EnsureBoardVisible(pPBoard);

    // Use hint to scroll board to make point visible somewhere
    // near the center of the view.
    CGamDocHint hint;
    hint.m_pPBoard = pPBoard;
    hint.m_point = point;
    UpdateAllViews(NULL, HINT_POINTINVIEW, &hint);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::EnsureBoardVisible(CPlayBoard* pPBoard)
{
    if (IsQuietPlayback()) return;

    // If board isn't visible, open it.
    MakeSurePBoardVisible(pPBoard);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::EnsureTrayIndexVisible(CTraySet* pYSet, int nPos)
{
    if (IsQuietPlayback()) return;
    if (!m_bTrayAVisible)
        OnViewTrayA();

    // Make sure item nPos is visible.
    int nGroup = GetTrayManager()->FindTrayByPtr(pYSet);
    ASSERT(nGroup >= 0);
    m_palTrayA.ShowTrayIndex(nGroup, nPos);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SelectObjectOnBoard(CPlayBoard* pPBoard, CDrawObj* pObj)
{
    if (IsQuietPlayback()) return;
    // If board isn't visible, open it.
    MakeSurePBoardVisible(pPBoard);

    // Use hint to add object to select list
    CGamDocHint hint;
    hint.m_pPBoard = pPBoard;
    hint.m_pDrawObj = pObj;
    UpdateAllViews(NULL, HINT_SELECTOBJ, &hint);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SelectObjectListOnBoard(CPlayBoard* pPBoard, CPtrList* pList)
{
    if (IsQuietPlayback()) return;
    // If board isn't visible, open it.
    MakeSurePBoardVisible(pPBoard);

    // Use hint to add object to select list
    CGamDocHint hint;
    hint.m_pPBoard = pPBoard;
    hint.m_pPtrList = pList;
    UpdateAllViews(NULL, HINT_SELECTOBJLIST, &hint);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SelectTrayItem(CTraySet* pYSet, PieceID pid, UINT nResourceID)
{
    CString str;
    str.LoadString(nResourceID);
    SelectTrayItem(pYSet, pid, str);
}

void CGamDoc::SelectTrayItem(CTraySet* pYSet, PieceID pid,
    LPCTSTR pszNotificationTip /* = NULL */)
{
    if (IsQuietPlayback()) return;
    // Make sure tray palette A is visible
    if (!m_bTrayAVisible)
        OnViewTrayA();

    // Select the piece in the appropriate trayset.
    int nGroup = GetTrayManager()->FindTrayByPtr(pYSet);
    ASSERT(nGroup >= 0);
    m_palTrayA.SelectTrayPiece(nGroup, pid, pszNotificationTip);
}

void CGamDoc::SelectMarkerPaletteItem(MarkID mid)
{
    if (IsQuietPlayback()) return;
    // Make sure marker palette s visible
    if (!m_bMarkPalVisible)
        OnViewMarkPalette();
    // Select the marker in the appropriate group.
    m_palMark.SelectMarker(mid);
}

/////////////////////////////////////////////////////////////////////////////

CView* CGamDoc::MakeSurePBoardVisible(CPlayBoard* pPBoard)
{
    if (IsQuietPlayback()) return NULL;

    CView* pView = FindPBoardView(pPBoard);
    if (pView != NULL)
    {
        // This board already has a view. Activate that view.
        CFrameWnd* pFrm = pView->GetParentFrame();
        ASSERT(pFrm);
        pFrm->ActivateFrame();
    }
    else
    {
        CreateNewFrame(GetApp()->m_pBrdViewTmpl,
            pPBoard->GetBoard()->GetName(), pPBoard);
        pView = FindPBoardView(pPBoard);
    }
    return pView;
}
/////////////////////////////////////////////////////////////////////////////

const int borderWidth = 3;

void CGamDoc::IndicateBoardToBoardPieceMove(CPlayBoard* pPBFrom,
    CPlayBoard* pPBTo, CPoint ptCtrFrom, CPoint ptCtrTo, CSize size)
{
    if (IsQuietPlayback()) return;
    IndicateBoardPiece(pPBFrom, ptCtrFrom, size);
    IndicateBoardPiece(pPBTo, ptCtrTo, size);

    // If on same board and move is not using plotted move mode.
    if (pPBFrom == pPBTo && !pPBFrom->GetPlotMoveMode())
        IndicateBoardPlotLine(pPBFrom, ptCtrFrom, ptCtrTo);
}

void CGamDoc::IndicateBoardPlotLine(CPlayBoard* pPBrd, CPoint ptA, CPoint ptB)
{
    if (IsQuietPlayback()) return;
    CLine* pObj = new CLine;
    pObj->SetForeColor(pPBrd->m_crPlotLineColor);
    pObj->SetLineWidth(pPBrd->m_nPlotLineWidth);        // Must set width first!
    pObj->SetLine(ptA.x, ptA.y, ptB.x, ptB.y);

    pPBrd->AddIndicatorObject(pObj);

    CGamDocHint hint;
    hint.m_pPBoard = pPBrd;
    hint.m_pDrawObj = pObj;
    UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
}

void CGamDoc::IndicateBoardPiece(CPlayBoard* pPBrd, CPoint ptCtr, CSize size)
{
    if (IsQuietPlayback()) return;
    CRect rct(CPoint(0, 0), size);
    CSize offset(size.cx / 2, size.cy / 2);
    rct += ptCtr - offset;
    rct.InflateRect(borderWidth + 1, borderWidth + 1);
    CRectObj* pObj = new CRectObj;
    pObj->SetRect(&rct);
    pObj->SetForeColor(pPBrd->m_crPlotLineColor);
    pObj->SetBackColor(noColor);
    pObj->SetLineWidth(borderWidth);

    pPBrd->AddIndicatorObject(pObj);

    CGamDocHint hint;
    hint.m_pPBoard = pPBrd;
    hint.m_pDrawObj = pObj;
    UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
}

// Shows a balloon tip so person knows what happened. Uses a resource ID.
void CGamDoc::IndicateTextTipOnBoard(CPlayBoard* pPBoard,
    CPoint pointWorkspace, UINT nResID)
{
    CString str;
    str.LoadString(nResID);
    IndicateTextTipOnBoard(pPBoard, pointWorkspace, str);
}

// Shows a balloon tip so person knows what happened.
void CGamDoc::IndicateTextTipOnBoard(CPlayBoard* pPBoard,
    CPoint pointWorkspace, LPCTSTR pszStr)
{
    if (IsQuietPlayback()) return;
    CPlayBoardView* pView = (CPlayBoardView*)FindPBoardView(pPBoard);
    ASSERT(pView != NULL);
    pView->WorkspaceToClient(pointWorkspace);
    pView->SetNotificationTip(pointWorkspace, pszStr);
}

void CGamDoc::FlushAllSelections()
{
    m_palTrayA.DeselectAll();
    m_palTrayB.DeselectAll();

    // Use hint to flush select lists.
    CGamDocHint hint;
    hint.m_pPBoard = NULL;
    hint.m_pDrawObj = NULL;
    UpdateAllViews(NULL, HINT_SELECTOBJ, &hint);
}

void CGamDoc::FlushAllIndicators()
{
    m_palTrayA.DeselectAll();
    m_palTrayB.DeselectAll();

    for (int i = 0; i < m_pPBMgr->GetNumPBoards(); i++)
    {
        CPlayBoard* pPBrd = m_pPBMgr->GetPBoard(i);
        ASSERT(pPBrd != NULL);

        UpdateAllBoardIndicators(pPBrd);

        CDrawList* pDwg = pPBrd->GetIndicatorList();
        ASSERT(pDwg != NULL);

        pDwg->Flush();

        pPBrd->SetPlotMoveMode(FALSE);
    }
    // Use hint to flush select lists.
    CGamDocHint hint;
    hint.m_pPBoard = NULL;
    hint.m_pDrawObj = NULL;
    UpdateAllViews(NULL, HINT_SELECTOBJ, &hint);
    // Use hint to flush tool tip indicators
    UpdateAllViews(NULL, HINT_CLEARINDTIP, &hint);
}

void CGamDoc::UpdateAllBoardIndicators(CPlayBoard* pPBrd)
{
    if (IsQuietPlayback()) return;
    CDrawList* pDwg = pPBrd->GetIndicatorList();
    ASSERT(pDwg != NULL);

    POSITION pos;
    for (pos = pDwg->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pDwg->GetNext(pos);
        ASSERT(pObj);
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
}
