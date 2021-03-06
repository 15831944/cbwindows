// GmDoc.h : interface of the CGamDoc class
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

/////////////////////////////////////////////////////////////////////////////

#ifndef _GMDOC_H
#define _GMDOC_H

#ifndef     _FONT_H
#include    "Font.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _BOARD_H
#include    "Board.h"
#endif

#ifndef     _PALTILE_H
#include    "PalTile.h"
#endif

#ifndef     _MAPSTRNG_H
#include    "MapStrng.h"
#endif

//////////////////////////////////////////////////////////////////////

#define     KEY_PASS_POSTFIX        "BaDkArMa"

//////////////////////////////////////////////////////////////////////
// Hints for UpdateAllViews/OnUpdate

#define     HINT_ALWAYSUPDATE       0       // Must be zero!

#define     HINT_TILECREATED        1       // HIWORD = TileID
#define     HINT_TILEMODIFIED       2       // HIWORD = TileID
#define     HINT_TILEDELETED        3       // HIWORD = TileID
#define     HINT_TILEGROUP          0x0F    // Mask for all tile hints

#define     HINT_BOARDDELETED       0x10    // pHint->m_pBoard;
#define     HINT_TILESETDELETED     0x11    // pHint->m_nVal = tset num
#define     HINT_PIECESETDELETED    0x12    // pHint->m_nVal = pset num
#define     HINT_MARKSETDELETED     0x13    // pHint->m_nVal = mset num
#define     HINT_PIECEDELETED       0x14
#define     HINT_MARKERDELETED      0x15

#define     HINT_DELETEGROUP        0x10    // Set if delete hint group

#define     HINT_TILESETPROPCHANGE  0x20
#define     HINT_BOARDPROPCHANGE    0x21    // pHint->m_pBoard
#define     HINT_PIECESETPROPCHANGE 0x22
#define     HINT_MARKERSETPROPCHANGE 0x23

#define     HINT_FORCETILEUPDATE    0x40    // Used before a save is done

#define     HINT_UPDATEPROJVIEW     0x0100  // Used to reload prject window

class CGmBoxHint : public CObject
{
    DECLARE_DYNCREATE(CGmBoxHint);
public:
    union
    {
        void*       m_pVoid;
        CBoard*     m_pBoard;
        int         m_nVal;
    };
};

//////////////////////////////////////////////////////////////////////

class CDib;
class CPieceManager;
class CMarkManager;

class CGamDoc : public CDocument
{
    friend class CGbxProjView;

protected:
    CGamDoc();
    DECLARE_DYNCREATE(CGamDoc)

// Class Global Attributes
public:
    static CFontTbl m_fontTbl;
    static CFontTbl* GetFontManager() { return &m_fontTbl; }
    // Current Tile Manager. Only valid when Serializing
    static CTileManager* c_pTileMgr;
    // Version of file being loaded
    static int c_fileVersion;

// Attributes
public:
    void ExportGamebox(LPCSTR pszPathName);

    // Current Tile Manager. Only valid when Serializing
    static CTileManager* GetCurrentTileManager() { return c_pTileMgr; }
    static void SetLoadingVersion(int ver) { c_fileVersion = ver; }
    static int GetLoadingVersion() { return c_fileVersion; }
    // -------- //
    DWORD GetGameBoxID() { return m_dwGameID; }
    // -------- //
    CBoardManager* GetBoardManager() { return m_pBMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }
    CPieceManager* GetPieceManager() { return m_pPMgr; }
    CMarkManager* GetMarkManager() { return m_pMMgr; }
    CTilePalette* GetTilePalWnd() { return &m_palTile; }
    // -------- //
    BOOL GetStickyDrawTools() { return m_bStickyDrawTools; }
    // -------- //
    void IncrMajorRevLevel();
    DWORD IssueGameBoxID();

    // If you need to pass a pointer to the views to be created,
    // bracket the view can call the GetCreateParameter() method.
    // It is only valid during the InitialUpdate() method.
    LPVOID GetCreateParameter() { return m_lpvCreateParam; }

// Operations
public:
    BOOL SetupBlankBoard();

    BOOL CreateNewFrame(CDocTemplate* pTemplate, LPCSTR pszTitle,
        LPVOID lpvCreateParam = NULL);

    BOOL NotifyTileDatabaseChange(BOOL bPurgeScan = TRUE);
    BOOL PurgeMissingTileIDs();
    BOOL QueryTileInUse(TileID tid);
    BOOL QueryAnyOfTheseTilesInUse(CWordArray& tbl);

