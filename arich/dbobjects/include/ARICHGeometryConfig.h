/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/GearDir.h>
#include "TVector2.h"
#include <arich/dbobjects/ARICHGeoHAPD.h>
#include <arich/dbobjects/ARICHGeoMerger.h>
#include <arich/dbobjects/ARICHGeoCablesEnvelope.h>
#include <arich/dbobjects/ARICHGeoCooling.h>
#include <arich/dbobjects/ARICHGeoFEBCooling.h>
#include <arich/dbobjects/ARICHGeoDetectorPlane.h>
#include <arich/dbobjects/ARICHGeoAerogelPlane.h>
#include <arich/dbobjects/ARICHGeoMirrors.h>
#include <arich/dbobjects/ARICHGeoMasterVolume.h>
#include <arich/dbobjects/ARICHGeoSupport.h>
#include <arich/dbobjects/ARICHGeoGlobalDisplacement.h>
#include <arich/dbobjects/ARICHGeoMirrorDisplacement.h>

#define MAX_N_ALAYERS 5
#define MAXPTS_QE 100

namespace Belle2 {

  //! The Class for ARICH Geometry Parameters
  /*! This class provides ARICH gemetry paramters for simulation, reconstruction and so on.
    These parameters are gotten from gearbox.
  */

  class ARICHGeometryConfig : public ARICHGeoBase {

  public:

    //! Default constructor
    ARICHGeometryConfig() {};

    //! Contructor
    explicit ARICHGeometryConfig(const GearDir&);

    //! Clears
    void clear(void);

    //! Print some debug information
    void print(const std::string& title = "ARICH geometry parameters") const;

    //! gets geometry parameters from gearbox.
    void read(const GearDir& content);

    /**
     * Use basf2 units when returning geometry parameters
     */
    static void useBasf2Units() {s_unit = Unit::cm; s_unitName = "cm";}


    /**
     * Use Geant4 units when returning geometry parameters
     */
    static void useGeantUnits() {s_unit = Unit::mm; s_unitName = "mm";}


    //! get z position of detector plane (starting z of HAPDs)
    double getDetectorZPosition() const;

    //! returns 1 if beam background study (to add additional sensitive modules, detect neutrons, ...), 0 else
    int doBeamBackgroundStudy() const;

    /**
     * Get position of HAPD (x,y) channel in local ARICH coordinates
     * @param moduleID module ID
     * @param xChn HAPD x channel
     * @param yChn HAPD y channel
     * @return channel position
     */
    TVector2 getChannelPosition(unsigned moduleID, unsigned xChn, unsigned yChn) const;

    /**
     * Get geometry configuration of HAPD plane
     * @return detector plane geometry parameters
     */
    const ARICHGeoDetectorPlane& getDetectorPlane() const {return m_detectorPlane;}

    /**
    * Get geometry configuration of aerogel plane
    * @return aerogel plane geometry parameters
    */
    const ARICHGeoAerogelPlane& getAerogelPlane() const {return m_aerogelPlane;}

    /**
     * Get mirrors geometry configuration
     * @return mirrors geometry parameters
     */
    const ARICHGeoMirrors& getMirrors() const {return m_mirrors;}

    /**
     * Get ARICH master volume geometry configuration
     * @return master volume geometry parameters
     */
    const ARICHGeoMasterVolume& getMasterVolume() const {return m_masterVolume;}

    /**
     * Get ARICH support structure geometry configuration
     * @return support structure geometry parameters
     */
    const ARICHGeoSupport& getSupportStructure() const {return m_supportStructure;}

    /**
     * Get HAPD geometry parameters
     * @return HAPD geometry parameters
     */
    const ARICHGeoHAPD& getHAPDGeometry() const { return m_hapd; }

    /**
     * Get Merger PCB geometry parameters
     * @return Merger PCB geometry parameters
     */
    const ARICHGeoMerger& getMergerGeometry() const { return m_merger; }

    /**
     * Get ARICH cables envelop geometry parameters
     * @return ARICH cables envelop geometry parameters
     */
    const ARICHGeoCablesEnvelope& getCablesEnvelope() const { return m_cablesenvelope; }

    /**
     * Get ARICH cooling system geometry parameters
     * @return ARICH cooling system geometry parameters
     */
    const ARICHGeoCooling& getCoolingGeometry() const { return m_cooling; }

    /**
     * Get ARICH FEB cooling system (v2) geometry parameters
     * @return ARICH FEB cooling system (v2) geometry parameters
     */
    const ARICHGeoFEBCooling& getFEBCoolingGeometry() const { return m_coolingv2; }

