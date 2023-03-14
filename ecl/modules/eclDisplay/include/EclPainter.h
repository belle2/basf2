/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <ecl/modules/eclDisplay/EclData.h>

class TString;

namespace Belle2 {
  class MultilineWidget;

  namespace ECL {
    class ECLChannelMapper;
  }

  /**
   * Painter for EclData, parent class, created with EclPainterFactory.
   */
  class EclPainter {
  public:
    /**
     * Default constructor.
     * @param data Data to display.
     */
    explicit EclPainter(EclData* data);
    /** Copy constructor */
    EclPainter(const EclPainter& other) { cloneFrom(other); }
    virtual ~EclPainter();

    /** Assignment operator */
    EclPainter& operator=(const EclPainter& other) { cloneFrom(other); return *this; }

    /** Set EclData to display in painter.  */
    void setData(EclData* data) { m_ecl_data = data; }
    /** Return currently displayed EclData. */
    EclData* getData() { return m_ecl_data; }
    /** Return currently displayed EclData. */
    const EclData* getData() const { return m_ecl_data; }

    /**
     * Set ECLChannelMapper for CellID <-> (crate, shaper, chid) conversion.
     */
    void setMapper(ECL::ECLChannelMapper* mapper);
    /**
     * Return currently set ECLChannelMapper
     */
    ECL::ECLChannelMapper* getMapper();

    /**
     * Change between the displayed ECL subsystem (barrel, forward and
     * backward endcaps).
     */
    void setDisplayedSubsystem(EclData::EclSubsystem sys);
    /**
     * Get currently displayed ECL subsystem.
     */
    EclData::EclSubsystem getDisplayedSubsystem();

    /**
     * Return title of ECL subsystem to use in painter.
     */
    TString getSubsystemTitle(EclData::EclSubsystem subsys);

    /**
     * Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     * @param panel MultilineWidget to display the information
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * Some EclPainters can shift to another view upon click.
     * (For example, clicking on crate reveals histogram of shapers in
     * that crate)
     * @return EclPainter with new perspective/range.
     */
    virtual EclPainter* handleClick(int px, int py);

    /**
     * Set XRange for histogram in EclPainter
     */
    virtual void setXRange(int x1, int x2);

    /**
     * Redraw the canvas.
     */
    virtual void Draw() = 0;

  protected:
    /**
     * Make unique name for next root object.
     */
    void getNewRootObjectName(char* buf, int size);

  private:
    /**  Counter to make unique names for new root objects. */
    static int m_obj_counter;
    /**  Data to draw. */
    EclData* m_ecl_data;
    /**  mapper for CellID <-> (crate, shaper, chid) conversion. */
    ECL::ECLChannelMapper* m_mapper;

    /**  Identifier of displayed ECL subsystem. */
    EclData::EclSubsystem displayed_subsys;

    /**
     * Clone attributes from other EclPainter
     */
    void cloneFrom(const EclPainter& other);
  };
}
