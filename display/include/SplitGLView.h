#ifndef SPLITGLVIEW_H
#define SPLITGLVIEW_H

#include "TGLayout.h"
#include "TGLWidget.h"
#include "TEvePad.h"

#include "TEveManager.h"
#include "TEveBrowser.h"
#include "TEveProjectionManager.h"
#include "TQObject.h"

#include "TGLEmbeddedViewer.h"

class TEveWindow;
class TGPopupMenu;
class TGStatusBar;
class TGLPhysicalShape;

namespace Belle2 {

  /** Responsible for arranging the GL viewers and providing related functionality.
   *
   *  Adapted from alice_esd_split.C example.
   */
  class SplitGLView : public TQObject {

  public:
    /** constructor. */
    explicit SplitGLView();
    /** destructor. */
    virtual ~SplitGLView();

    /** handler for clicks inside GL viewer. */
    void           itemClicked(TGListTreeItem* item, Int_t btn, Int_t x, Int_t y = 43);
    /** menu item handler */
    void           handleMenu(Int_t id);
    /** Save a geometry extract from the current state of the TGeo geometry. */
    void saveExtract();
    /** make current viewer active & show name of obj in status bar. */
    void           onClicked(TObject* obj);
    /** show name of shape in status bar. */
    void           onMouseOver(TGLPhysicalShape* shape);
    /** toggle wether the active viewer may be rotated (not that useful for projections). */
    void           toggleOrthoRotate();
    /** Toggle state of the 'Ortho allow dolly' menu entry. */
    void           toggleOrthoDolly();

    /** return R-Phi projection manager. */
    TEveProjectionManager* getRPhiMgr() const { return m_rphiManager; }
    /** return Rho-Z projection manager. */
    TEveProjectionManager* getRhoZMgr() const { return m_rhozManager; }
    /** return TGLEmbeddedViewer that is active right now. */
    TGLEmbeddedViewer* getActiveGLViewer();

    /** Which menu command was selected? */
    enum EMyCommands {
      kGLPerspYOZ, kGLPerspXOZ, kGLPerspXOY, kGLXOY,
      kGLXOZ, kGLZOY, kGLOrthoRotate, kGLOrthoDolly,
      kSceneUpdate, kSceneUpdateAll, kSaveGeometryExtract
    };

  private:
    /** set m_activeViewer and update UI accordingly. */
    void setActiveViewer(TGLEmbeddedViewer* v);

    TGLEmbeddedViewer*     m_glViewer[3];   /**< GL viewers */
    TEveWindow*            m_window[3];   /**< corresponding windows */
    int                    m_activeViewer;   /**< selected viewer/window, or -1. */
    TEveProjectionManager* m_rphiManager;   /**< R-Phi projection */
    TEveProjectionManager* m_rhozManager;   /**< Rho-Z projection */

    TGPopupMenu*           m_cameraMenu;    /**< 'Camera' popup menu */
    TGStatusBar*           m_statusBar;     /**< status bar */

    /** Generate root dictionaries. */
    ClassDef(SplitGLView, 0)
  };
}
#endif
