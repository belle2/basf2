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
  * The Class for VXD Polycone Plane
  */

  class VXDPolyConePlanePar: public TObject {
  public:

    //! Default constructor
    VXDPolyConePlanePar() {}
    //! Constructor using Gearbox
    explicit VXDPolyConePlanePar(const GearDir& content) { read(content); }
    //! Read parameters from Gearbox
    void read(const GearDir&);
    //! Get PosZ in mm
    double getPosZ(void) const { return m_posZ; }
    //! Get InnerRadius in mm
    double getInnerRadius(void) const { return m_innerRadius; }
    //! Get OuterRadius in mm
    double getOuterRadius(void) const { return m_outerRadius; }

  private:
    //! PosZ in mm
    double m_posZ;
    //! InnerRadius in mm
    double m_innerRadius;
    //! OuterRadius in mm
    double m_outerRadius;

    ClassDef(VXDPolyConePlanePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


  /**
  * The Class for VXD PolyCone, possibly with coutouts
  */

  class VXDPolyConePar: public TObject {
  public:

    //! Default constructor
    VXDPolyConePar() {}
    //! Constructor using Gearbox
    explicit VXDPolyConePar(const GearDir& content) { read(content); }
    //! Read parameters from Gearbox
    void read(const GearDir&);
    //! Get name
    std::string getName(void) const { return m_name; }
    //! Get material name
    std::string getMaterial(void) const { return m_materialName; }
    //! Get MinPhi in rad
    double getMinPhi(void) const { return m_minPhi; }
    //! Get MaxPhi in rad
    double getMaxPhi(void) const { return m_maxPhi; }
    //! Get doCutOut flag
    bool getDoCutOut(void) const { return m_doCutOut; }
    //! Get CutOut Width in mm
    double getCutOutWidth(void) const { return m_cutOutWidth; }
    //! Get CutOut Height in mm
    double getCutOutHeight(void) const { return m_cutOutHeight; }
    //! Get CutOut Depth in mm
    double getCutOutDepth(void) const { return m_cutOutDepth; }
    //! Get planes
    std::vector<VXDPolyConePlanePar> getPlanes(void) const { return m_planes; }

  private:
    //! Name
    std::string m_name;
    //! minPhi in rad
    double m_minPhi;
    //! maxPhi in rad
    double m_maxPhi;
    //! Material name
    std::string m_materialName;
    //! doCutOut flag
    bool m_doCutOut;
    //! CutOut Width in mm
    double m_cutOutWidth;
    //! CutOut Height in mm
    double m_cutOutHeight;
    //! CutOut Depth in mm
    double m_cutOutDepth;
    //! Planes
    std::vector<VXDPolyConePlanePar> m_planes;

    ClassDef(VXDPolyConePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };


} // end of namespace Belle2

