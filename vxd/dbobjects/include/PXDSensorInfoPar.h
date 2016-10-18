/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class PXDSensorInfoPar: public TObject {

  public:

    //! Default constructor
    PXDSensorInfoPar() {}
    //! Constructor using Gearbox
    explicit PXDSensorInfoPar(const GearDir& content) { read(content); }
    //! Destructor
    ~PXDSensorInfoPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    double m_width;
    double m_length;
    double m_height;
    int m_pixelsU;
    int m_pixelsV1;
    int m_pixelsV2;
    double m_splitLength;

    double m_bulkDoping;
    double m_backVoltage;
    double m_topVoltage;
    double m_sourceBorderSmallPixel;
    double m_clearBorderSmallPixel;
    double m_drainBorderSmallPixel;
    double m_sourceBorderLargePixel;
    double m_clearBorderLargePixel;
    double m_drainBorderLargePixel;
    double m_gateDepth;
    bool m_doublePixel;
    double m_chargeThreshold;
    double m_noiseFraction;
    double m_integrationStartTime;
    double m_integrationEndTime;

    ClassDef(PXDSensorInfoPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

