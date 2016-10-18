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
#include <vector>


namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD Alignment Component payload
  */
  class VXDAlignmentComponentPar: public TObject {
  public:
    //! Default constructor
    VXDAlignmentComponentPar() {}
    //! Constructor using Gearbox
    explicit VXDAlignmentComponentPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDAlignmentComponentPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    double m_dU;
    double m_dV;
    double m_dW;
    double m_alpha;
    double m_beta;
    double m_gamma;


    ClassDef(VXDAlignmentComponentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

