/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/simulation/SensitiveDetectorDebugHelper.h>
#include <framework/gearbox/Unit.h>
#include <cassert>

namespace Belle2 {
  namespace VXD {

    SensitiveDetectorDebugHelper& SensitiveDetectorDebugHelper::getInstance()
    {
      static SensitiveDetectorDebugHelper instance;
      return instance;
    }

    SensitiveDetectorDebugHelper::SensitiveDetectorDebugHelper()
    {
      m_file = new TFile("debug-vxdsensitivedetector.root", "RECREATE");
      m_tree = new TTree("sensortraversal", "Debug info for VXD Sensitive Detector implementation");
//Due to laziness, define some macros to create branches with less typing
#define ADDBRANCH__(name,var,type)   m_tree->Branch(#name,&m_info.var,#type)
#define ADDBARRAY__(name,var,n,type) ADDBRANCH__(name,var,name[n]/type)
#define ADDBRANCH(x,type)            ADDBRANCH__(x,x,x/type)
#define ADDBARRAY(x,n,type)          ADDBARRAY__(x,x,n,type)
      //and create all branches
      ADDBRANCH(sensorID, I);
      ADDBRANCH(pdg, I);
      ADDBRANCH(contained, I);
      ADDBRANCH(primary, I);
      ADDBRANCH(length, D);
      ADDBRANCH(stepN, I);
      ADDBARRAY(stepInfo, stepN, D);
      ADDBRANCH(trueN, I);
      ADDBARRAY(trueInfo, trueN, D);
      ADDBRANCH(edepN, I);
      ADDBARRAY(edepInfo, edepN, D);
      ADDBRANCH(simhitN, I);
      ADDBARRAY(simhitInfo, simhitN, D);
    }
    void SensitiveDetectorDebugHelper::startTraversal(VxdID sensorID, const SensorTraversal& traversal)
    {
      m_info.sensorID = sensorID;
      m_info.pdg = traversal.getPDGCode();
      m_info.contained = traversal.isContained();
      m_info.primary = traversal.isPrimary();
      m_info.length = traversal.getLength();
      m_info.stepN = 0;
      m_info.trueN = 0;
      m_info.edepN = 0;
      m_info.simhitN = 0;

      //add all steps if there is enough space
      assert(traversal.size() <= MAX_STEPS);
      for (const StepInformation& step : traversal) {
        m_info.stepInfo[m_info.stepN + 0] = step.position.x();
        m_info.stepInfo[m_info.stepN + 1] = step.position.y();
        m_info.stepInfo[m_info.stepN + 2] = step.position.z();
        m_info.stepInfo[m_info.stepN + 3] = step.momentum.x();
        m_info.stepInfo[m_info.stepN + 4] = step.momentum.y();
        m_info.stepInfo[m_info.stepN + 5] = step.momentum.z();
        m_info.stepInfo[m_info.stepN + 6] = step.electrons;
        m_info.stepInfo[m_info.stepN + 7] = step.time;
        m_info.stepInfo[m_info.stepN + 8] = step.length;
        m_info.stepN += SIZE_STEP;
      }
    }

    void SensitiveDetectorDebugHelper::addTrueHit(const VXDTrueHit* truehit)
    {
      m_info.trueInfo[0] = truehit->getEntryU();
      m_info.trueInfo[1] = truehit->getEntryV();
      m_info.trueInfo[2] = truehit->getEntryW();
      m_info.trueInfo[3] = truehit->getU();
      m_info.trueInfo[4] = truehit->getV();
      m_info.trueInfo[5] = truehit->getW();
      m_info.trueInfo[6] = truehit->getExitU();
      m_info.trueInfo[7] = truehit->getExitV();
      m_info.trueInfo[8] = truehit->getExitW();
      m_info.trueInfo[9] = truehit->getEntryMomentum()[0];
      m_info.trueInfo[10] = truehit->getEntryMomentum()[1];
      m_info.trueInfo[11] = truehit->getEntryMomentum()[2];
      m_info.trueInfo[12] = truehit->getMomentum()[0];
      m_info.trueInfo[13] = truehit->getMomentum()[1];
      m_info.trueInfo[14] = truehit->getMomentum()[2];
      m_info.trueInfo[15] = truehit->getExitMomentum()[0];
      m_info.trueInfo[16] = truehit->getExitMomentum()[1];
      m_info.trueInfo[17] = truehit->getExitMomentum()[2];
      m_info.trueInfo[18] = truehit->getEnergyDep();
      m_info.trueInfo[19] = truehit->getGlobalTime();
      m_info.trueN = SIZE_TRUE;
    }

    void SensitiveDetectorDebugHelper::addSimHit(const VXDSimHit* simhit, int startPoint, int endPoint)
    {
      assert(m_info.simhitN + SIZE_HITS < MAX_HITS * SIZE_HITS);
      m_info.simhitInfo[m_info.simhitN + 0] = simhit->getPosIn()[0];
      m_info.simhitInfo[m_info.simhitN + 1] = simhit->getPosIn()[1];
      m_info.simhitInfo[m_info.simhitN + 2] = simhit->getPosIn()[2];
      m_info.simhitInfo[m_info.simhitN + 3] = simhit->getPosOut()[0];
      m_info.simhitInfo[m_info.simhitN + 4] = simhit->getPosOut()[1];
      m_info.simhitInfo[m_info.simhitN + 5] = simhit->getPosOut()[2];
      m_info.simhitInfo[m_info.simhitN + 6] = simhit->getGlobalTime();
      m_info.simhitInfo[m_info.simhitN + 7] = startPoint;
      m_info.simhitInfo[m_info.simhitN + 8] = endPoint;
      //Add optimized energy deposition profile
      m_info.simhitInfo[m_info.simhitN + 9] = m_info.edepN / 2;
      for (auto step : simhit->getElectronProfile()) {
        m_info.edepInfo[m_info.edepN] = step.first;
        ++m_info.edepN;
        m_info.edepInfo[m_info.edepN] = step.second;
        ++m_info.edepN;
      }
      m_info.simhitInfo[m_info.simhitN + 10] = m_info.edepN / 2;
      //And also add electron profile sampled by distance
      for (auto step : simhit->getElectronsConstantDistance(50 * Unit::um)) {
        m_info.edepInfo[m_info.edepN] = step.first;
        ++m_info.edepN;
        m_info.edepInfo[m_info.edepN] = step.second;
        ++m_info.edepN;
      }
      m_info.simhitInfo[m_info.simhitN + 11] = m_info.edepN / 2;
      //And sampled by electrons
      for (auto step : simhit->getElectronsConstantNumber(1000)) {
        m_info.edepInfo[m_info.edepN] = step.first;
        ++m_info.edepN;
        m_info.edepInfo[m_info.edepN] = step.second;
        ++m_info.edepN;
      }
      m_info.simhitInfo[m_info.simhitN + 12] = m_info.edepN / 2;
      m_info.simhitN += SIZE_HITS;
    }
  }
} //Belle2 namespace
