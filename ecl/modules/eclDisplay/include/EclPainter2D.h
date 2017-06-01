/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECL_CANVAS_2D
#define ECL_CANVAS_2D

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH2.h>
#include <TPad.h>
#include <TLine.h>

namespace Belle2 {
  /**
   * @brief Painter for EclData, 2D histograms.
   */
  class EclPainter2D : public EclPainter {
  public:
    /// CHANNEL_2D (theta_id:phi_id) histogram.
    /// SHAPER_2D (shaper:crate) histogram.
    enum Type {CHANNEL_2D, SHAPER_2D, NONE};

    /**
     * @brief Constructor for EclPainter subclass.
     */
    EclPainter2D(EclData* data, Type type);
    /**
     * @brief Destructor for EclPainter subclass.
     */
    ~EclPainter2D();

  private:
    /// Display subtypes of this class.
    Type m_type;
    /// Displayed histogram.
    TH2F* m_hist;

    /// The grid itself, drawn in drawGrid().
    TH2C* hgrid;
    /// Grid pad, drawn in drawGrid().
    TPad* grid;

    /**
     * @brief Update histogram titles.
     */
    void setTitles();
    /**
     * @brief Returns number of X bins.
     */
    int getMaxX();
    /**
     * @brief Returns number of Y bins.
     */
    int getMaxY();
    /**
     * @brief Convert channel id to X bin number.
     */
    int channelToSegIdX(int channel);
    /**
     * @brief Convert channel id to Y bin number.
     */
    int channelToSegIdY(int channel);
    /**
     * @brief Initialize grid for drawGrid().
     */
    void initGrid();
    /**
     * @brief Draw grid over histogram. Call after initGrid().
     */
    void drawGrid();

  public:
    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * @brief Return subtype of ECLPainter2D.
     */
    Type getType();

    /**
     * @brief Redraw the canvas.
     */
    virtual void Draw();
  };
}

#endif // ECL_CANVAS_2D
