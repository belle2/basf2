/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECL_FRAME
#define ECL_FRAME

#define ECLDISPLAY_OPENGL_PAINTER

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
#include <TGFileDialog.h>
#include <TGMenu.h>
#include <TTimer.h>
#include <TCanvas.h>
#include <TGeoVolume.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TStyle.h>
#include <TSystem.h>

#include <ecl/utility/eclChannelMapper.h>

#include <ecl/modules/eclDisplay/EclData.h>
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>
#include <ecl/modules/eclDisplay/EclPainterFactory.h>
#include <ecl/modules/eclDisplay/EclPainter.h>

namespace Belle2 {
  /**
   * @brief Root TGMainFrame that contains multiple widgets that display the
   * ECLSimHit's w.r.t. structure of ECL geometry and its data acquisition
   * subsystem.
   */
  class EclFrame : public TGMainFrame {
  private:
    /* Frames */
    /// Main frame for settings; its subframes contain settings for different categories.
    TGVerticalFrame* m_settings;

    /// Information subframe
    MultilineWidget* m_frame1;
    /// Event range subframe
    TGGroupFrame* m_frame2;
    /// Optional subframe, crate/shaper select.
    TGGroupFrame* m_frame3;
    TGListTree* m_list_tree;
    /// Channel exclusion subframe
    TGGroupFrame* m_frame4;
    /// Energy threshold subframe
    TGGroupFrame* m_frame5;
    /// Button that initiates drawing of the data for selected events.
    TGTextButton* m_draw;
    /// Button that initiates drawing of the data for all events.
    TGTextButton* m_draw_all;

    /// Embedded canvas.
    TRootEmbeddedCanvas* m_ecanvas;

    /// Current painter type. See EclPainterFactory for the list of all types.
    EclPainterType m_painter_type;

    /// EclData, class containing data to display.
    EclData* m_ecl_data;
    TGDoubleHSlider* m_time_slider;

    /// Slider for choosing events range.
    TGDoubleHSlider* m_ev_slider;
    /// Min value for range of displayed events.
    TGNumberEntry* m_events_min;
    /// Max value for range of displayed events.
    TGNumberEntry* m_events_max;

    /// Number field for channel exclusion.
    TGNumberEntry* m_channel_id;

    /// Current subsystem displayed
    EclData::EclSubsystem m_subsys;

    /* Energy threshold */
    /// Min value for energy threshold.
    TGNumberEntry* m_min_en_threshold;
    /// Max value for energy threshold.
    TGNumberEntry* m_max_en_threshold;
    /// Check button to turn energy threshold on/off.
    TGCheckButton* m_threshold_switch;

    /// Current EclPainter
    EclPainter* m_ecl_painter;
    /// ECLChannelMapper, class for conversion from CellID to (crate, shaper, chn_id).
    ECLChannelMapper* m_mapper;

    /// This flag controls whether to display newly loaded events automatically.
    bool m_auto_display;
    /// If m_auto_display is set, this is the evtn of last displayed event.
    int m_last_event;
    /// Default is true, m_open is set to false when EclFrame closes.
    bool m_open;

    /// Enum for menu commands.
    enum EclFrameCommandIdentifiers {
      M_FILE_SAVE,
      M_FILE_EXIT,

      M_VIEW_EVENTS, // Display histograms for events.
      M_VIEW_ENERGY, // Display histograms for energy.
      M_VIEW_DET_FULL, // Use data from all ECL subsystems.
      M_VIEW_DET_FORW, // Use data from forward endcap only.
      M_VIEW_DET_BACK, // Use data from backward endcap only.
      M_VIEW_DET_BARR  // Use data from barrel only.
    };
    /// Possible export filetypes for histograms.
    const char* filetypes[30] = {
      "PDF",                        "*.pdf",
      "PostScript",                 "*.ps",
      "Encapsulated PostScript",    "*.eps",
      "SVG",                        "*.svg",
      "TeX",                        "*.tex",
      "GIF",                        "*.gif",
      "ROOT macros",                "*.C",
      "ROOT files",                 "*.root",
      "XML",                        "*.xml",
      "PNG",                        "*.png",
      "XPM",                        "*.xpm",
      "JPEG",                       "*.jpg",
      "TIFF",                       "*.tiff",
      "XCF",                        "*.xcf",
      0,                            0
    };

  public:
    /**
     * @param auto_load Display new events as soon as they are loaded.
     */
    EclFrame(int painter_type, EclData* data, bool auto_display,
             ECLChannelMapper* mapper);
    /**
     * ECLFrame destructor. Calls parent class Cleanup() method.
     */
    virtual ~EclFrame();

    /**
     * @brief Getter for m_open
     */
    bool isOpen();
    /**
     * @brief Update view of the data.
     */
    void loadNewData();

    /**
     * @brief Redraw m_ecanvas.
     */
    void updateCanvas();

  private:
    void initGUI(int w, int h);
    void initData();

    void MapSubwindows();

  public:
    /*   SLOTS   */
    /**
     * @brief Apply action from menu.
     */
    void handleMenu(int id);
    /**
     * @brief Change event range and pass information to m_ecl_data.
     */
    void updateEventRange();
    /**
     * @brief Show previous range of events.
     */
    void showPrevEvents();
    /**
     * @brief Show next range of events.
     */
    void showNextEvents();
    /**
     * @brief Exclude channel specified in the GUI.
     */
    void excludeChannel();
    /**
     * @brief Get view parameters from GUI and call updateCanvas().
     */
    void doDraw();
    /**
     * @brief Draw all events on m_ecanvas.
     */
    void doDrawAll();
    /**
     * @brief Update information on the cursor position in the
     * histogram.
     */
    void updateInfo(int event, int px, int py, TObject*);
    /**
     * @brief Opens shapers of specific crate/channels of specific shaper.
     */
    void changeRange(TGListTreeItem* entry, int btn);
    /**
     * @brief Change EclPainter type when selected in drop-down menu.
     * See EclPainterFactory.h, EclPainterType.
     */
    void changeType(int type, bool redraw = true);
    /**
     * @brief When window is closed, this slot sets 'm_open' field to false.
     */
    void setClosed();
    ClassDef(EclFrame, 0)
  };
}

#endif // ECL_FRAME
