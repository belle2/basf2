/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleEcmsBBModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/ParticleList.h>

#include <TTree.h>
#include <TH1D.h>


using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsObjectsMiraBelleEcmsBB);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsObjectsMiraBelleEcmsBBModule::PhysicsObjectsMiraBelleEcmsBBModule() : HistoModule()
{
  //Set module properties
  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("Collect data for eCMS calibration algorithm using the momenta of the hadronic events");
  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
  addParam("BmListName", m_BmListName, "Name of the B- particle list", std::string("B-:merged"));
  addParam("B0ListName", m_B0ListName, "Name of the B0 particle list", std::string("B0:merged"));
}


void PhysicsObjectsMiraBelleEcmsBBModule::initialize()
{
  REG_HISTOGRAM
}

void PhysicsObjectsMiraBelleEcmsBBModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleEcmsBB");
  oldDir->cd("PhysicsObjectsMiraBelleEcmsBB");

  const double cMBp = EvtGenDatabasePDG::Instance()->GetParticle("B+")->Mass();
  //const double cMB0 = EvtGenDatabasePDG::Instance()->GetParticle("B0")->Mass();

  // In the original ML-based analysis there are 40 bins for visualisation
  // For simplicity, the same histogram lower bound (B+ mass) is used
  m_hB0 = new TH1D("hB0", "", 80, cMBp, 5.37);
  m_hBp = new TH1D("hBp", "", 80, cMBp, 5.37);
  oldDir->cd();
}

void PhysicsObjectsMiraBelleEcmsBBModule::beginRun()
{
  m_hBp->Reset();
  m_hB0->Reset();
}





void PhysicsObjectsMiraBelleEcmsBBModule::event()
{
  // store event info
  //m_evt  = m_emd->getEvent();
  //m_run  = m_emd->getRun();
  //m_exp  = m_emd->getExperiment();
  //m_time = m_emd->getTime() / 1e9 / 3600.; //from ns to hours

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }


  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
  if (accepted == false) return;

  StoreObjPtr<ParticleList> B0(m_B0ListName);//"B0:merged");
  StoreObjPtr<ParticleList> Bm(m_BmListName);//"B-:merged");


  //put all the B candidates into the vector
  std::vector<const Particle*> Bparts;

  if (B0.isValid()) {
    for (unsigned i = 0; i < B0->getListSize(); ++i)
      if (B0->getParticle(i))
        Bparts.push_back(B0->getParticle(i));
  }
  if (Bm.isValid()) {
    for (unsigned i = 0; i < Bm->getListSize(); ++i)
      if (Bm->getParticle(i))
        Bparts.push_back(Bm->getParticle(i));
  }

  if (Bparts.size() == 0) return;

  for (unsigned i = 0; i < Bparts.size(); ++i) {
    const Particle* Bpart = Bparts[i];

    //Convert mBC and deltaE to the Y4S reference
    double m_pBcms  = PCmsLabTransform::labToCms(Bpart->get4Vector()).P();
    double m_mB        = Bpart->getMass();
    double m_pdg       = Bpart->getPDGCode();
    //double m_mode      = Bpart->getExtraInfo("decayModeID");
    double m_R2        = Variable::R2(Bpart);


    const Particle* D    = nullptr;
    double m_dmDstar = -99;


    static const int c_PdgD0    = abs(EvtGenDatabasePDG::Instance()->GetParticle("D0")->PdgCode());
    static const int c_PdgDplus = abs(EvtGenDatabasePDG::Instance()->GetParticle("D+")->PdgCode());

    static const int c_PdgDstar0    = abs(EvtGenDatabasePDG::Instance()->GetParticle("D*0")->PdgCode());
    static const int c_PdgDstarPlus = abs(EvtGenDatabasePDG::Instance()->GetParticle("D*+")->PdgCode());

    //if D0 or D+ meson
    if (abs(Bpart->getDaughter(0)->getPDGCode()) == c_PdgD0 || abs(Bpart->getDaughter(0)->getPDGCode()) == c_PdgDplus) {
      D = Bpart->getDaughter(0);
    } else if (abs(Bpart->getDaughter(0)->getPDGCode()) == c_PdgDstar0 || abs(Bpart->getDaughter(0)->getPDGCode()) == c_PdgDstarPlus) {
      const Particle* Dstar = Bpart->getDaughter(0);
      D = Dstar->getDaughter(0);
      m_dmDstar    = Dstar->getMass() - D->getMass();
    } else {
      B2INFO("No D meson found");
    }
    double m_mD;
    //double m_Kpid;
    if (D != nullptr) {
      m_mD    = D->getMass();
      //const Particle* Kaon =  D->getDaughter(0);

      //m_Kpid    = -99;
      //if (Kaon && Kaon->getPIDLikelihood()) {
      //  m_Kpid   = Kaon->getPIDLikelihood()->getProbability(Const::kaon, Const::pion);
      //}
    } else {
      m_mD    = -99;
      //m_Kpid    = -99;
    }

    // Fill events to histogram
    //const double cmsE0 = EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)")->Mass(); //Y4S mass


    double p =  m_pBcms;
    double mInv =  m_mB;
    double mD = m_mD;
    double dmDstar = m_dmDstar;
    double pdg = m_pdg;
    double R2 = m_R2;

    //get mass of B+- or B0
    double mB = EvtGenDatabasePDG::Instance()->GetParticle(abs(pdg))->Mass();


    // Filling the events
    if (1.830 < mD && mD < 1.894)
      if (abs(mInv - mB) < 0.05)
        if (R2 < 0.3)
          if ((dmDstar < -10)  || (0.143 < dmDstar && dmDstar < 0.147)) {
            double eBC = sqrt(p * p + pow(mB, 2)); // beam constrained energy
            if (eBC > 5.37) continue;

            if (abs(pdg) == 511) {
              m_hB0->Fill(eBC);
            } else {
              m_hBp->Fill(eBC);
            }

          }
  }



}
