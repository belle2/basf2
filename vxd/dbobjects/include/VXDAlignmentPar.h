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

#include <vxd/dbobjects/VXDAlignmentComponentPar.h>

namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD Alignment Component payload
  */
  class VXDAlignmentPar: public TObject {
  public:
    //! Default constructor
    VXDAlignmentPar() {}
    //! Constructor using Gearbox
    explicit VXDAlignmentPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDAlignmentPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
  private:

    std::map<std::string, VXDAlignmentComponentPar > m_components;

    ClassDef(VXDAlignmentPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

