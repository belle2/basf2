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
   * @brief Painter for EclData that shows common event characteristics on
   * 1D histograms.
   */
  class EclPainterCommon : public EclPainter {
  public:
    /// Subtype of histogram to draw.
    /// AMP Amplitude per channel distribution.
    /// AMP_SUM Amplitude per event distribution.
    enum Type {AMP, AMP_SUM, TIME};

    /**
     * @brief Constructor for EclPainter subclass.
     */
    EclPainterCommon(EclData* data, Type type);
    /**
     * @brief Destructor for EclPainter subclass.
     */
    virtual ~EclPainterCommon();

  private:
    /// Display subtypes of this class.
    Type m_type;
    /// Histogram for amplitude distribution.
    TH1F* m_hist;

    /**
     * @brief Return number of X bins.
     */
    int getBinCount();
    /**
     * @brief Return m_x_min.
     */
    int getMinX();
    /**
     * @brief Return m_x_max.
     */
    int getMaxX();
    /**
     * @brief Initialize histogram.
     */
    void initHisto();
    /**
     * @brief Update titles of the histogram.
     */
    void setTitles();

  public:
    /**
     * @brief Return subtype of ECLPainterCommon.
     */
    Type getType();

    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * @brief Redraw the canvas.
     */
    void Draw();
  };
}

#endif // ECL_PAINTER_COMMON
