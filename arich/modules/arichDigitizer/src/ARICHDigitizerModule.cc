/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Own header.
#include <arich/modules/arichDigitizer/ARICHDigitizerModule.h>

// Hit classes
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHDigit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>

// framework aux
#include <framework/logging/Logger.h>

// magnetic field manager
#include <framework/geometry/BFieldManager.h>

#include <framework/dataobjects/BackgroundInfo.h>


// ROOT
#include <Math/Vector2D.h>
#include <Math/Vector3D.h>
#include <TRandom3.h>

using namespace boost;

namespace Belle2 {

  //-----------------------------------------------------------------
  ///                Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHDigitizer);


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHDigitizerModule::ARICHDigitizerModule() :  Module(),
    m_maxQE(0)
  {

    // Set description()
    setDescription("This module creates ARICHDigits from ARICHSimHits. Here spatial digitization is done, channel-by-channel QE is applied, and readout time window cut is applied.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("TimeWindow", m_timeWindow, "Readout time window width in ns", 250.0);
    addParam("TimeWindowStart", m_timeWindowStart, "Shift of the readout time window with respect to the global zero in ns", -50.0);
    addParam("BackgroundHits", m_bkgLevel, "Number of background hits per hapd per readout (electronics noise)", 0.066);
    addParam("MagneticFieldDistorsion", m_bdistort, "Apply image distortion due to non-perp. magnetic field", 0);
  }

  ARICHDigitizerModule::~ARICHDigitizerModule()
  {

  }

  void ARICHDigitizerModule::initialize()
  {
    // QE at 400nm (3.1eV) applied in SensitiveDetector
    m_maxQE = m_simPar->getQE(3.1);

    m_ARICHSimHits.isRequired();
    m_ARICHDigits.registerInDataStore();

    m_bgOverlay = false;
    StoreObjPtr<BackgroundInfo> bgInfo("", DataStore::c_Persistent);
    if (bgInfo.isValid()) {
      if (bgInfo->getMethod() == BackgroundInfo::c_Overlay) m_bgOverlay = true;
    }

  }

  void ARICHDigitizerModule::beginRun()
  {
    if (m_simPar->getNBkgHits() > 0)  m_bkgLevel = m_simPar->getNBkgHits();
  }

  void ARICHDigitizerModule::event()
  {

    //---------------------------------------------------------------------
    // Convert SimHits one by one to digitizer hits.
    //---------------------------------------------------------------------

    // We try to include the effect of opposite-polarity crosstalk among channels
    // on each chip, which depend on number of p.e. on chip

    std::map<std::pair<int, int>, int> photoElectrons; // this contains number of photoelectrons falling on each channel
    std::map<std::pair<int, int>, int> chipHits; // this contains number of photoelectrons on each chip

    // Loop over all photon hits
    for (const ARICHSimHit& aSimHit : m_ARICHSimHits) {

      // check for time window
      double time = aSimHit.getGlobalTime() - m_timeWindowStart;

      if (time < 0 || time > m_timeWindow) continue;

      ROOT::Math::XYVector locpos = aSimHit.getLocalPosition();

      // Get id of module
      int moduleID = aSimHit.getModuleID();

      if (m_bdistort) magFieldDistorsion(locpos, moduleID);

      // skip if not active
      if (!m_modInfo->isActive(moduleID)) continue;

      // Get id number of hit channel
      int chX, chY;
      m_geoPar->getHAPDGeometry().getXYChannel(locpos.X(), locpos.Y(), chX, chY);

      if (chX < 0 && chY < 0) continue;

      int asicChannel = m_chnMap->getAsicFromXY(chX, chY);


      // eliminate un-active channels
      if (asicChannel < 0 || !m_chnMask->isActive(moduleID, asicChannel)) continue;

      // apply channel dependent QE scale factor
      //double qe_scale =  0.27 / m_maxQE;
      //double qe_scale = m_modInfo->getChannelQE(moduleID, asicChannel) * m_simPar->getColEff() / m_maxQE; // eventually move collection efficiency to here!
      double qe_scale = m_modInfo->getChannelQE(moduleID, asicChannel) / m_maxQE;

      if (qe_scale > 1.) B2ERROR("Channel QE is higher than QE applied in SensitiveDetector");
      if (gRandom->Uniform(1.) > qe_scale) continue;

      // photon was converted to photoelectron
      chipHits[std::make_pair(moduleID, asicChannel / 36)] += 1;
      photoElectrons[std::make_pair(moduleID, asicChannel)] += 1;

    }

    // loop over produced photoelectrons. Apply suppression due to the reverse polarization crosstalk
    // among channels on the same chip, and produce hit bitmap (4 bits).

    for (std::map<std::pair<int, int>, int>::iterator it = photoElectrons.begin(); it != photoElectrons.end(); ++it) {

      std::pair<int, int> modch = it->first;
      double npe = double(it->second);

      // reduce efficiency
      npe /= (1.0 + m_simPar->getChipNegativeCrosstalk() * (double(chipHits[std::make_pair(modch.first, modch.second / 36)]) - 1.0));
      if (npe < 1.0 && gRandom->Uniform(1) > npe) continue;

      // Make hit bitmap (depends on number of p.e. on channel). For now bitmap is 0001 for single p.e., 0011 for 2 p.e., ...
      // More proper implementation is to be done ...
      uint8_t bitmap = 0;
      for (int i = 0; i < npe; i++) {
        bitmap |= 1 << i;
        if (i == 3) break;
      }

      // make new digit!
      m_ARICHDigits.appendNew(modch.first, modch.second, bitmap);

    }

    //--- if background not overlayed add electronic noise hits
    if (m_bgOverlay) return;
    uint8_t bitmap = 1;
    unsigned nSlots = m_geoPar->getDetectorPlane().getNSlots();
    for (unsigned id = 1; id < nSlots + 1; id++) {
      if (!m_modInfo->isActive(id)) continue;
      int nbkg = gRandom->Poisson(m_bkgLevel);
      for (int i = 0; i < nbkg; i++) {
        m_ARICHDigits.appendNew(id, gRandom->Integer(144), bitmap);
      }
    }

  }

  void ARICHDigitizerModule::magFieldDistorsion(ROOT::Math::XYVector& hit, int copyno)
  {

    ROOT::Math::XYVector hitGlob;
    double phi = m_geoPar->getDetectorPlane().getSlotPhi(copyno);
    double r = m_geoPar->getDetectorPlane().getSlotR(copyno);
    double z = m_geoPar->getDetectorZPosition() + m_geoPar->getHAPDGeometry().getWinThickness();
    hitGlob.SetXY(r * std::cos(phi), r * std::sin(phi));
    ROOT::Math::XYVector shift = hit;
    shift.Rotate(phi);
    hitGlob += shift;
    ROOT::Math::XYZVector Bfield = BFieldManager::getField(m_geoPar->getMasterVolume().pointToGlobal(ROOT::Math::XYZVector(hitGlob.X(),
                                                           hitGlob.Y(), z)));
    double cc = m_geoPar->getHAPDGeometry().getPhotocathodeApdDistance() / abs(Bfield.Z());
    shift.SetX(cc * Bfield.X());
    shift.SetY(cc * Bfield.Y());
    shift.Rotate(phi);
    hit.SetX(hit.X() + shift.X());
    hit.SetY(hit.Y() + shift.Y());
  }

} // end Belle2 namespace
