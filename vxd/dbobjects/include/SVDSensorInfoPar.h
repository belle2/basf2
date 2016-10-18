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

  class SVDSensorInfoPar: public TObject {

  public:

    //! Default constructor
    SVDSensorInfoPar() {}
    //! Constructor using Gearbox
    explicit SVDSensorInfoPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDSensorInfoPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    double m_width;
    double m_length;
    double m_height;
    int m_stripsU;
    int m_stripsV;
    double m_width2 = {0};

    double m_depletionVoltage;
    double m_biasVoltage;
    double m_backplaneCapacitance;
    double m_interstripCapacitance;
    double m_couplingCapacitance;
    double m_electronicNoiseU;
    double m_electronicNoiseV;

    ClassDef(SVDSensorInfoPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

