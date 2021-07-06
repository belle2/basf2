/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/modules/fullsim/FullSimTimingModule.h>
#include <simulation/kernel/RunManager.h>
#include <framework/gearbox/Unit.h>
#include <G4EventManager.hh>
#include <G4VSteppingVerbose.hh>
#include <G4RegionStore.hh>
#include <framework/logging/Logger.h>

#include <TProfile.h>
#include <TFile.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FullSimTiming)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FullSimTimingModule::FullSimTimingModule() : Module()
{
  // Set module properties
  setDescription("Provide more detailed timing info for FullSim module");

  //So, we have to get rid of the old instance first if we want to get our
  //timing instance in there, otherwise we get an G4Exception
  G4VSteppingVerbose* old = G4VSteppingVerbose::GetInstance();
  if (old) {
    delete old;
    G4VSteppingVerbose::SetInstance(nullptr);
  }
  //Now we create our timing class and set it as the class to be used. Set the callback to our processStep member
  m_timing = new Simulation::StepTiming([&](G4Track * track, G4LogicalVolume * volume, double time) {
    processStep(track, volume, time);
  });
  G4VSteppingVerbose::SetInstance(m_timing);

  // Parameter definitions
  addParam("rootFile", m_rootFileName, "Name of the output root file. If none "
           " is given, no file will be created", std::string(""));
}


void FullSimTimingModule::initialize()
{
  m_isInitialized = false;

  //There are two (useful) scenarios: either this module is in front or after
  //the FullSim module. In both cases we want it to work correctly. As the
  //FullSim module overrides the verbosity of the SteppingManager we have to be
  //careful when to initialize, it must be after the FullSim::initialize(). So
  //either here if we are after the FullSim module or in the first event() call
  //if we are before

  Simulation::RunManager& runManager = Simulation::RunManager::Instance();
  //So, we check if the Simulation has been intialized already. If so we can
  //initialize the StepTiming now, otherwise we do it in the first call to the
  //event function. For this we just check if the PrimaryGeneratorAction has
  //been set
  if (runManager.GetUserPrimaryGeneratorAction()) {
    //initialization is done by just calling the event function once
    event();
  }
}

void FullSimTimingModule::event()
{
  if (!m_isInitialized) {
    //Override verbosity of the Stepping manager to make sure that the
    //TrackingVerbose instance is called
    G4SteppingManager* stepping = G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager();
    //Set the timing class
    stepping->SetVerbose(m_timing);
    //Make sure the SteppingManager is set to verbose otherwise the timing class will not be called
    stepping->SetVerboseLevel(1);
    //Make sure the timing class knows about the stepping manager
    m_timing->SetManager(stepping);
    //And set it to silent so the TrackingManager does not print the TrackBanner
    m_timing->SetSilent(1);
    m_timing->SetSilentStepInfo(1);
    //Ok, we want to plot the regions in the order they were created so we need
    //to map from region to index in the store
    int index {0};
    for (G4Region* region : * (G4RegionStore::GetInstance())) {
      //We don't care about regions wihtout volumes
      if (region->GetNumberOfRootVolumes() == 0) continue;
      //Increase the index otherwise
      m_regionIndices[region] = index++;
    }
    //create profile to store timings
    m_timingProfile = new TProfile(
      "SimulationTiming", "Simulation Timing;region;stepping time per event / ms",
      index, 0, index, "s"
    );
    //Set counter variables
    m_totalTime = 0;
    m_eventCount = 0;
    m_eventTime = 0;
    //And we only do this once
    m_isInitialized = true;
  }
  //If the eventTime is >0 we recorded at least one step, so let's update the
  //profile and reset the counters
  if (m_eventTime > 0) {
    ++m_eventCount;
    m_eventTime = 0;
    for (auto& it : m_regionCache) {
      int index = m_regionIndices[it.first];
      m_timingProfile->Fill(index, it.second);
      it.second = 0;
    }
  }
}

void FullSimTimingModule::processStep(G4Track*, G4LogicalVolume* volume, double time)
{
  const G4Region* const region = volume->GetRegion();
  //Check if we are still in the same region to speed up the lookup
  if (m_lastRegion == end(m_regionCache) || m_lastRegion->first != region) {
    //Apparently not, so find or create the region in the Cache
    m_lastRegion = m_regionCache.insert(std::make_pair(region, 0.0)).first;
  }
  //Record times
  m_lastRegion->second += time;
  m_totalTime += time;
  m_eventTime += time;
}

void FullSimTimingModule::terminate()
{
  //Call event one last time to get the last timing in case we were before the FullSim module
  event();
  //Ok, now finish the results
  B2RESULT("FullSimTiming: Total simulation stepping time: " << (m_totalTime / Unit::s) << " s");
  B2RESULT("FullSimTiming: Number of simulated events: " << m_eventCount);
  //Remove stat box
  m_timingProfile->SetStats(0);
  //Convert to ms
  m_timingProfile->Scale(1. / Unit::ms);
  //Set the labels of the profile to the region names and check the maximum length
  size_t regionWidth {6};
  for (auto& it : m_regionIndices) {
    m_timingProfile->GetXaxis()->SetBinLabel(it.second + 1, it.first->GetName().c_str());
    regionWidth = std::max(regionWidth, it.first->GetName().size());
  }
  //Do we want a root file?
  if (!m_rootFileName.empty()) {
    TFile* file = new TFile(m_rootFileName.c_str(), "RECREATE");
    file->cd();
    m_timingProfile->Write();
    file->Close();
  }
  if (m_totalTime > 0) {
    //Now print the result as a table. But first we need the total average
    double totalAvg {0};
    size_t maxContentWidth {4};
    size_t maxErrorWidth {4};
    for (int i = 0; i < m_timingProfile->GetNbinsX(); ++i) {
      //We want to calculate the total avg per event and the width required to
      //show all times and their errors. So we need times and errors in ms
      const double content = m_timingProfile->GetBinContent(i + 1);
      const double error = m_timingProfile->GetBinError(i + 1);
      //add up the averages.
      totalAvg += content;
      //and calculate the number of digits
      const size_t contentWidth = std::floor(std::log10(content > 0 ? content : 1) + 1);
      const size_t errorWidth = std::floor(std::log10(error > 0 ? error : 1) + 1);
      //and get the maximum, including dot and 2 digits
      maxContentWidth = std::max(maxContentWidth, contentWidth + 3);
      maxErrorWidth = std::max(maxErrorWidth, errorWidth + 3);
    }
    //We want percentages ...
    totalAvg /= 100;
    //And finally we print the table
    B2RESULT("FullSimTiming: Simulation stepping time per event and region");
    for (int i = 0; i < m_timingProfile->GetNbinsX(); ++i) {
      B2RESULT("  "
               << std::setw(regionWidth + 1) << std::left << m_timingProfile->GetXaxis()->GetBinLabel(i + 1) << " "
               << std::right << std::fixed << std::setprecision(2)
               << std::setw(maxContentWidth) << m_timingProfile->GetBinContent(i + 1) << " ms +- "
               << std::setw(maxErrorWidth) << m_timingProfile->GetBinError(i + 1) << " ms ("
               << std::setw(6) << (m_timingProfile->GetBinContent(i + 1) / totalAvg) << " %)"
              );
    }
  }
}
