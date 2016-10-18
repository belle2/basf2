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
    //! Destructor
    ~SVDSupportBoxPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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
    //! Destructor
    ~SVDSupportTabPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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
    //! Destructor
    ~SVDEndmountPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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
  * The Class for SVD Support Rib
  */

  class SVDSupportRibsLayerPar: public TObject {

  public:

    //! Default constructor
    SVDSupportRibsLayerPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportRibsLayerPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDSupportRibsLayerPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

  private:

    int m_id;
    double m_spacing;
    double m_height;

    std::vector<SVDSupportTabPar> m_tabs;
    std::vector<SVDSupportBoxPar> m_boxes;
    std::vector<SVDEndmountPar> m_endmounts;

    ClassDef(SVDSupportRibsLayerPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

  /**
  * The Class for VXD Support Ribs
  */

  class SVDSupportRibsPar: public TObject {

  public:

    //! Default constructor
    SVDSupportRibsPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportRibsPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDSupportRibsPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);

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

    std::vector<SVDSupportRibsLayerPar> m_layers;

    ClassDef(SVDSupportRibsPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

