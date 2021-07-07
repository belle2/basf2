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
#include <map>
#include <vector>

#include <vxd/dbobjects/VXDGlobalPar.h>
#include <vxd/dbobjects/VXDAlignmentPar.h>
#include <vxd/dbobjects/VXDEnvelopePar.h>
#include <vxd/dbobjects/VXDHalfShellPar.h>

#include <vxd/dbobjects/VXDGeoComponentPar.h>
#include <vxd/dbobjects/VXDGeoSensorPar.h>
#include <vxd/dbobjects/VXDGeoLadderPar.h>
#include <vxd/dbobjects/VXDGeoRadiationSensorsPar.h>

namespace Belle2 {

  /**
  * The Class for VXD geometry
  */

  class VXDGeometryPar: public TObject {
  public:
    //! Default constructor
    VXDGeometryPar() {}

    /** get prefix */
    std::string getPrefix() const {return m_prefix;}
    /** set prefix */
    void setPrefix(const std::string& prefix) { m_prefix = prefix;}
    /** get global parameters */
    const VXDGlobalPar& getGlobalParams() const {return m_globals;}
    /** set global parameters */
    void setGlobalParams(const VXDGlobalPar& globals)  { m_globals = globals;}
    /** get envelope parameters */
    const VXDEnvelopePar& getEnvelope() const {return m_envelope;}
    /** set envelope parameters */
    void setEnvelope(const VXDEnvelopePar& envelope) {m_envelope = envelope;}
    /** get radiation sensor parameters */
    const VXDGeoRadiationSensorsPar& getRadiationSensors() const {return m_radiationsensors;}
    /** set radiation sensor parameters */
    void setRadiationSensors(const VXDGeoRadiationSensorsPar& diamonds) {m_radiationsensors = diamonds;}
    /** get alignmant map*/
    std::map<std::string, VXDAlignmentPar>& getAlignmentMap() {return m_alignment;}
    /** get alignment parameters for component name */
    VXDAlignmentPar getAlignment(std::string name) const;
    /** get half-shell */
    const std::vector<VXDHalfShellPar>& getHalfShells() const {return m_halfShells;}
    /** get half-shell */
    std::vector<VXDHalfShellPar>& getHalfShells() {return m_halfShells;}
    /** get ladder map */
    std::map<int, VXDGeoLadderPar>& getLadderMap() { return m_ladders; }
    /** get ladder for given layer */
    const VXDGeoLadderPar& getLadder(int layerID) const;
    /** get sensor type */
    const VXDGeoSensorPar& getSensor(std::string sensorTypeID) const;
    /** get component */
    const VXDGeoComponentPar& getComponent(std::string name) const;
    /** get sensor map */
    const std::map<std::string, VXDGeoSensorPar>& getSensorMap() const {return m_sensorMap;}
    /** get sensor map */
    std::map<std::string, VXDGeoSensorPar>& getSensorMap() {return m_sensorMap;}
    /** get component maps */
    const std::map<std::string, VXDGeoComponentPar>& getComponentMap() const {return m_componentCache;   }
    /** get component maps */
    std::map<std::string, VXDGeoComponentPar>& getComponentMap() {return m_componentCache;   }
    /** get Bkg sensitive chip Id*/
    int getSensitiveChipID(std::string name) const;
    /** get sensitive chip id map */
    std::map<std::string, int>& getSensitiveChipIdMap() {return m_sensitiveIDCache;}
    /** get component insert order */
    const std::vector<std::string>& getComponentInsertOder() const {return m_componentInsertOrder;}
    /** get component insert order */
    std::vector<std::string>& getComponentInsertOder() {return m_componentInsertOrder;}

  private:

    /** Prefix to prepend to all volume names */
    std::string m_prefix {""};
    /** Container for a few general parameters */
    VXDGlobalPar m_globals;
    /** Alignment parameters for all components */
    std::map<std::string, VXDAlignmentPar> m_alignment;
    /** Envelope parameters */
    VXDEnvelopePar m_envelope;
    /** Cache for half shells  */
    std::vector<VXDHalfShellPar> m_halfShells;
    /** Cache of all previously created components */
    std::map<std::string, VXDGeoComponentPar> m_componentCache;
    /** Vector remembering the order of insertions into component cache */
    std::vector<std::string> m_componentInsertOrder;
    /** Map containing Information about all defined sensor types */
    std::map<std::string, VXDGeoSensorPar> m_sensorMap;
    /** Diamond radiation sensors */
    VXDGeoRadiationSensorsPar m_radiationsensors;
    /** Parameters of the detector ladders */
    std::map<int, VXDGeoLadderPar> m_ladders;
    /** Cache of all Bkg sensitive chip Ids*/
    std::map<std::string, int> m_sensitiveIDCache;

    ClassDef(VXDGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

