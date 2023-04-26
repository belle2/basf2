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

    /** Set tolerance in z */
    void setToleranceZ(const double toleranceZ) { m_toleranceZ = toleranceZ; }
    /** Set tolerance in phi */
    void setTolerancePhi(const double tolerancePhi) { m_tolerancePhi = tolerancePhi; }
    /** Set sigma_u which represents the minimum ROI size in u */
    void setSigmaSystU(const double sigmaSystU) { m_sigmaSystU = sigmaSystU; }
    /** Set sigma_v which represents the minimum ROI size in v */
    void setSigmaSystV(const double sigmaSystV) { m_sigmaSystV = sigmaSystV; }
    /** Set number of sigmas used to calculate ROI size in u */
    void setNumSigmaTotU(const double numSigmaTotU) { m_numSigmaTotU = numSigmaTotU; }
    /** Set number of sigmas used to calculate ROI size in v */
    void setNumSigmaTotV(const double numSigmaTotV) { m_numSigmaTotV = numSigmaTotV; }
    /** Set maximum ROI size in u */
    void setMaxWidthU(const double maxWidthU) { m_maxWidthU = maxWidthU; }
    /** Set maximum ROI size in v */
    void setMaxWidthV(const double maxWidthV) { m_maxWidthV = maxWidthV; }

    /** Get tolerance in z */
    double getToleranceZ() const { return m_toleranceZ; }
    /** Get tolerance in phi */
    double getTolerancePhi() const { return m_tolerancePhi; }
    /** Get sigma_u which represents the minimum ROI size in u */
    double getSigmaSystU() const { return m_sigmaSystU; }
    /** Get sigma_v which represents the minimum ROI size in v */
    double getSigmaSystV() const { return m_sigmaSystV; }
    /** Get number of sigmas used to calculate ROI size in u */
    double getNumSigmaTotU() const { return m_numSigmaTotU; }
    /** Get number of sigmas used to calculate ROI size in v */
    double getNumSigmaTotV() const { return m_numSigmaTotV; }
    /** Get maximum ROI size in u */
    double getMaxWidthU() const { return m_maxWidthU; }
    /** Get maximum ROI size in v */
    double getMaxWidthV() const { return m_maxWidthV; }

  private:
    /** tolerance for finding sensor in Z coordinate (cm) */
    double m_toleranceZ = 0.5;
    /** tolerance for finding sensor in phi coordinate (radians) */
    double m_tolerancePhi = 0.15;
    /** fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystU = 0.02;
    /** fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_sigmaSystV = 0.02;
    /** number of sigma (stat+syst) determining the U width of the ROI */
    double m_numSigmaTotU = 10;
    /** number of sigma (stat+syst) determining the U width of the ROI */
    double m_numSigmaTotV = 10;
    /** maximum U width of the ROI */
    double m_maxWidthU = 0.5;
    /** maximum V width of the ROI */
    double m_maxWidthV = 0.5;

    /** you might to disable ROI finding for every Nth event (-1 if not used) */
    int m_disableROIforEveryNth;
    /** store whether ROI finding was used */
    bool m_useROIfinding;

    ClassDef(ROIParameters, 2);  /**< ClassDef, necessary for ROOT */
  };
}
