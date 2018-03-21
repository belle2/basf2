/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yu Hu and Torben Ferber                                  *
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
   * The Class for Service Materials between CDC and ECL, ARICH and TOP, TOP and ECL.  */
  class ServiceGapsMaterialsCdcArichTopPar: public TObject {
  public:

    /** Constructor */
    ServiceGapsMaterialsCdcArichTopPar(const std::string& name = "", const std::string& material = "", int identifier = 0,
                                       double innerR = 0, double outerR = 0, double backwardZ = 0, double forwardZ = 0) :
      m_name(name), m_material(material), m_identifier(identifier), m_innerR(innerR), m_outerR(outerR), m_backwardZ(backwardZ),
      m_forwardZ(forwardZ)
    {}
    /** Get InnerR in mm */
    double getInnerR(void) const { return m_innerR; }
    /** Get OuterR in mm */
    double getOuterR(void) const { return m_outerR; }
    /** Get BackwardZ in mm */
    double getBackwardZ(void) const { return m_backwardZ; }
    /** Get ForwardZ in mm */
    double getForwardZ(void) const { return m_forwardZ; }
    /** Get material */
    std::string getMaterial(void) const { return m_material; }
    /** Get name */
    std::string getName(void) const { return m_name; }
    /** Get identifier */
    int getIdentifier(void) const { return m_identifier; }

  private:
    /** Name */
    std::string m_name;
    /** Material */
    std::string m_material;
    /** Identifier */
    int m_identifier;
    /** InnerR in mm */
    double m_innerR;
    /** OuterR in mm */
    double m_outerR;
    /** BackwardZ in mm */
    double m_backwardZ;
    /** ForwardZ in mm */
    double m_forwardZ;

    ClassDef(ServiceGapsMaterialsCdcArichTopPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
   * The Class for Service Materials between  barrel and endcap of ECL */
  class ServiceGapsMaterialsEclPar: public TObject {
  public:

    /** Constructor */
    ServiceGapsMaterialsEclPar(const std::string& name = "", const std::string& material = "", int identifier = 0,
                               double innerR1 = 0, double outerR1 = 0, double innerR2 = 0, double outerR2 = 0,
                               double backwardZ = 0, double forwardZ = 0) :
      m_name(name), m_material(material), m_identifier(identifier), m_innerR1(innerR1), m_outerR1(outerR1), m_innerR2(innerR2),
      m_outerR2(outerR2),  m_backwardZ(backwardZ), m_forwardZ(forwardZ)
    {}
    /** Get InnerR1 in mm */
    double getInnerR1(void) const { return m_innerR1; }
    /** Get OuterR1 in mm */
    double getOuterR1(void) const { return m_outerR1; }
    /** Get InnerR2 in mm */
    double getInnerR2(void) const { return m_innerR2; }
    /** Get OuterR2 in mm */
    double getOuterR2(void) const { return m_outerR2; }
    /** Get BackwardZ in mm */
    double getBackwardZ(void) const { return m_backwardZ; }
    /** Get ForwardZ in mm */
    double getForwardZ(void) const { return m_forwardZ; }
    /** Get material */
    std::string getMaterial(void) const { return m_material; }
    /** Get name */
    std::string getName(void) const { return m_name; }
    /** Get identifier */
    int getIdentifier(void) const { return m_identifier; }

  private:
    /** Name */
    std::string m_name;
    /** Material */
    std::string m_material;
    /** Identifier */
    int m_identifier;
    /** InnerR1 in mm */
    double m_innerR1;
    /** OuterR1 in mm */
    double m_outerR1;
    /** InnerR2 in mm */
    double m_innerR2;
    /** OuterR in mm */
    double m_outerR2;
    /** BackwardZ in mm */
    double m_backwardZ;
    /** ForwardZ in mm */
    double m_forwardZ;

    ClassDef(ServiceGapsMaterialsEclPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };


  /**
   * The class for the  mother volume of the Service Materials */
  class ServiceGapsMomVolPar: public TObject {

  public:
    /** Constructor */
    ServiceGapsMomVolPar() {}

    /** Destructor */
    ~ServiceGapsMomVolPar() {}

    /** Append a new node */
    void appendNode(double rmin, double rmax , double z)
    {
      m_rmin.push_back(rmin);
      m_rmax.push_back(rmax);
      m_z.push_back(z);
    }

    /** Get the number of the mother volume nodes */
    int getNNodes() const { return m_rmin.size();}
    /** Get the list of the Rmin corrdinates */
    std::vector<double> getRmin() const { return m_rmin;}
    /** Get the list of the Rmax corrdinates */
    std::vector<double> getRmax() const { return m_rmax;}
    /** Get the list of the z corrdinates */
    std::vector<double> getZ() const { return m_z;}


  private:
    std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
    std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
    std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */

    ClassDef(ServiceGapsMomVolPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

  /**
  * The class for the thicknesses and the density of  gap element cell */
  class ThicknessDensityPar: public TObject {

  public:
    /** Constructor */
    ThicknessDensityPar(double IRCDCB = 0, double IPhiCDCB = 0, double IRCDCF = 0, double IPhiCDCF = 0,
                        double IRECLB = 0, double IPhiECLB = 0, double IRECLF = 0, double IPhiECLF = 0,
                        double IRARICHF = 0, double IPhiARICHF = 0,  double IPhiTOPF = 0,
                        std::vector<double> thicknisses = std::vector<double>(),
                        std::vector<double> density = std::vector<double>()):
      m_IRCDCB(IRCDCB), m_IPhiCDCB(IPhiCDCB), m_IRCDCF(IRCDCF), m_IPhiCDCF(IPhiCDCF), m_IRECLB(IRECLB),
      m_IPhiECLB(IPhiECLB), m_IRECLF(IRECLF), m_IPhiECLF(IPhiECLF), m_IRARICHF(IRARICHF),
      m_IPhiARICHF(IPhiARICHF), m_IPhiTOPF(IPhiTOPF), m_thick(thicknisses), m_density(density) {}

    /** Destructor */
    ~ThicknessDensityPar() {}

    /** Append a new node */
    void appendNode(double thick)
    {
      m_thick.push_back(thick);
    }

    /** Get the number of the mother volume nodes */
    int getNNodes() const { return m_thick.size();}
    /** Get the segmentation in R of CDC backward */
    int getIRCDCB() const { return m_IRCDCB;}
    /** Get the segmentation in Phi of CDC backward */
    int getIPhiCDCB() const { return m_IPhiCDCB;}
    /** Get the segmentation in R of CDC forward */
    int getIRCDCF() const { return m_IRCDCF;}
    /** Get the segmentation in Phi of CDC forward */
    int getIPhiCDCF() const { return m_IPhiCDCF;}
    /** Get the segmentation in R of ECL gap backward */
    int getIRECLB() const { return m_IRECLB;}
    /** Get the segmentation in Phi of ECL gap backward */
    int getIPhiECLB() const { return m_IPhiECLB;}
    /** Get the segmentation in R of ECL gap forward */
    int getIRECLF() const { return m_IRECLF;}
    /** Get the segmentation in Phi of ECL gap forward */
    int getIPhiECLF() const { return m_IPhiECLF;}
    /** Get the segmentation in R of ARICH gap forward */
    int getIRARICHF() const { return m_IRARICHF;}
    /** Get the segmentation in Phi of ARICH gap forward */
    int getIPhiARICHF() const { return m_IPhiARICHF;}
    /** Get the segmentation in Phi of TOP gap forward */
    int getIPhiTOPF() const { return m_IPhiTOPF;}
    /** Get the list of the thicknesses */
    std::vector<double> getthickness() const { return m_thick;}
    /** Get the list of the density */
    std::vector<double> getdensity() const { return m_density;}


  private:
    double m_IRCDCB;      /**< segmentation in R of CDC backward     */
    double m_IPhiCDCB;    /**< segmentation in Phi of CDC backward   */
    double m_IRCDCF;      /**< segmentation in R of CDC forward      */
    double m_IPhiCDCF;    /**< segmentation in Phi of CDC forward    */
    double m_IRECLB;      /**< segmentation in R of ECL backward     */
    double m_IPhiECLB;    /**< segmentation in Phi of ECL backward   */
    double m_IRECLF;      /**< segmentation in R of ECL forward      */
    double m_IPhiECLF;    /**< segmentation in Phi of ECL forward    */
    double m_IRARICHF;    /**< segmentation in R of ARICH forward    */
    double m_IPhiARICHF;  /**< segmentation in Phi of ARICH forward  */
    double m_IPhiTOPF;    /**< segmentation in Phi of TOP forward    */
    std::vector<double> m_thick; /**< Thickness list of CDC gap element cell. */
    std::vector<double> m_density; /**< Densities list of ECL, ARICH and top gap element cell. */

    ClassDef(ThicknessDensityPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };




  /**
   * The Class for services materials geometry */
  class ServiceGapsMaterialsPar: public TObject {

  public:
    /** Constructor */
    ServiceGapsMaterialsPar(bool recordBackground = false): m_recordBackground(recordBackground) {}

    /** Destructor */
    ~ServiceGapsMaterialsPar() {}

    /** Append a new node */
    void appendNode(double rmin, double rmax , double z)
    {
      m_rmin.push_back(rmin);
      m_rmax.push_back(rmax);
      m_z.push_back(z);
    }

    /** Get the number of the mother volume nodes */
    int getNNodes() const { return m_rmin.size();}
    /** Get the list of the Rmin corrdinates */
    std::vector<double> getRmin() const { return m_rmin;}
    /** Get the list of the Rmax corrdinates */
    std::vector<double> getRmax() const { return m_rmax;}
    /** Get the list of the z corrdinates */
    std::vector<double> getZ() const { return m_z;}
    /** Get Backward Gap MomVolume */
    const ServiceGapsMomVolPar& getMomVolBack(void) const { return m_momvolback; }
    /** Get Backward Gap MomVolume */
    ServiceGapsMomVolPar& getMomVolBack(void)  { return m_momvolback; }
    /** Get Forward Gap MomVolume */
    const ServiceGapsMomVolPar& getMomVolFor(void) const { return m_momvolfor; }
    /** Get Forward Gap MomVolume */
    ServiceGapsMomVolPar& getMomVolFor(void)  { return m_momvolfor; }
    /** Get Service Materials */
    const std::vector<ServiceGapsMaterialsCdcArichTopPar>& getServiceGapsMaterials(void) const { return m_ServiceGapsMaterials; }
    /** Get Service Materials */
    std::vector<ServiceGapsMaterialsCdcArichTopPar>& getServiceGapsMaterials(void)  { return m_ServiceGapsMaterials; }
    /** Get Service Materials at ECL */
    const std::vector<ServiceGapsMaterialsEclPar>& getServiceGapsEclMaterials(void) const { return m_ServiceGapsEclMaterials; }
    /** Get Service Materials at ECL */
    std::vector<ServiceGapsMaterialsEclPar>& getServiceGapsEclMaterials(void)  { return m_ServiceGapsEclMaterials; }
    /** Get Gap element cell Thickness and Density */
    const ThicknessDensityPar& getthick(void) const { return m_thick; }
    /** Get Gap element cell Thickness and Density */
    ThicknessDensityPar& getthick(void)  { return m_thick; }


  private:
    /** Record background */
    bool m_recordBackground;
    std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
    std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
    std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */
    std::vector<ServiceGapsMaterialsCdcArichTopPar> m_ServiceGapsMaterials; /**< Vector with Service Materials between CDC and ECL. */
    std::vector<ServiceGapsMaterialsEclPar>
    m_ServiceGapsEclMaterials; /**< Vector with Service Materials between barrel and endcap of ECL. */
    ServiceGapsMomVolPar m_momvolback;  /**< Backward mother volume for Service Materials. */
    ServiceGapsMomVolPar m_momvolfor;   /**< Backward mother volume for Service Materials. */
    ThicknessDensityPar m_thick;        /**< Gap element cell Thickness and density for Service Materials. */

    ClassDef(ServiceGapsMaterialsPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