    /**
     * Get global displacement parameters
     * @return global displacement parameters
     */
    const ARICHGeoGlobalDisplacement& getGlobalDisplacement() const { return m_globalDispl; }

    /**
     * Get mirror displacement parameters
     * @return mirror displacement parameters
     */
    const ARICHGeoMirrorDisplacement& getMirrorDisplacement() const { return m_mirrorDispl; }


    /**
    * Set geometry configuration of aerogel plane
    * @param aerogelPlane aerogel plane geometry parameters
    */
    void setAerogelPlane(const ARICHGeoAerogelPlane& aerogelPlane)
    {
      m_aerogelPlane = aerogelPlane;
    }

    /**
     * Set geometry configuration of HAPD plane
     * @param detectorPlane detector plane geometry parameters
     */
    void setDetectorPlane(const ARICHGeoDetectorPlane& detectorPlane)
    {
      m_detectorPlane = detectorPlane;
    }

    /**
     * Set mirrors geometry configuration
     * @param mirrors mirrors geometry parameters
     */
    void setMirrors(const ARICHGeoMirrors& mirrors)
    {
      m_mirrors = mirrors;
    }

    /**
     * Set master volume geometry configuration
     * @param masterVolume master volume geometry parameters
     */
    void setMasterVolume(const ARICHGeoMasterVolume& masterVolume)
    {
      m_masterVolume = masterVolume;
    }

    /**
     * Set support structure geometry configuration
     * @param supportStructure support structure geometry parameters
     */
    void setSupportStructure(const ARICHGeoSupport& supportStructure)
    {
      m_supportStructure = supportStructure;
    }

    /**
     * Set global displacement parameters
     * @param displ global displacement parameters
     */
    void setGlobalDisplacement(const ARICHGeoGlobalDisplacement& displ)
    {
      m_globalDispl = displ;
    }

    /**
     * Set mirror displacement parameters
     * @param displ mirror displacement parameters
     */
    void setMirrorDisplacement(const ARICHGeoMirrorDisplacement& displ)
    {
      m_mirrorDispl = displ;
    }

    void setUseGlobalDisplacement(bool use)
    {
      m_displaceGlobal = use;
    }

    void setUseMirrorDisplacement(bool use)
    {
      m_displaceMirrors = use;
    }

    bool useGlobalDisplacement() const
    {
      return m_displaceGlobal;
    }

    bool useMirrorDisplacement() const
    {
      return m_displaceMirrors;
    }


  private:

    ARICHGeoDetectorPlane m_detectorPlane;    /**< detector plane geometry configuration */
    ARICHGeoAerogelPlane m_aerogelPlane;      /**< aerogel plane geometry configuration */
    ARICHGeoMirrors m_mirrors;                /**< mirrors geometry configuration */
    ARICHGeoMasterVolume m_masterVolume;      /**< master volume geometry configuration */
    ARICHGeoSupport m_supportStructure;       /**< support structure geometry configuration */
    ARICHGeoHAPD m_hapd;                      /**< HAPD geometry configuration */
    ARICHGeoMerger m_merger;                  /**< Merger PCB geometry configuration */
    ARICHGeoCablesEnvelope m_cablesenvelope;  /**< ARICH cables envelop geometry configuration */
    ARICHGeoCooling m_cooling;                /**< ARICH cooling system geometry configuration */
    ARICHGeoFEBCooling m_coolingv2;           /**< ARICH cooling system (v2) geometry configuration */
    ARICHGeoGlobalDisplacement m_globalDispl; /**< global displacement parameters */
    ARICHGeoMirrorDisplacement m_mirrorDispl; /**< mirror displacement parameters */
    bool m_displaceMirrors = 0;               /**< use mirror displacement parameters */
    bool m_displaceGlobal = 0;                /**< use global displacement parameters */

    int m_bbstudy = 0; /**< is beam background study */

    //! initializes the positions of HAPD modules, with the parameters from xml.
    void modulesPosition(const GearDir& content);

    ClassDef(ARICHGeometryConfig, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };

  inline double ARICHGeometryConfig::getDetectorZPosition() const
  {
    return m_detectorPlane.getPosition().Z() - m_hapd.getModuleSizeZ() / 2.;
  }

  inline int ARICHGeometryConfig::doBeamBackgroundStudy() const
  {
    return m_bbstudy;
  }

} // end of namespace Belle2
