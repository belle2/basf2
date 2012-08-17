#ifndef SPLITGLVIEW_H
#define SPLITGLVIEW_H

#include "TApplication.h"
#include "TSystem.h"
#include "TGFrame.h"
#include "TGLayout.h"
#include "TGSplitter.h"
#include "TGLWidget.h"
#include "TEvePad.h"
#include "TGeoManager.h"
#include "TString.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGFileDialog.h"
#include "TGMsgBox.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "HelpText.h"
#include "TClass.h"
#include "Riostream.h"
#include "TGListTree.h"
#include "TOrdCollection.h"
#include "TArrayF.h"
#include "TGHtml.h"
#include "TPRegexp.h"

#include "TEveManager.h"
#include "TEveViewer.h"
#include "TEveBrowser.h"
#include "TEveProjectionManager.h"
#include "TEveProjectionAxes.h"
#include "TEveScene.h"
#include "TEveGeoNode.h"
#include "TEveEventManager.h"
#include "TEveTrack.h"
#include "TEveSelection.h"

#include "TGSplitFrame.h"
#include "TGLOverlayButton.h"
#include "TGLEmbeddedViewer.h"
#include "TGDockableFrame.h"
#include "TGShapedFrame.h"
#include "TGButton.h"
#include "TGTab.h"
#include "TEnv.h"

namespace Belle2 {

  /*
  class HtmlObjTable : public TObject {
    public:                     // make them public for shorter code

      TString   fName;
      Int_t     fNValues;      // number of values
      Int_t     fNFields;      // number of fields
      TArrayF*  fValues;
      TString*  fLabels;
      Bool_t    fExpand;

      TString   fHtml;         // HTML output code

      void Build();
      void BuildTitle();
      void BuildLabels();
      void BuildTable();

    public:
      HtmlObjTable(const char* name, Int_t nfields, Int_t nvals, Bool_t exp = kTRUE);
      virtual ~HtmlObjTable();

      void     SetLabel(Int_t col, const char* label) { fLabels[col] = label; }
      void     SetValue(Int_t col, Int_t row, Float_t val) { fValues[col].SetAt(val, row); }
      TString  Html() const { return fHtml; }

      ClassDef(HtmlObjTable, 0);
  };

  class HtmlSummary : public TObject {
    public:                           // make them public for shorter code
      Int_t           fNTables;
      TOrdCollection* fObjTables;    // ->array of object tables
      TString         fHtml;         // output HTML string
      TString         fTitle;        // page title
      TString         fHeader;       // HTML header
      TString         fFooter;       // HTML footer

      void     MakeHeader();
      void     MakeFooter();

    public:
      HtmlSummary(const char* title);
      virtual ~HtmlSummary();

      HtmlObjTable*  AddTable(const char* name, Int_t nfields, Int_t nvals,
          Bool_t exp = kTRUE, Option_t* opt = "");
      HtmlObjTable*  GetTable(Int_t at) const { return (HtmlObjTable*)fObjTables->At(at); }
      void           Build();
      void           Clear(Option_t* option = "");
      void           Reset(Option_t* option = "");
      TString        Html() const { return fHtml; }

      ClassDef(HtmlSummary, 0);
  };
  */


  /** Responsible for arranging the GL viewers and providing related functionality. */
  class SplitGLView : public TGMainFrame {

  public:
    /** constructor. */
    SplitGLView(const TGWindow* p = 0, UInt_t w = 800, UInt_t h = 600);
    /** destructor. */
    virtual ~SplitGLView();

    /** handler for clicks inside GL viewer. */
    void           ItemClicked(TGListTreeItem* item, Int_t btn, Int_t x, Int_t y);
    /** menu item handler */
    void           HandleMenu(Int_t id);
    /** show name of obj in status bar. */
    void           OnClicked(TObject* obj);
    /** show name of shape in status bar. */
    void           OnMouseOver(TGLPhysicalShape* shape);
    /** handle Activated signals from GLViewer. */
    void           OnViewerActivated();
    /** Open a Root file to display a geometry in the GL viewers. */
    void           OpenFile(const char* fname);
    /** swap specified viewer with main view. */
    void           SwapToMainView(TGLViewerBase* viewer);
    /** toggle wether the active viewer may be rotated (not that useful for projections). */
    void           ToggleOrthoRotate();
    /** Toggle state of the 'Ortho allow dolly' menu entry. */
    void           ToggleOrthoDolly();
    /** move viewer into standalone window. */
    void           UnDock(TGLViewerBase* viewer);
    /** unused. */
    static void    UpdateSummary();

    /** return R-Phi projection manager. */
    TEveProjectionManager* getRPhiMgr() const { return fRPhiMgr; }
    /** return Rho-Z projection manager. */
    TEveProjectionManager* getRhoZMgr() const { return fRhoZMgr; }
    /** return TGLViewer that is active right now. */
    TGLViewer* getActiveGLViewer() const { return fActViewer; }

    enum EMyCommands {
      kFileOpen, kFileExit, kFileLoadConfig, kFileSaveConfig,
      kHelpAbout, kGLPerspYOZ, kGLPerspXOZ, kGLPerspXOY, kGLXOY,
      kGLXOZ, kGLZOY, kGLOrthoRotate, kGLOrthoDolly, kSceneUpdate,
      kSceneUpdateAll, kSummaryUpdate
    };

  private:
    TEvePad*               fPad;           /**< pad used as geometry container */
    TGSplitFrame*          fSplitFrame;    /**< main (first) split frame */
    TGLEmbeddedViewer*     fGLViewer[3];   /**< GL viewers */
    TGLEmbeddedViewer*     fActViewer;     /**< active GL viewer */
    //static HtmlSummary*    fgHtmlSummary;  /** summary HTML table */
    //static TGHtml*         fgHtml;
    TGMenuBar*             fMenuBar;       /**< main menu bar */
    TGPopupMenu*           fMenuFile;      /**< 'File' popup menu */
    TGPopupMenu*           fMenuHelp;      /**< 'Help' popup menu */
    TGPopupMenu*           fMenuCamera;    /**< 'Camera' popup menu */
    TGPopupMenu*           fMenuScene;     /**< 'Scene' popup menu */
    TGStatusBar*           fStatusBar;     /**< status bar */

    TEveViewer*            fViewer[3]; /**< eve viewers */
    TEveProjectionManager* fRPhiMgr;   /**< R-Phi projection */
    TEveProjectionManager* fRhoZMgr;   /**< Rho-Z projection */

    /** Generate root dictionaries. */
    ClassDef(SplitGLView, 0)
  };
}

#endif
