/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {


  /** The payload containing all PXD ROI parameters.
      Right now this is only whether ROI finding was enabled or not,
      but this class can be extended to store size of ROIs etc. */
  class ROIParameters: public TObject {
  public:
    /** Default constructor */
    ROIParameters() : m_useROIfinding(true), m_disableROIforEveryNth(-1) {}
    /** Destructor */
    ~ROIParameters() {}

    /** Set whether ROI finding was used */
    void setROIfinding(bool useROIfinding)
    {
      m_useROIfinding = useROIfinding;
    }

    /** Get whether ROI finding was used */
    bool getROIfinding() const
    {
      return m_useROIfinding;
    }

    /** Set if ROI finding was disabled for every Nth event (-1 if not used) */
    void setDisableROIforEveryNth(int disableROIforEveryNth)
    {
      m_disableROIforEveryNth = disableROIforEveryNth;
    }

    /** Get if ROI finding was disabled for every Nth event (-1 if not used) */
    int getDisableROIforEveryNth() const
    {
      return m_disableROIforEveryNth;
    }

  private:
    /** store whether ROI finding was used */
    bool m_useROIfinding;

    /** you might to disable ROI finding for every Nth event (-1 if not used) */
    int m_disableROIforEveryNth;

    ClassDef(ROIParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}