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
  * The Class for SVD Support Box
  */

  class SVDSupportBoxPar: public TObject {

  public:

    //! Default constructor
    SVDSupportBoxPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportBoxPar(const GearDir& content) { read(content); }
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    /** get theta */
    double getTheta() const { return m_theta; }
    /** get z position */
    double getZ() const { return m_zpos; }
    /** get r position */
    double getR() const { return m_rpos; }
    /** get length */
    double getLength() const { return m_length; }

  private:
    double m_theta;
    double m_zpos;
    double m_rpos;
    double m_length;

    ClassDef(SVDSupportBoxPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for SVD Support Rib Tab
  */

  class SVDSupportTabPar: public TObject {

  public:

    //! Default constructor
    SVDSupportTabPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportTabPar(const GearDir& content) { read(content); }
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    /** get theta */
    double getTheta() const { return m_theta; }
    /** get z position */
    double getZ() const { return m_zpos; }
    /** get r position */
    double getR() const { return m_rpos; }

  private:
    double m_theta;
    double m_zpos;
    double m_rpos;

    ClassDef(SVDSupportTabPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for SVD Support Rib Endmounts
  */

  class SVDEndmountPar: public TObject {

  public:

    //! Default constructor
    SVDEndmountPar() {}
    //! Constructor using Gearbox
    explicit SVDEndmountPar(const GearDir& content) { read(content); }
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

    /** get name */
    const std::string& getName() const { return m_name; }
    /** get height */
    double getHeight() const { return m_height; }
    /** get width */
    double getWidth() const { return m_width; }
    /** get length */
    double getLength() const { return m_length; }
    /** get z position */
    double getZ() const { return m_zpos; }
    /** get r position */
    double getR() const { return m_rpos; }

  private:

    std::string m_name;
    double m_height;
    double m_width;
    double m_length;
    double m_zpos;
    double m_rpos;

    ClassDef(SVDEndmountPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };



  /**
  * The Class for SVD Support Ribs (one layer)
  */

  class SVDSupportRibsPar: public TObject {

  public:

    //! Default constructor
    SVDSupportRibsPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportRibsPar(int layer, const GearDir& support) { read(layer, support); }
    //! Get geometry parameters from Gearbox
    void read(int, const GearDir&);

    /** get spacing */
    double getSpacing() const { return m_spacing; }
    /** get height */
    double getHeight() const { return m_height; }
    /** get inner width */
    double getInnerWidth() const { return m_innerWidth; }
    /** get outer width */
    double getOuterWidth() const { return m_outerWidth; }
    /** get tabLength */
    double getTabLength() const { return m_tabLength; }
    /** get the name of outer material*/
    const std::string& getOuterMaterial() const { return m_outerMaterial; }
    /** get the name of inner material*/
    const std::string& getInnerMaterial() const { return m_innerMaterial; }
    /** get the name of inner color*/
    const std::string& getInnerColor() const { return m_innerColor; }
    /** get the name of outer color*/
    const std::string& getOuterColor() const { return m_outerColor; }
    /** get the name of endmount material*/
    const std::string& getEndmountMaterial() const { return m_endmountMaterial; }
    /** get tabs*/
    const std::vector<SVDSupportTabPar>& getTabs() const { return m_tabs; }
    /** get boxes*/
    const std::vector<SVDSupportBoxPar>& getBoxes() const { return m_boxes; }
    /** get endmounts*/
    const std::vector<SVDEndmountPar>& getEndmounts() const { return m_endmounts; }

  private:

    // Get the common values for all layers
    double m_spacing;
    double m_height;
    double m_innerWidth;
    double m_outerWidth;
    double m_tabLength;

    std::string m_outerMaterial;
    std::string m_innerMaterial;
    std::string m_outerColor;
    std::string m_innerColor;
    std::string m_endmountMaterial;

    std::vector<SVDSupportTabPar> m_tabs;
    std::vector<SVDSupportBoxPar> m_boxes;
    std::vector<SVDEndmountPar> m_endmounts;

    ClassDef(SVDSupportRibsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

