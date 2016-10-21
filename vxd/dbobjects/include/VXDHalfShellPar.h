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
#include <map>
#include <vector>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD half shell payload
  */

  class VXDHalfShellPar: public TObject {

  public:
    //! Default constructor
    VXDHalfShellPar() {}
    //! Constructor using Gearbox
    explicit VXDHalfShellPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDHalfShellPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:
    std::string m_shellName;
    double m_shellAngle;

    std::map< int, std::vector<std::pair<int, double>> > m_layers;


    ClassDef(VXDHalfShellPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

