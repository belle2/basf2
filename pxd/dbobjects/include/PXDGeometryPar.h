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

#include <pxd/dbobjects/PXDSensorInfoPar.h>
#include <vxd/dbobjects/VXDGeometryPar.h>
#include <vxd/dbobjects/VXDPolyConePar.h>

namespace Belle2 {


  /**
  * The Class for VXD geometry
  */

  class PXDGeometryPar: public VXDGeometryPar {
  public:
    //! Default constructor
    PXDGeometryPar() {}
    /** build support */
    bool getBuildSupport() const { return (m_nCutouts > 0); }
    /** get number of cutouts */
    int getNCutOuts() const { return m_nCutouts; }
    /** set number of cutouts */
    void setNCutOuts(int nCutouts) { m_nCutouts = nCutouts; }
    /** get width of cutouts */
    double getCutOutWidth() const { return m_cutOutWidth; }
    /** set width of cutouts */
    void setCutOutWidth(double width) { m_cutOutWidth = width; }
    /** get height of cutouts */
    double getCutOutHeight() const { return m_cutOutHeight; }
    /** set height of cutouts */
    void setCutOutHeight(double height) { m_cutOutHeight = height; }
    /** get  shift of cutouts */
    double getCutOutShift() const { return m_cutOutShift; }
    /** set  shift of cutouts */
    void setCutOutShift(double shift) { m_cutOutShift = shift; }
    /** get rphi of cutouts */
    double getCutOutRPhi() const { return m_cutOutRphi; }
    /** set rphi of cutouts */
    void setCutOutRPhi(double rphi) { m_cutOutRphi = rphi; }
    /** get start phi of cutouts */
    double getCutOutStartPhi() const { return m_cutOutStartPhi; }
    /** set start phi of cutouts */
    void setCutOutStartPhi(double start)  { m_cutOutStartPhi = start; }
    /** get deltaphi of cutouts */
    double getCutOutDeltaPhi() const { return m_cutOutDeltaPhi; }
    /** set deltaphi of cutouts */
    void setCutOutDeltaPhi(double delta) { m_cutOutDeltaPhi = delta; }

    /** get number of tubes */
    int getNTubes() const { return m_nTubes; }
    /** set number of tubes */
    void setNTubes(int nTubes)  { m_nTubes = nTubes; }
    /** get tubes minZ*/
    double getTubesMinZ() const { return m_tubesMinZ; }
    /** set tubes minZ*/
    void setTubesMinZ(double minZ) { m_tubesMinZ = minZ; }
    /** get tubes maxZ*/
    double getTubesMaxZ() const { return m_tubesMaxZ; }
    /** set tubes maxZ*/
    void setTubesMaxZ(double maxZ)  { m_tubesMaxZ = maxZ; }
    /** get tubes minR*/
    double getTubesMinR() const { return m_tubesMinR; }
    /** set tubes minR*/
    void setTubesMinR(double minR) { m_tubesMinR = minR; }
    /** get tubes maxR*/
    double getTubesMaxR() const { return m_tubesMaxR; }
    /** set tubes maxR*/
    void setTubesMaxR(double maxR) { m_tubesMaxR = maxR; }
    /** get tubes tubes RPhi*/
    double getTubesRPhi() const { return m_tubesRPhi; }
    /** set tubes tubes RPhi*/
    void setTubesRPhi(double rphi) { m_tubesRPhi = rphi; }
    /** get tubes tubes StartPhi*/
    double getTubesStartPhi() const { return m_tubesStartPhi; }
    /** set tubes tubes StartPhi*/
    void setTubesStartPhi(double start) { m_tubesStartPhi = start; }
    /** get tubes tubes DeltaPhi*/
    double getTubesDeltaPhi() const { return m_tubesDeltaPhi; }
    /** set tubes tubes DeltaPhi*/
    void setTubesDeltaPhi(double delta) { m_tubesDeltaPhi = delta; }
    /** get tubes tubes material*/
    const std::string& getTubesMaterial() const { return m_tubesMaterial; }
    /** set tubes tubes material*/
    void setTubesMaterial(const std::string& material) { m_tubesMaterial = material; }
    /** get  Endflanges */
    const std::vector<VXDPolyConePar>& getEndflanges() const {return m_endflanges;}
    /** get  sensorInfos */
    std::vector<PXDSensorInfoPar*>& getSensorInfos()  {return m_SensorInfo;}
    /** get  Endflanges */
    std::vector<VXDPolyConePar>& getEndflanges() {return m_endflanges;}

  private:

    /** Vector of points to SensorInfo objects */
    std::vector<PXDSensorInfoPar*> m_SensorInfo;

    /** PXD Endflanges */
    std::vector<VXDPolyConePar> m_endflanges;

    /** Number of PXD endflange coutOuts */
    int m_nCutouts {0};
    /** Width of PXD endflange coutOuts */
    double m_cutOutWidth {0};
    /** Height of PXD endflange coutOuts */
    double m_cutOutHeight {0};
    /** Shift of PXD endflange coutOuts */
    double m_cutOutShift {0};
    /** Rphi of PXD endflange coutOuts */
    double m_cutOutRphi {0};
    /** Start Rphi of PXD endflange coutOuts */
    double m_cutOutStartPhi {0};
    /** Delta Rphi of PXD endflange coutOuts */
    double m_cutOutDeltaPhi {0};

    /** Number of PXD carbon tubes */
    int m_nTubes {0};
    /** MinZ of PXD carbon tubes */
    double m_tubesMinZ {0};
    /** MaxZ of PXD carbon tubes */
    double m_tubesMaxZ {0};
    /** MinR of PXD carbon tubes */
    double m_tubesMinR {0};
    /** MaxZ of PXD carbon tubes */
    double m_tubesMaxR {0};
    /** Rphi of PXD carbon tubes */
    double m_tubesRPhi {0};
    /** Start Rphi of PXD carbon tubes */
    double m_tubesStartPhi {0};
    /** Delta Phi of PXD carbon tubes */
    double m_tubesDeltaPhi {0};
    /** Material of PXD carbon tubes */
    std::string m_tubesMaterial {"Carbon"};

    ClassDef(PXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

