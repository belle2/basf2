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

  class VXDGeometryPar: public TObject {

  public:

    //! Default constructor
    VXDGeometryPar() {}

    //! Constructor using Gearbox
    explicit VXDGeometryPar(const GearDir&);

    //! Destructor
    ~VXDGeometryPar();

    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    //! Get VXD-geometry version
    int getVersion() const { return m_version; }

    //! Set VXD geometry version
    void setVersion(int version) { m_version = version; }

    //! Get comment
    std::string getVXDGeometryParComment() const { return m_comment; }

    //! Set comment
    void setVXDGeometryParComment(const std::string& s) { m_comment = s; }


  private:

    //! Geometry version
    int m_version;

    //! Optional comment
    std::string m_comment;

    ClassDef(VXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

