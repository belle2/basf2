/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//ECL
#include <ecl/modules/eclDisplay/EclPainter.h>

// TODO: Dynamic bin count for ENERGY_SUM

class TH1F;

namespace Belle2 {
  /**
   * Painter for EclData that shows common event characteristics on
   * 1D histograms.
   */
  class EclPainterCommon : public EclPainter {
  public:
    /**  Subtype of histogram to draw. */
    enum Type {
      ENERGY, /**< Energy per channel distribution. */
      ENERGY_SUM, /**< Energy per event distribution. */
      TIME /**< Time distribution. */
    };

    /**
     * Constructor for EclPainter subclass.
     */
    EclPainterCommon(EclData* data, Type type);
    /**
     * Destructor for EclPainter subclass.
     */
    virtual ~EclPainterCommon() override;
    /** Copy constructor (disabled). */
    EclPainterCommon(const EclPainterCommon&) = delete;

    /** Operator = (disabled). */
    EclPainterCommon& operator=(const EclPainterCommon&) = delete;

  private:
    /**  Display subtypes of this class. */
    Type m_type;
    /**  Histogram for energy distribution. */
    TH1F* m_hist;

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
     * Redraw the canvas.
     */
    void Draw() override;
  };
}
