/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    double getToleranceZ() const { return m_toleranceZ; }
    double getTolerancePhi() const { return m_tolerancePhi; }
    double getSigmaSystU() const { return m_sigmaSystU; }
    double getSigmaSystV() const { return m_sigmaSystV; }
    double getNumSigmaTotU() const { return m_numSigmaTotU; }
    double getNumSigmaTotV() const { return m_numSigmaTotV; }
    double getMaxWidthU() const { return m_maxWidthU; }
    double getMaxWidthV() const { return m_maxWidthV; }

  private:
    /** store whether ROI finding was used */
    bool m_useROIfinding;

    /** you might to disable ROI finding for every Nth event (-1 if not used) */
    int m_disableROIforEveryNth;


    /** tolerance for finding sensor in Z coordinate (cm) */
    double m_toleranceZ;
    /** tolerance for finding sensor in phi coordinate (radians) */
    double m_tolerancePhi;
    /** fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystU;
    /** fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_sigmaSystV;
    /** number of sigma (stat+syst) determining the U width of the ROI */
    double m_numSigmaTotU;
    /** number of sigma (stat+syst) determining the U width of the ROI */
    double m_numSigmaTotV;
    /** maximum U width of the ROI */
    double m_maxWidthU;
    /** maximum V width of the ROI */
    double m_maxWidthV;

    ClassDef(ROIParameters, 2);  /**< ClassDef, necessary for ROOT */
  };
}
