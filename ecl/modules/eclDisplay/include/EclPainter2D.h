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

class TH2C;
class TH2F;
class TPad;

namespace Belle2 {
  /**
   * Painter for EclData, 2D histograms.
   */
  class EclPainter2D : public EclPainter {
  public:
    /**  Subtype of histogram to draw. */
    enum Type {
      CHANNEL_2D, /**< (theta_id:phi_id) histogram. */
      SHAPER_2D, /**< (shaper:crate) histogram. */
      NONE
    };

    /**
     * Constructor for EclPainter subclass.
     */
    EclPainter2D(EclData* data, Type type);
    /**
     * Destructor for EclPainter subclass.
     */
    ~EclPainter2D();

  private:
    /**  Display subtypes of this class. */
    Type m_type;
    /**  Displayed histogram. */
    TH2F* m_hist;

    /**  The grid itself, drawn in drawGrid(). */
    TH2C* hgrid;
    /**  Grid pad, drawn in drawGrid(). */
    TPad* grid;

    /**
     * Update histogram titles.
     */
    void setTitles();
    /**
     * Returns number of X bins.
     */
    int getMaxX();
    /**
     * Returns number of Y bins.
     */
    int getMaxY();
    /**
     * Convert channel id to X bin number.
     */
    int channelToSegIdX(int channel);
    /**
     * Convert channel id to Y bin number.
     */
    int channelToSegIdY(int channel);
    /**
     * Initialize grid for drawGrid().
     */
    void initGrid();
    /**
     * Draw grid over histogram. Call after initGrid().
     */
    void drawGrid();

  public:
    /**
     * Sets the information to be displayed in the provided MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * Return subtype of ECLPainter2D.
     */
    Type getType();

    /**
     * Redraw the canvas.
     */
    virtual void Draw();
  };
}
