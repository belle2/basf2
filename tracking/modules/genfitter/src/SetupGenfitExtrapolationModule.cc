/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Tadeas Bilka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//
// Common setup for everything that uses genfit's extrapolation code.
//

#include <tracking/modules/genfitter/SetupGenfitExtrapolationModule.h>
#include <tracking/modules/genfitter/Geant4MaterialInterface.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/IO.h>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/concepts.hpp>

#include <TGeoManager.h>

#include <vxd/geometry/GeoCache.h>
#include <../alignment/include/Hierarchy.h>
#include <../alignment/include/Hierarchy.h>
#include <../alignment/include/Hierarchy.h>


#include <TMath.h>

using namespace Belle2;

REG_MODULE(SetupGenfitExtrapolation)

namespace {
//! Stream that writes to Belle II logging system at the debug level
//! given by the template parameter.
  template<size_t T_level>
  class genfitSink : public boost::iostreams::sink {
  public:
    //! The actual function that does the writing.
    std::streamsize write(const char* s, std::streamsize n)
    {
      B2DEBUG(T_level, s);
      return n;
    }
  };

//! Sink for debug output.
  genfitSink<200> debugSink;
//! Buffer for debug output.
  boost::iostreams::stream_buffer<genfitSink<200> > debugStreamBuf(debugSink);
//! Sink for error output.
  genfitSink<100> errorSink;
//! Buffer for error output.
  boost::iostreams::stream_buffer<genfitSink<100> > errorStreamBuf(errorSink);
//! Sink for output from ...::Print() callls.
  genfitSink<150> printSink;
//! Buffer for output from ...::Print() calls.
  boost::iostreams::stream_buffer<genfitSink<150> > printStreamBuf(printSink);

//! Directs output from genfit into the Belle II logging system.
  void setupGenfitStreams()
  {
    genfit::debugOut.rdbuf(&debugStreamBuf);
    genfit::errorOut.rdbuf(&errorStreamBuf);
    genfit::printOut.rdbuf(&printStreamBuf);
  }
}

SetupGenfitExtrapolationModule::SetupGenfitExtrapolationModule() :
  Module(), m_vxdAlignment()
{

  setDescription("Sets up material handling for genfit extrapolation.  Also setups up I/O streams for"
                 " genfit in order to integrate it into basf2 logging system. Also sets up update of VXDAlignment from DB (temporary).");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("whichGeometry", m_geometry,
           "Which geometry should be used, either 'TGeo' or 'Geant4'", std::string("Geant4"));

  // Energy loss, multiple scattering configuration.
  addParam("energyLossBetheBloch", m_energyLossBetheBloch,
           "activate the material effect: EnergyLossBetheBloch", true);
  addParam("noiseBetheBloch", m_noiseBetheBloch,
           "activate the material effect: NoiseBetheBloch", true);
  addParam("noiseCoulomb", m_noiseCoulomb,
           "activate the material effect: NoiseCoulomb", true);
  addParam("energyLossBrems", m_energyLossBrems,
           "activate the material effect: EnergyLossBrems", true);
  addParam("noiseBrems", m_noiseBrems,
           "activate the material effect: NoiseBrems", true);
  addParam("noEffects", m_noEffects,
           "switch off all material effects in Genfit. This overwrites all "
           "individual material effects switches", false);
  addParam("MSCModel", m_mscModel,
           "Multiple scattering model", std::string("Highland"));
  addParam("useVXDAlignment", m_useVXDAlignment,
           "Use VXD alignment from database?", bool(true));
}

SetupGenfitExtrapolationModule::~SetupGenfitExtrapolationModule()
{
}

void SetupGenfitExtrapolationModule::initialize()
{
  setupGenfitStreams();

  //pass the magnetic field to genfit
  if (genfit::FieldManager::getInstance()->isInitialized()) {
    B2ERROR("Magnetic field handling already initialized.  Not touching settings.");
  } else {
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
  }

  if (genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2ERROR("Material handling already initialized.  Not touching settings.");
    return;
  }

  if (!geometry::GeometryManager::getInstance().getTopVolume()) {
    B2FATAL("No geometry set up so far. Load the geometry module.");
  }

  if (m_geometry == "TGeo") {
    if (!gGeoManager) {
      B2INFO("Building TGeo representation.");
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
    }
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
  } else if (m_geometry == "Geant4") {
    genfit::MaterialEffects::getInstance()->init(new Geant4MaterialInterface());
  } else {
    B2FATAL("Invalid choice of geometry interface.  Please use 'TGeo' or 'Geant4'.");
  }

  // activate / deactivate material effects in genfit
  if (m_noEffects) {
    genfit::MaterialEffects::getInstance()->setNoEffects(true);
  } else {
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
    genfit::MaterialEffects::getInstance()->setMscModel(m_mscModel);
  }
}

void SetupGenfitExtrapolationModule::beginRun()
{
}

void SetupGenfitExtrapolationModule::event()
{
}

void SetupGenfitExtrapolationModule::endRun()
{
}

void SetupGenfitExtrapolationModule::terminate()
{
}

