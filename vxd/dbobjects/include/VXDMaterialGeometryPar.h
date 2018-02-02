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

#include <framework/gearbox/GearDir.h>
#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {


  /**
  * The Class for Beast Materials between CDC and ECL
  */
  class BeastMaterialsPar: public TObject {
  public:

    /**Constructor */
    BeastMaterialsPar(const std::string& name = "", const std::string& material = "", int identifier = 0,
                      double innerR = 0, double outerR = 0, double backwardZ = 0, double forwardZ = 0) :
      m_name(name), m_material(material), m_identifier(identifier), m_innerR(innerR), m_outerR(outerR), m_backwardZ(backwardZ),
      m_forwardZ(forwardZ)
    {}
    //! Get InnerR in mm
    double getInnerR(void) const { return m_innerR; }
    //! Get OuterR in mm
    double getOuterR(void) const { return m_outerR; }
    //! Get BackwardZ in mm
    double getBackwardZ(void) const { return m_backwardZ; }
    //! Get ForwardZ in mm
    double getForwardZ(void) const { return m_forwardZ; }
    //! Get material
    std::string getMaterial(void) const { return m_material; }
    //! Get name
    std::string getName(void) const { return m_name; }
    //! Get identifier
    int getIdentifier(void) const { return m_identifier; }

  private:
    //! Name
    std::string m_name;
    //! Material
    std::string m_material;
    //! Identifier
    int m_identifier;
    //! InnerR in mm
    double m_innerR;
    //! OuterR in mm
    double m_outerR;
    //! BackwardZ in mm
    double m_backwardZ;
    //! ForwardZ in mm
    double m_forwardZ;

    ClassDef(BeastMaterialsPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
   * The Class for Beast Materials between  barrel and endcap of ECL
  */
  class BeastEclMaterialsPar: public TObject {
  public:

    /**Constructor */
    BeastEclMaterialsPar(const std::string& name = "", const std::string& material = "", int identifier = 0,
                         double innerR1 = 0, double outerR1 = 0, double innerR2 = 0, double outerR2 = 0,
                         double backwardZ = 0, double forwardZ = 0) :
      m_name(name), m_material(material), m_identifier(identifier), m_innerR1(innerR1), m_outerR1(outerR1), m_innerR2(innerR2),
      m_outerR2(outerR2),  m_backwardZ(backwardZ), m_forwardZ(forwardZ)
    {}
    //! Get InnerR1 in mm
    double getInnerR1(void) const { return m_innerR1; }
    //! Get OuterR1 in mm
    double getOuterR1(void) const { return m_outerR1; }
    //! Get InnerR2 in mm
    double getInnerR2(void) const { return m_innerR2; }
    //! Get OuterR2 in mm
    double getOuterR2(void) const { return m_outerR2; }
    //! Get BackwardZ in mm
    double getBackwardZ(void) const { return m_backwardZ; }
    //! Get ForwardZ in mm
    double getForwardZ(void) const { return m_forwardZ; }
    //! Get material
    std::string getMaterial(void) const { return m_material; }
    //! Get name
    std::string getName(void) const { return m_name; }
    //! Get identifier
    int getIdentifier(void) const { return m_identifier; }

  private:
    //! Name
    std::string m_name;
    //! Material
    std::string m_material;
    //! Identifier
    int m_identifier;
    //! InnerR1 in mm
    double m_innerR1;
    //! OuterR1 in mm
    double m_outerR1;
    //! InnerR2 in mm
    double m_innerR2;
    //! OuterR in mm
    double m_outerR2;
    //! BackwardZ in mm
    double m_backwardZ;
    //! ForwardZ in mm
    double m_forwardZ;

    ClassDef(BeastEclMaterialsPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD service geometry
  */
  class GapMomVolBackPar: public TObject {

  public:
    /** Constructor */
    GapMomVolBackPar() {}

    //! Destructor.
    ~GapMomVolBackPar() {}

    //! Append a new node.
    void appendNode(double rmin, double rmax , double z)
    {
      m_rmin.push_back(rmin);
      m_rmax.push_back(rmax);
      m_z.push_back(z);
    }

    //! Get the number of the mother volume nodes.
    int getNNodes() const { return m_rmin.size();}
    //! Get the list of the Rmin corrdinates.
    std::vector<double> getRmin() const { return m_rmin;}
    //! Get the list of the Rmax corrdinates.
    std::vector<double> getRmax() const { return m_rmax;}
    //! Get the list of the z corrdinates.
    std::vector<double> getZ() const { return m_z;}


  private:
    std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
    std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
    std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */

    ClassDef(GapMomVolBackPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };


  /**
  * The Class for VXD service geometry
  */
  class GapMomVolForPar: public TObject {

  public:
    /** Constructor */
    GapMomVolForPar() {}

    //! Destructor.
    ~GapMomVolForPar() {}

    //! Append a new node.
    void appendNode(double rmin, double rmax , double z)
    {
      m_rmin.push_back(rmin);
      m_rmax.push_back(rmax);
      m_z.push_back(z);
    }

    //! Get the number of the mother volume nodes.
    int getNNodes() const { return m_rmin.size();}
    //! Get the list of the Rmin corrdinates.
    std::vector<double> getRmin() const { return m_rmin;}
    //! Get the list of the Rmax corrdinates.
    std::vector<double> getRmax() const { return m_rmax;}
    //! Get the list of the z corrdinates.
    std::vector<double> getZ() const { return m_z;}


  private:
    std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
    std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
    std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */

    ClassDef(GapMomVolForPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

  /**
  * The thickness for backward CDC gap element cell
  */
  class ThicknessBackPar: public TObject {

  public:
    /** Constructor */
    ThicknessBackPar() {}

    //! Destructor.
    ~ThicknessBackPar() {}

    //! Append a new node.
    void appendNode(double thick)
    {
      m_thick.push_back(thick);
    }

    //! Get the number of the mother volume nodes.
    int getNNodes() const { return m_thick.size();}
    //! Get the list of the Rmin corrdinates.
    std::vector<double> getThick() const { return m_thick;}


  private:
    std::vector<double> m_thick; /**< Thickness list of backward CDC gap element cell. */

    ClassDef(ThicknessBackPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };


  /**
  * The thickness for Forward CDC gap element cell
  */
  class ThicknessForPar: public TObject {

  public:
    /** Constructor */
    ThicknessForPar() {}

    //! Destructor.
    ~ThicknessForPar() {}

    //! Append a new node.
    void appendNode(double thick)
    {
      m_thick.push_back(thick);
    }

    //! Get the number of the mother volume nodes.
    int getNNodes() const { return m_thick.size();}
    //! Get the list of the Rmin corrdinates.
    std::vector<double> getThick() const { return m_thick;}


  private:
    std::vector<double> m_thick; /**< Thickness list of forward CDC gap element cell. */

    ClassDef(ThicknessForPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };



  /**
  * The Class for VXD service geometry
  */
  class VXDMaterialGeometryPar: public TObject {

  public:
    /** Constructor */
    VXDMaterialGeometryPar(bool recordBackground = false): m_recordBackground(recordBackground) {}

    //! Destructor.
    ~VXDMaterialGeometryPar() {}

    //! Append a new node.
    void appendNode(double rmin, double rmax , double z)
    {
      m_rmin.push_back(rmin);
      m_rmax.push_back(rmax);
      m_z.push_back(z);
    }

    //! Get the number of the mother volume nodes.
    int getNNodes() const { return m_rmin.size();}
    //! Get the list of the Rmin corrdinates.
    std::vector<double> getRmin() const { return m_rmin;}
    //! Get the list of the Rmax corrdinates.
    std::vector<double> getRmax() const { return m_rmax;}
    //! Get the list of the z corrdinates.
    std::vector<double> getZ() const { return m_z;}
    //! Get Backward Gap MomVolume.
    const GapMomVolBackPar& getMomVolBack(void) const { return m_momvolback; }
    //! Get Backward Gap MomVolume.
    GapMomVolBackPar& getMomVolBack(void)  { return m_momvolback; }
    //! Get Forward Gap MomVolume.
    const GapMomVolForPar& getMomVolFor(void) const { return m_momvolfor; }
    //! Get Forward Gap MomVolume.
    GapMomVolForPar& getMomVolFor(void)  { return m_momvolfor; }
    //! Get Beast Materials.
    const std::vector<BeastMaterialsPar>& getbeastMaterials(void) const { return m_beastMaterials; }
    //! Get Beast Materials.
    std::vector<BeastMaterialsPar>& getbeastMaterials(void)  { return m_beastMaterials; }
    //! Get Beast Materials at ECL.
    const std::vector<BeastEclMaterialsPar>& getbeastEclMaterials(void) const { return m_beastEclMaterials; }
    //! Get Beast Materials at ECL.
    std::vector<BeastEclMaterialsPar>& getbeastEclMaterials(void)  { return m_beastEclMaterials; }
    //! Get Forward Gap element cell Thickness.
    const ThicknessForPar& getthickFor(void) const { return m_thickfor; }
    //! Get Forward Gap element cell Thickness.
    ThicknessForPar& getthickFor(void)  { return m_thickfor; }
    //! Get Backward Gap element cell Thickness.
    const ThicknessBackPar& getthickBack(void) const { return m_thickback; }
    //! Get Backward Gap element cell Thickness.
    ThicknessBackPar& getthickBack(void)  { return m_thickback; }


  private:
    //! Record background
    bool m_recordBackground;
    std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
    std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
    std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */
    std::vector<BeastMaterialsPar> m_beastMaterials; /**< Vector with Beast Materials between CDC and ECL. */
    std::vector<BeastEclMaterialsPar> m_beastEclMaterials; /**< Vector with Beast Materials between barrel and endcap of ECL. */
    GapMomVolBackPar m_momvolback;
    GapMomVolForPar m_momvolfor;
    ThicknessBackPar m_thickback;
    ThicknessForPar m_thickfor;

    ClassDef(VXDMaterialGeometryPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

