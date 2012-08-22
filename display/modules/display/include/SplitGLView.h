#ifndef SPLITGLVIEW_H
#define SPLITGLVIEW_H

#include "TGFrame.h"
#include "TGLayout.h"
#include "TGSplitter.h"
#include "TGLWidget.h"
#include "TEvePad.h"
#include "TString.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGFileDialog.h"
#include "TGLPhysicalShape.h"
#include "TGLLogicalShape.h"
#include "TClass.h"

#include "TEveManager.h"
#include "TEveViewer.h"
#include "TEveBrowser.h"
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TEveEventManager.h"
#include "TEveSelection.h"

#include "TGSplitFrame.h"
#include "TGLEmbeddedViewer.h"

namespace Belle2 {

  /** Responsible for arranging the GL viewers and providing related functionality.
   *
   *  Adapted from alice_esd_split.C example.
   */
  class SplitGLView : public TGMainFrame {

  public:
    /** constructor. */
    explicit SplitGLView(const TGWindow* p = 0, UInt_t w = 800, UInt_t h = 600);
    /** destructor. */
    virtual ~SplitGLView();

    /** handler for clicks inside GL viewer. */
    void           itemClicked(TGListTreeItem* item, Int_t btn, Int_t x, Int_t y);
    /** menu item handler */
    void           handleMenu(Int_t id);
    /** make current viewer active & show name of obj in status bar. */
    void           onClicked(TObject* obj);
    /** show name of shape in status bar. */
    void           onMouseOver(TGLPhysicalShape* shape);
    /** swap specified viewer with main view. */
    void           swapToMainView(TGLViewerBase* viewer);
    /** toggle wether the active viewer may be rotated (not that useful for projections). */
    void           toggleOrthoRotate();
    /** Toggle state of the 'Ortho allow dolly' menu entry. */
    void           toggleOrthoDolly();
    /** move viewer into standalone window. */
    void           unDock(TGLViewerBase* viewer);

    /** return R-Phi projection manager. */
    TEveProjectionManager* getRPhiMgr() const { return m_rphiManager; }
    /** return Rho-Z projection manager. */
    TEveProjectionManager* getRhoZMgr() const { return m_rhozManager; }
    /** return TGLViewer that is active right now. */
    TGLViewer* getActiveGLViewer() const { return m_activeViewer; }

    /** Which menu command was selected? */
    enum EMyCommands {
      kFileExit,
      kHelpAbout, kGLPerspYOZ, kGLPerspXOZ, kGLPerspXOY, kGLXOY,
      kGLXOZ, kGLZOY, kGLOrthoRotate, kGLOrthoDolly, kSceneUpdate,
      kSceneUpdateAll
    };

  private:
    /** set m_activeViewer and update UI accordingly. */
    void setActiveViewer(TGLEmbeddedViewer* v);

    TEvePad*               m_pad;           /**< pad used as geometry container */
    TGSplitFrame*          m_splitFrame;    /**< main (first) split frame */
    TGLEmbeddedViewer*     m_glViewer[3];   /**< GL viewers */
    TGLEmbeddedViewer*     m_activeViewer;  /**< active GL viewer */
    TGPopupMenu*           m_cameraMenu;    /**< 'Camera' popup menu */
    TGStatusBar*           m_statusBar;     /**< status bar */

    TEveViewer*            m_viewer[3];     /**< eve viewers */
    TEveProjectionManager* m_rphiManager;   /**< R-Phi projection */
    TEveProjectionManager* m_rhozManager;   /**< Rho-Z projection */

    /** Generate root dictionaries. */
    ClassDef(SplitGLView, 0)
  };
}
#endif
