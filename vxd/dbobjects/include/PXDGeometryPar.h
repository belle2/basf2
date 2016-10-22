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

#include <vxd/dbobjects/PXDSensorInfoPar.h>
#include <vxd/dbobjects/VXDGeometryPar.h>
#include <vxd/dbobjects/VXDPolyConePar.h>

namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD geometry
  */

  class PXDGeometryPar: public VXDGeometryPar {
  public:
    //! Default constructor
    PXDGeometryPar() {}
    //! Constructor using Gearbox
    explicit PXDGeometryPar(const GearDir& content) : VXDGeometryPar(content) {  }
    //! Destructor
    ~PXDGeometryPar();
    //! Get geometry parameters from Gearbox
    //void read(const GearDir&);

    /**
     * Read the sensor definitions from the database
     * @param sensors Reference to the database containing the parameters
       */
    VXDSensorInfoBasePar* createSensorInfo(const GearDir& sensor);

    /**
     * Create support structure for VXD Half Shell, that means everything
     * thagt does not depend on layer or sensor alignment
     * @param support Reference to the database containing the parameters
     */
    void createHalfShellSupport(GearDir support);

    /**
     * Create support structure for a VXD Layer
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    void createLayerSupport(int layer, GearDir support);

    /**
     * Create support structure for a VXD Ladder
     * @param layer Layer ID to create the support for
     * @param support Reference to the database containing the parameters
     */
    void createLadderSupport(int layer, GearDir support);

  private:

    /** Vector of points to SensorInfo objects */
    std::vector<PXDSensorInfoPar*> m_SensorInfo;

    /** PXD Endflanges */
    std::vector<VXDPolyConePar> m_endflanges;

    /** PXD Endflange CoutOuts */
    int m_nCutouts;
    double m_cutOutWidth;
    double m_cutOutHeight;
    double m_cutOutShifz;
    double m_cutOutRphi;
    double m_cutOutStartPhi;
    double m_cutOutDeltaPhi;

    /** PXD Carbon Tubes */
    int m_nTubes;
    double m_tubesMinZ;
    double m_tubesMaxZ;
    double m_tubesMinR;
    double m_tubesMaxR;
    double m_tubesRPhi;
    double m_tubesStartPhi;
    double m_tubesDeltaPhi;
    std::string m_tubesMaterial;


    ClassDef(PXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

