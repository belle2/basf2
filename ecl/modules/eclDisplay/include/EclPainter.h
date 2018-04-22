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
    EclPainter(EclData* data);
    virtual ~EclPainter();

    /**
     * Set EclData to display in painter.
     */
    void setData(EclData* data);
    /**
     * Return currently displayed EclData.
     */
    EclData* getData();

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
  };
}
