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
     * Copy constructor.
     */
    EclPainter2D(const EclPainter2D& other) : EclPainter(other) { cloneFrom(other); }
    /**
     * Assignment operator.
     */
    EclPainter2D& operator=(const EclPainter2D& other) { cloneFrom(other); return *this; }

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
     * Clone attributes from other EclPainter2D
     */
    void cloneFrom(const EclPainter2D& other);
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
     * @param panel MultilineWidget to display the information
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel) override;

    /**
     * Return subtype of ECLPainter2D.
     */
    Type getType();

    /**
     * Redraw the canvas.
     */
    virtual void Draw() override;
  };
}