    TileID CreateTileFromDib(CDib* pDib, int nTSet);
    CView* FindTileEditorView(TileID tid);
    CView* FindBoardEditorView(CBoard* pBoard);

    // Support for strings associated with game elements (pieces, markers)
    CString     GetGameElementString(GameElement gelem);
    BOOL        HasGameElementString(GameElement gelem);

    BOOL DoBoardPropertyDialog(CBoard* pBoard);

    void DoGbxProperties() { OnEditGbxProperties(); }
    void DoCreateBoard() { OnEditCreateBoard(); }
    void DoCreateTileGroup() { OnEditCreateTileGroup(); }
    void DoCreatePieceGroup() { OnEditCreatePieceGroup(); }
    void DoCreateMarkGroup() { OnEditCreateMarkGroup(); }

    int  GetCompressLevel() { return (int)m_wCompressLevel; }
    void SetCompressLevel(int nCompressLevel) { m_wCompressLevel = (WORD)nCompressLevel; }

    LPVOID GetCustomColors();
    void   SetCustomColors(LPVOID pCustColors);

    CGameElementStringMap& GetGameStringMap() { return m_mapStrings; }

    void ComputeGameboxPasskey(LPCTSTR pszPassword, LPBYTE pBfr);
    void ClearGameboxPasskey();

    void OnFileClose() { CDocument::OnFileClose(); }    // Expose protected

    // OnIdle is called by the App object to inform
    // documents of idle condition. A flag indicates if
    // this is the active document.
    void OnIdle(BOOL bActive);

    // Forced override of this (note not virtual)
    void UpdateAllViews(CView* pSender, LPARAM lHint = 0L,
        CObject* pHint = NULL);

// Implementation
protected:
    BYTE            m_abyteBoxID[16];// Special hashed UUID assigned to this gamebox.
    WORD            m_wReserved1;   // For future need (set to 0)
    WORD            m_wReserved2;   // For future need (set to 0)
    WORD            m_wReserved3;   // For future need (set to 0)
    WORD            m_wReserved4;   // For future need (set to 0)
    WORD            m_nBitsPerPixel;// Geometry of bitmaps (4bpp or 8bpp)
    DWORD           m_dwMajorRevs;  // Major revisions (stuff was deleted)
    DWORD           m_dwMinorRevs;  // Minor revisions (stuff was added)
    DWORD           m_dwGameID;     // Unique autogened signature for gamebox
    CString         m_strAuthor;    // Game box author (<= 32 chars)
    CString         m_strTitle;     // Game box tile (<= 64 chars)
    CString         m_strDescr;     // Game box description (<= 64 chars)
    BYTE            m_abytePass[16];// MD5 Hash of password and uuid based box ID
    WORD            m_wCompressLevel;// Amount of compression to apply to bitmaps and such
    CGameElementStringMap m_mapStrings; // Mapping of pieces and markers to strings.
    LPVOID          m_pCustomColors; // Container for custom edit colors
    CTileManager*   m_pTMgr;        // Tiles
    CBoardManager*  m_pBMgr;        // Playing boards
    CPieceManager*  m_pPMgr;        // Playing pieces
    CMarkManager*   m_pMMgr;        // Annotation markers
    BOOL            m_bStickyDrawTools; // If TRUE, don't select the select tool after drawing

    BOOL            m_bMajorRevIncd;// Major rev number was increased.

    CTilePalette    m_palTile;      // Tile palette child window is in document

    LPVOID  m_lpvCreateParam;       // Used to pass parameters to new views

public:
    virtual ~CGamDoc();
    virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(const char* pszPathName);
    virtual void DeleteContents();
    virtual void OnCloseDocument();

// Generated message map functions
protected:
    afx_msg void OnEditGbxProperties();
    afx_msg void OnEditCreateBoard();
    afx_msg void OnEditCreateTileGroup();
    afx_msg void OnEditCreatePieceGroup();
    afx_msg void OnEditCreateMarkGroup();
    afx_msg void OnProjectChangeFingerPrint();
    afx_msg void OnStickyDrawTools();
    afx_msg void OnUpdateStickyDrawTools(CCmdUI* pCmdUI);
    afx_msg void OnDumpTileData();
    afx_msg void OnBugFixDumpBadTiles();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnExportGamebox();
};

#endif

