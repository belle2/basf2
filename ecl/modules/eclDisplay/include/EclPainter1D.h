/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#pragma once

//ECL
#include <ecl/modules/eclDisplay/EclPainter.h>

class TH1F;

namespace Belle2 {
  /**
   * Painter for EclData, 1D histograms.
   */
  class EclPainter1D : public EclPainter {
  public:
    /**  Subtype of histogram to draw. */
    enum Type {
      CHANNEL, /**< Events/energy per channel */
      SHAPER, /**< Events/energy per ShaperDSP */
      CRATE, /**< Events/energy per crate/ECLCollector */
      PHI,
      THETA
    };

    /**
     * Constructor for EclPainter subclass.
     */
    EclPainter1D(EclData* data, Type type);
    /**
     * Destructor for EclPainter subclass.
     */
    virtual ~EclPainter1D();

  private:
    /**  Display subtypes of this class. */
    Type m_type;
    /**  Displayed histogram. */
    TH1F* m_hist;

    /**  ID of currently selected shaper. */
    int m_shaper;
    /**  ID of currently selected crate. */
    int m_crate;

    /**
     * Convert channel id to X bin number.
     */
    int channelToSegId(int channel);
    /**
     * Returns number of X bins.
     */
    int getMaxX();
    /**
     * Initialize histogram.
     */
    void initHisto();
    /**
     * Update titles of the histogram.
     */
    void setTitles();

  public:
    /**
     * Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * Return subtype of ECLPainter1D.
     */
    Type getType();

    /**
     * Creates sub-histogram for crates and shapers. This function is
     * called upon click in EclFrame.
     */
    virtual EclPainter* handleClick(int px, int py);

    /**
     * Set XRange for histogram.
     */
    void setXRange(int xmin, int xmax);

    /**
     * Show data only from specific shaper.
     * Show data for all shapers if on of the arguments is negative.
     */
    void setShaper(int crate, int shaper);

    /**
     * Show data only from specific crate.
     * Show data for all crates if param 'crate' < 0.
     */
    void setCrate(int crate);

    /**
     * Redraw the canvas.
     */
    void Draw();
  };
}
