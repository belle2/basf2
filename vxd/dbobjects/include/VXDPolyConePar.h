/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>


namespace Belle2 {

  /**
  * The Class for VXD Polycone Plane
  */

  class VXDPolyConePlanePar: public TObject {
  public:
    /** Constructor */
    VXDPolyConePlanePar(double posZ = 0, double innerRadius = 0, double outerRadius = 0):
      m_posZ(posZ), m_innerRadius(innerRadius), m_outerRadius(outerRadius)
    {}

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
    /** Constructor */
    VXDPolyConePar(const std::string& name = "", const std::string& material = "", double minPhi = 0, double maxPhi = 0,
                   bool doCutOut = false, double cutOutWidth = 0, double cutOutHeight = 0, double cutOutDepth = 0):
      m_name(name), m_material(material), m_minPhi(minPhi), m_maxPhi(maxPhi), m_doCutOut(doCutOut), m_cutOutWidth(cutOutWidth),
      m_cutOutHeight(cutOutHeight), m_cutOutDepth(cutOutDepth)
    {}

    //! Get name
    std::string getName(void) const { return m_name; }
    //! Get material name
    std::string getMaterial(void) const { return m_material; }
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
    std::vector<VXDPolyConePlanePar>& getPlanes(void) { return m_planes; }
    //! Get planes
    const std::vector<VXDPolyConePlanePar>& getPlanes(void) const { return m_planes; }

  private:
    //! Name
    std::string m_name;
    //! Material name
    std::string m_material;
    //! minPhi in rad
    double m_minPhi;
    //! maxPhi in rad
    double m_maxPhi;
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

