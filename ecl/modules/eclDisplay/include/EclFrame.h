#ifndef ECL_FRAME
#define ECL_FRAME

#include <TGClient.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGDoubleSlider.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>
#include <TGListTree.h>
#include <TTimer.h>

#include <TGLabel.h>

#include <ecl/modules/eclDisplay/EclData.h>
#include <ecl/modules/eclDisplay/viewer.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>
#include <ecl/modules/eclDisplay/EclPainterFactory.h>
#include <ecl/modules/eclDisplay/EclPainter.h>
//#include <TGLSAViewer.h>
//#include <ecl/modules/eclDisplay/include/TGLSAViewer.h>
//#include <ecl/modules/eclDisplay/TGLSAViewer.h>
#include <TCanvas.h>

namespace Belle2 {
  class EclFrame : public TGMainFrame {
  private:
    /* Frames */
    TGVerticalFrame* m_settings;

    /// Information subframe
    MultilineWidget* m_frame1;
    /// Event range subframe
    TGGroupFrame* m_frame2;
    /// Optional subframe
    TGGroupFrame* m_frame3;
    TGListTree* m_list_tree;
    /// Channel exclusion subframe
    TGGroupFrame* m_frame4;

    TGGroupFrame* m_frame5;

    TGTextButton* m_draw;

    TRootEmbeddedCanvas* m_ecanvas;

//    TGLSAViewer* m_glviewer;
    bool m_gl_view;

    EclPainterType m_painter_type;

    EclData* m_ecl_data;
    TGDoubleHSlider* m_time_slider;

    /// Slider for choosing events range.
    TGDoubleHSlider* m_ev_slider;
    TGNumberEntry* m_events_max;
    TGNumberEntry* m_events_min;

    TGNumberEntry* m_channel_id;

    /* Energy threshold */
    TGNumberEntry* m_min_en_threshold;
    TGNumberEntry* m_max_en_threshold;
    TGCheckButton* m_threshold_switch;

    EclPainter* m_ecl_painter;

    /// Timer used to automatically load new events from EclData.
    TTimer* m_timer;
    /// This flag controls whether to display newly loaded events automatically.
    bool m_auto_display;
    /// If m_auto_display is set, this is the evtn of last displayed event.
    int m_last_event;

  public:
    /**
     * @param auto_load Display new events as soon as they are loaded.
     */
    EclFrame(int mode, EclData* data, bool auto_display = true);
    virtual ~EclFrame();

  private:
    void InitGUI(int w, int h, bool gl_view);
    void InitData();

    void MapSubwindows();
    /**
     * @brief Changes viewing mode between TRootEmbeddedCanvas and TGLSAViewer.
     */
    void SetGLViewMode(bool mode, bool request_update = false);

  public:
    /* slots */
    void LoadNewData();
    void UpdateEventRange();
    void ShowPrevEvents();
    void ShowNextEvents();
    void ExcludeChannel();
    void DoDraw();
    /**
     * @brief Update information on the cursor position in the
     * histogram.
     */
    void UpdateInfo(int event, int px, int py, TObject*);
    void UpdateInfo3D(TObject* obj, UInt_t state);
    void ChangeRange(TGListTreeItem* entry, int btn);
    void ChangeType(int type, bool redraw = true);
    ClassDef(EclFrame, 0)
  };
}

#endif // ECL_FRAME
