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
  * The Class for VXD Alignment payload
  */
  class VXDAlignmentPar: public TObject {
  public:
    //! Default constructor
    VXDAlignmentPar() {}
    //! Constructor using Gearbox
    explicit VXDAlignmentPar(const std::string& component, const GearDir& alignment) { read(component, alignment); }
    //! Destructor
    ~VXDAlignmentPar() {}
    //! Get geometry parameters from Gearbox
    void read(const std::string&, const GearDir&);
    /** get dU */
    double getDU() const { return m_dU; }
    /** get dV */
    double getDV() const { return m_dV; }
    /** get dW */
    double getDW() const { return m_dW; }
    /** get alpha */
    double getAlpha() const { return m_alpha; }
    /** get beta */
    double getBeta() const { return m_beta; }
    /** get gamma */
    double getGamma() const { return m_gamma; }

  private:
    double m_dU;
    double m_dV;
    double m_dW;
    double m_alpha;
    double m_beta;
    double m_gamma;


    ClassDef(VXDAlignmentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

