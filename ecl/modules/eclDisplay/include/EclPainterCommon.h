/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECL_PAINTER_COMMON
#define ECL_PAINTER_COMMON

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH1.h>

// TODO: Dynamic bin count for AMP_SUM

namespace Belle2 {
  /**
   * Painter for EclData that shows common event characteristics on
   * 1D histograms.
   */
  class EclPainterCommon : public EclPainter {
  public:
    /**  Subtype of histogram to draw. */
    enum Type {
      AMP, /**< Amplitude per channel distribution. */
      AMP_SUM, /**< Amplitude per event distribution. */
      TIME /**< Time distribution. */
    };

    /**
     * Constructor for EclPainter subclass.
     */
    EclPainterCommon(EclData* data, Type type);
    /**
     * Destructor for EclPainter subclass.
     */
    virtual ~EclPainterCommon();

  private:
    /**  Display subtypes of this class. */
    Type m_type;
    /**  Histogram for amplitude distribution. */
    TH1F* m_hist;

    /**
     * Return number of X bins.
     */
    int getBinCount();
    /**
     * Return m_x_min.
     */
    int getMinX();
    /**
     * Return m_x_max.
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
     * Return subtype of ECLPainterCommon.
     */
    Type getType();

    /**
     * Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * Redraw the canvas.
     */
    void Draw();
  };
}

#endif // ECL_PAINTER_COMMON
