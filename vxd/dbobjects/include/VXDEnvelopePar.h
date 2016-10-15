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
#include <list>
#include <vector>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD Envelope parameters
  */
  class VXDEnvelopePar: public TObject {
  public:
    //! Default constructor
    VXDEnvelopePar() {}
    //! Constructor using Gearbox
    explicit VXDEnvelopePar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDEnvelopePar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:
    //! List of points in the ZX plane
    std::list< std::pair<double, double> > m_innerPoints;
    std::list< std::pair<double, double> > m_outerPoints;
    double m_minPhi;
    double m_maxPhi;

    ClassDef(VXDEnvelopePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

