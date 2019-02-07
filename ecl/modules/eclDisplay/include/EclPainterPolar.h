/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#pragma once

//ECL
#include <ecl/modules/eclDisplay/EclPainter.h>

class TH2F;
class TCrown;
class TText;

namespace Belle2 {
  /**
   * Painter for EclData, polar energy/event_count distribution.
   */
  class EclPainterPolar : public EclPainter {
  public:
    /**
     * Type for polar histogram.
     */
    enum Type {PHI, THETA};

    /**
     * Constructor for EclPainter subclass.
     */
    EclPainterPolar(EclData* data, Type type);
    /** Copy constructor */
    EclPainterPolar(const EclPainterPolar& other) : EclPainter(other) { cloneFrom(other); }
    /**
     * Destructor for EclPainter subclass.
     */
    ~EclPainterPolar();

    /** Assignment operator */
    EclPainterPolar& operator=(const EclPainterPolar& other) { cloneFrom(other); return *this; }

  private:
    /**  Type for polar histogram. */
    Type m_type;
    /**  Histogram that generates Z-axis. */
    TH2F* m_hist;
    /**  Phi (or theta) segments of the ECL. */
    TCrown** m_segs;
    /**  Labels for phi segments. */
    TText** m_labels;

    /**
     * Clone attributes from other EclPainterPolar
     */
    void cloneFrom(const EclPainterPolar& other);
    /**
     * Convert ECL channel id to id of the phi (theta) segment.
     */
    int channelToSegId(int channel);
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
    virtual void getInformation(int px, int py, MultilineWidget* panel) override;

    /**
     * Return subtype of ECLPainterPolar.
     */
    Type getType();

    /**
     * Redraw the canvas.
     */
    virtual void Draw() override;
  };
}
