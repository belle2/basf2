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
    explicit PXDGeometryPar(const std::string& prefix, const GearDir& content) : VXDGeometryPar(prefix, content) {  }
    //! Destructor
    ~PXDGeometryPar();
    /** build support */
    bool getBuildSupport() const { return (m_nCutouts > 0); }
    /** get number of cutouts */
    int getNCutOuts() const { return m_nCutouts; }
    /** get width of cutouts */
    double getCutOutWidth() const { return m_cutOutWidth; }
    /** get height of cutouts */
    double getCutOutHeight() const { return m_cutOutHeight; }
    /** get  shift of cutouts */
    double getCutOutShift() const { return m_cutOutShift; }
    /** get rphi of cutouts */
    double getCutOutRPhi() const { return m_cutOutRphi; }
    /** get start phi of cutouts */
    double getCutOutStartPhi() const { return m_cutOutStartPhi; }
    /** get deltaphi of cutouts */
    double getCutOutDeltaPhi() const { return m_cutOutDeltaPhi; }

    /** get number of tubes */
    int getNTubes() const { return m_nTubes; }
    /** get tubes minZ*/
    double getTubesMinZ() const { return m_tubesMinZ; }
    /** get tubes maxZ*/
    double getTubesMaxZ() const { return m_tubesMaxZ; }
    /** get tubes minR*/
    double getTubesMinR() const { return m_tubesMinR; }
    /** get tubes maxR*/
    double getTubesMaxR() const { return m_tubesMaxR; }
    /** get tubes tubes RPhi*/
    double getTubesRPhi() const { return m_tubesRPhi; }
    /** get tubes tubes StartPhi*/
    double getTubesStartPhi() const { return m_tubesStartPhi; }
    /** get tubes tubes DeltaPhi*/
    double getTubesDeltaPhi() const { return m_tubesDeltaPhi; }
    /** get tubes tubes material*/
    const std::string& getTubesMaterial() const { return m_tubesMaterial; }
    /** get  Endflanges */
    const std::vector<VXDPolyConePar>& getEndflanges() const {return m_endflanges;}

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
    int m_nCutouts {0};
    double m_cutOutWidth {0};
    double m_cutOutHeight {0};
    double m_cutOutShift {0};
    double m_cutOutRphi {0};
    double m_cutOutStartPhi {0};
    double m_cutOutDeltaPhi {0};

    /** PXD Carbon Tubes */
    int m_nTubes {0};
    double m_tubesMinZ {0};
    double m_tubesMaxZ {0};
    double m_tubesMinR {0};
    double m_tubesMaxR {0};
    double m_tubesRPhi {0};
    double m_tubesStartPhi {0};
    double m_tubesDeltaPhi {0};
    std::string m_tubesMaterial {"Carbon"};


    ClassDef(PXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

