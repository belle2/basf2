/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <svd/dbobjects/SVDSensorInfoPar.h>
#include <vxd/dbobjects/VXDGeometryPar.h>

#include <svd/dbobjects/SVDCoolingPipesPar.h>
#include <svd/dbobjects/SVDSupportRibsPar.h>
#include <svd/dbobjects/SVDEndringsPar.h>
#include <vxd/dbobjects/VXDRotationSolidPar.h>


namespace Belle2 {

  /**
  * The Class for VXD geometry
  */

  class SVDGeometryPar: public VXDGeometryPar {

  public:
    //! Default constructor
    SVDGeometryPar() {}

    /** get SVD halfshell Rotation Solids */
    const std::vector<VXDRotationSolidPar>& getRotationSolids() const {return m_halfShell;}

    /** get SVD halfshell Rotation Solids */
    std::vector<VXDRotationSolidPar>& getRotationSolids() {return m_halfShell;}

    /** get endrings */
    const std::map<int, SVDEndringsPar>& getEndrings() const {return m_endrings;}

    /** get endrings */
    std::map<int, SVDEndringsPar>& getEndrings()  {return m_endrings;}

    /** get cooling pipes */
    const std::map<int, SVDCoolingPipesPar>& getCoolingPipes() const {return m_coolingPipes;}

    /** get cooling pipes */
    std::map<int, SVDCoolingPipesPar>& getCoolingPipes() {return m_coolingPipes;}

    /** get support ribs */
    const std::map<int, SVDSupportRibsPar>& getSupportRibs() const {return m_supportRibs;}

    /** get support ribs */
    std::map<int, SVDSupportRibsPar>& getSupportRibs() {return m_supportRibs;}

    /** get SVD Support Ribs */
    const SVDSupportRibsPar& getSupportRibs(int) const;

    /** get SVD Endrings */
    const SVDEndringsPar& getEndrings(int) const;

    /** get SVD Cooling Pipes */
    const SVDCoolingPipesPar& getCoolingPipes(int) const;

    /** return if SVD Support Ribs exist */
    bool getSupportRibsExist(int) const;

    /** return if endrings exist */
    bool getEndringsExist(int) const;

    /** return if endrings exist */
    bool getCoolingPipesExist(int) const;

    /** get sensorInfos */
    std::vector<SVDSensorInfoPar*>& getSensorInfos()  {return m_SensorInfo;}

  private:

    /** Vector of pointers to SensorInfo objects */
    std::vector<SVDSensorInfoPar*> m_SensorInfo;

    /** SVD Half Shell parameters */
    std::vector<VXDRotationSolidPar> m_halfShell;

    /** SVD Endrings parameters */
    std::map<int, SVDEndringsPar> m_endrings;

    /** SVD Cooling Pipes parameters */
    std::map<int, SVDCoolingPipesPar> m_coolingPipes;

    /** SVD SupportRibs parameters */
    std::map<int, SVDSupportRibsPar> m_supportRibs;

    ClassDef(SVDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

