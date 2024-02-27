/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>

#include <tracking/dataobjects/ExtHit.h>

#include <klm/modules/KLMMuIDGetter/KLMMuIDGetterModule.h>

#include <klm/dataobjects/KLMHit2d.h>
//#include <klm/dataobjects/KLMMuidHit.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
//#include <klm/bklm/geometry/Module.h>
#include <klm/muid/MuidElementNumbers.h>

// #include <klm/muid/MuidBuilder.h>
#include <klm/muidgnn/MuidBuilder_fixed.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/Variables.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>


using namespace Belle2;

REG_MODULE(KLMMuIDGetter);

KLMMuIDGetterModule::KLMMuIDGetterModule() : Module()//, m_bklmGeoPar(nullptr)
{
  // Set module properties
  setDescription(R"DOC(Get information from KLMMuIDLikelihood)DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName,
           "list of input ParticleList name", std::string(""));


}

void KLMMuIDGetterModule::initialize()
{
  //m_klmMuidLikelihoods.isRequired();

  StoreObjPtr<ParticleList>().isRequired(m_inputListName);
  m_klmHit2ds.isRequired();

  m_bklmGeoPar = bklm::GeometryPar::instance();
  //getStripPosition();
}

void KLMMuIDGetterModule::event()
{
  // auto builder = new MuidBuilder(13);
  mubuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(13));
  pibuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(211));

  const StoreObjPtr<ParticleList> plist(m_inputListName);

  const unsigned int nParticles = plist->getListSize();

  for (unsigned int iPart = 0; iPart < nParticles; iPart++) {

    m_attributelist_hit.clear();
    m_attributelist_ext.clear();
    m_layerinfo_hit.clear();
    m_layerinfo_ext.clear();

    //std::cout << "********************************" << std::endl;
    //std::cout << "new particle" << std::endl;
    //std::cout << "********************************" << std::endl;
    // const Particle* part = plist->getParticle(iPart);
    Particle* part = plist->getParticle(iPart);

    auto mcpart = part->getMCParticle();
    if (mcpart) {
      int hasMuonDaughter = 0;
      for (auto dau : mcpart->getDaughters()) {
        if (abs(dau->getPDG()) == 13) {
          hasMuonDaughter = 1;
          break;
        }
      }
      part->writeExtraInfo("hasMuonDaughter", hasMuonDaughter);

      // for reconstructed muons to see if their mother is pion
      auto mcmother = mcpart->getMother();
      if (mcmother) {
        int hasPionMother = (abs(mcmother->getPDG()) == 211);
        part->writeExtraInfo("hasPionMother", hasPionMother);
      }

      if (hasMuonDaughter) {
        auto vertex = mcpart->getDecayVertex();
        double vertex_x = vertex.X();
        double vertex_y = vertex.Y();
        double vertex_z = vertex.Z();
        double vertex_r = sqrt(vertex_x * vertex_x + vertex_y * vertex_y);
        bool decayafterKLM = (vertex_z < -320 || vertex_z > 275 || vertex_r > 310);
        bool decayinBKLM = (vertex_z > -180. && vertex_z < 275 && vertex_r > 210) && !decayafterKLM;
        bool decayinEKLM = ((vertex_z < -180. && vertex_r > 127) || (vertex_z > 275 && vertex_r > 127)) && !decayafterKLM;
        int decaybeforeKLM, decayinsideKLM;
        if (decayinBKLM || decayinEKLM) {
          decayinsideKLM = 1;
        } else {
          decayinsideKLM = 0;
        }
        if (decayinsideKLM || decayafterKLM) {
          decaybeforeKLM = 0;
        } else {
          decaybeforeKLM = 1;
        }
        part->writeExtraInfo("decaybeforeKLM", decaybeforeKLM);
        part->writeExtraInfo("decayinsideKLM", decayinsideKLM);
        part->writeExtraInfo("decayafterKLM", decayafterKLM);
      }
    }

    // auto klmll = part->getTrack()->getRelatedTo<KLMMuidLikelihood>();
    auto track = part->getTrack();

    auto klmll = track->getRelatedTo<KLMMuidLikelihood>();

    if (klmll) {
      //part->writeExtraInfo("KLM_mu_L_Likelihood", mubuilder->getLongitudinalPDF(klmll));
      //part->writeExtraInfo("KLM_mu_T_Likelihood", mubuilder->getTransversePDF(klmll));
      //part->writeExtraInfo("KLM_pi_L_Likelihood", pibuilder->getLongitudinalPDF(klmll));
      //part->writeExtraInfo("KLM_pi_T_Likelihood", pibuilder->getTransversePDF(klmll));

      //for (int ii = 0; ii < 15; ii ++) {
      //  part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_efficiency", double(mubuilder->getUnhitLayerEfficiency(klmll, 1, ii)));
      //}
      //for (int ii=0; ii < 14; ii ++){
      //  part->writeExtraInfo("EKLM_FWD_layer"+std::to_string(ii)+"_efficiency", double(mubuilder->getUnhitLayerEfficiency(klmll, 0, ii)));
      //  part->writeExtraInfo("EKLM_BWD_layer"+std::to_string(ii)+"_efficiency", double(mubuilder->getUnhitLayerEfficiency(klmll, 2, ii)));
      //}

      //MuidElementNumbers::Hypothesis muhypo, pihypo;

      //if (part->getCharge()>0){
      //  muhypo = MuidElementNumbers::Hypothesis::c_MuonPlus;
      //  pihypo = MuidElementNumbers::Hypothesis::c_PionPlus;
      //} else{
      //  muhypo = MuidElementNumbers::Hypothesis::c_MuonMinus;
      //  pihypo = MuidElementNumbers::Hypothesis::c_PionMinus;
      //}

      int barrelExtLayer = klmll->getBarrelExtLayer();
      int endcapExtLayer = klmll->getEndcapExtLayer();
      unsigned int extoutcome = klmll->getOutcome();

      //for (int ii = 0; ii <= (barrelExtLayer + endcapExtLayer + 1); ii ++) {
      //  //part->writeExtraInfo("BKLM_muPDF_layer"+std::to_string(ii), builder->getLayerPDF(muhypo, klmll, ii));
      //  //part->writeExtraInfo("BKLM_piPDF_layer"+std::to_string(ii), builder->getLayerPDF(pihypo, klmll, ii));
      //  part->writeExtraInfo("muPDF_layer" + std::to_string(ii), mubuilder->getLayerPDF(klmll, ii));
      //  part->writeExtraInfo("piPDF_layer" + std::to_string(ii), pibuilder->getLayerPDF(klmll, ii));
      //}

      //part->writeExtraInfo("KLM_hitlayer", klmll->getHitLayer());
      //part->writeExtraInfo("KLM_extlayer", klmll->getExtLayer());
      //part->writeExtraInfo("KLM_outlayerdiff", klmll->getExtLayer() - klmll->getHitLayer());
      //part->writeExtraInfo("KLM_hitlayerpattern", klmll->getHitLayerPattern());
      //part->writeExtraInfo("KLM_extlayerpattern", klmll->getExtLayerPattern());

      //part->writeExtraInfo("BKLM_hitlayer", klmll->getBarrelHitLayer());
      //part->writeExtraInfo("BKLM_extlayer", barrelExtLayer);
      part->writeExtraInfo("extOutcome", extoutcome);
      //part->writeExtraInfo("BKLM_outlayerdiff", klmll->getBarrelExtLayer() - klmll->getBarrelHitLayer());

      //part->writeExtraInfo("EKLM_hitlayer", klmll->getEndcapHitLayer());
      //part->writeExtraInfo("EKLM_extlayer", endcapExtLayer);
      //part->writeExtraInfo("EKLM_outlayerdiff", klmll->getEndcapExtLayer() - klmll->getEndcapHitLayer());

      //// see hit pattern
      //unsigned int hitpattern = klmll->getHitLayerPattern();
      //unsigned int extpattern = klmll->getExtLayerPattern();

      //for (int ii = 0; ii < 14; ii ++) {
      //  part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_hashit", get_binary(hitpattern, ii));
      //  part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_hasext", get_binary(extpattern, ii));
      //}

      double chiSquared = klmll->getChiSquared();
      int ndof = klmll->getDegreesOfFreedom();
      part->writeExtraInfo("KLMchi2", chiSquared);
      part->writeExtraInfo("KLMndof", ndof);



      //int nklmallhits = 0;
      //int nklmallontrackhits = 0;
      //// for all klm hit
      //for (int hh = 0; hh < m_klmHit2ds.getEntries(); ++hh) {
      //  nklmallhits += 1;
      //  KLMHit2d* klmhit = m_klmHit2ds[hh];

      //  unsigned long int hit_Zstripmin = klmhit->getZStripMin();
      //  unsigned long int hit_Zstripmax = klmhit->getZStripMax();
      //  double  hit_Zposition = klmhit->getPositionZ();
      //  double  hit_Xposition = klmhit->getPositionX();
      //  double  hit_Yposition = klmhit->getPositionY();
      //  double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
      //  unsigned long int hit_Phistripmin = klmhit->getPhiStripMin();
      //  unsigned long int hit_Phistripmax = klmhit->getPhiStripMax();
      //  unsigned long int hit_layer = klmhit->getLayer();
      //  unsigned long int hit_sector = klmhit->getSector();
      //  unsigned long int hit_section = klmhit->getSection();
      //  int ontrack = HitOnTrack(klmhit, track);
      //  nklmallontrackhits += ontrack;

      //  unsigned long int channelID = (hit_Zstripmin << 0) + (hit_Phistripmin << 6) + (hit_Zstripmax << 12) + (hit_Phistripmax << 18) +
      //                                (hit_layer << 24) + (hit_sector << 29) + (hit_section << 32);
      //  part->writeExtraInfo("AllHitchannelID_" + std::to_string(nklmallhits - 1), channelID);
      //  part->writeExtraInfo("AllHitZposition_" + std::to_string(nklmallhits - 1), hit_Zposition);
      //  part->writeExtraInfo("AllHitRposition_" + std::to_string(nklmallhits - 1), hit_Rposition);

      //}
      //part->writeExtraInfo("nklmallhits", nklmallhits);
      //part->writeExtraInfo("nklmallontrackhits", nklmallontrackhits);


      // for KLM cluster hits
      int nklmhits = 0;
      //int nklmontrackhits = 0;
      //int nklmontrackhitistrue = 0;
      //int nklmclusters = 0;
      //int hitTruthlist[3] = {0};

      // for hit pattern creation
      float previoushitposition[3] = {0.};
      //int previouslayer = -1;
      int maxBKLMlayer = 0;

      // find the outermost BKLM layer
      for (ExtHit& exthit : track->getRelationsTo<ExtHit>()) {

        if (exthit.getDetectorID() != Const::EDetector::BKLM) continue; // BKLM = 0x107

        int layer;
        int copyid = exthit.getCopyID();

        int section, sector, plane, strip;
        BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);

        if (maxBKLMlayer < layer) {
          maxBKLMlayer = layer;
        }
      }
      part->writeExtraInfo("BKLM_extlayer", maxBKLMlayer);


      for (KLMHit2d& klmhit : track->getRelationsTo<KLMHit2d>()) {
        //for (KLMCluster& klmcluster : track->getRelationsTo<KLMCluster>()) {

        //  // releation weight?
        //  nklmclusters += 1;

        //  for (KLMHit2d& klmhit : klmcluster.getRelationsTo<KLMHit2d>()) {
        nklmhits += 1;
        //const bklm::Module* bklmgeo = m_bklmGeoPar->findModule(klmhit.getSection(), klmhit.getSector(), klmhit.getLayer());
        ////std::cout << "has KLM hit" << std::endl;
        //CLHEP::Hep3Vector local  = bklmgeo->getLocalPosition(klmhit.getPhiStripMin(), klmhit.getZStripMin());
        //CLHEP::Hep3Vector global = bklmgeo->localToGlobal(local);
        //Zmin = klmhit.getPositionZ() < Zmin ? klmhit.getPositionZ() : Zmin;
        //Zmax = klmhit.getPositionZ() > Zmax ? klmhit.getPositionZ() : Zmax;
        //zstripmin = klmhit.getZStripMin() < zstripmin ? klmhit.getZStripMin() : zstripmin;
        //zstripmax = klmhit.getZStripMax() > zstripmax ? klmhit.getZStripMax() : zstripmax;
        //phistripmin = klmhit.getPhiStripMin();
        //phistripmax = klmhit.getPhiStripMax();
        ////std::cout << "Zstrip min = " << zstripmin << ", max = " << zstripmax << std::endl;
        //
        double  hit_Xposition = klmhit.getPositionX();
        double  hit_Yposition = klmhit.getPositionY();
        double  hit_Zposition = klmhit.getPositionZ();
        //double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
        double  hit_MinStripXposition = klmhit.getPositionXOfMinStrip();
        double  hit_MinStripYposition = klmhit.getPositionYOfMinStrip();
        double  hit_MinStripZposition = klmhit.getPositionZOfMinStrip();
        //double  hit_time = klmhit.getTime();
        //unsigned long int hit_Phistripmin;
        //unsigned long int hit_Phistripmax;
        //unsigned long int hit_Zstripmin  ;
        //unsigned long int hit_Zstripmax  ;
        //int ontrack = HitOnTrack(&klmhit, track);
        //nklmontrackhits += ontrack;

        unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
        //if (hit_inBKLM) {
        //  hit_Phistripmin = klmhit.getPhiStripMin();
        //  hit_Phistripmax = klmhit.getPhiStripMax();
        //  hit_Zstripmin   = klmhit.getZStripMin();
        //  hit_Zstripmax   = klmhit.getZStripMax();
        //} else {
        //  hit_Phistripmin = klmhit.getXStripMin();
        //  hit_Phistripmax = klmhit.getXStripMax();
        //  hit_Zstripmin = klmhit.getYStripMin();
        //  hit_Zstripmax = klmhit.getYStripMax();
        //}
        unsigned long int hit_layer = klmhit.getLayer();
        unsigned long int hit_sector = klmhit.getSector();
        unsigned long int hit_section = klmhit.getSection();

        /*
         * bit 1...6 for min Z strip, bit 7...12 for min phi strip, bit 13...18 for max Z strip, bit 19...24 for max phi strip
         * bit 1...5 for layer, bit 6...8 for sector, bit 9...10 for section, bit 11 for subdetector
         */
        //unsigned long int channelID = (hit_Zstripmin << 0) + (hit_Phistripmin << 6) + (hit_Zstripmax << 12) + (hit_Phistripmax << 18);
        //unsigned long int sensorID = (hit_layer << 0) + (hit_sector << 5) + (hit_section << 8) + (hit_inBKLM << 10);
        //part->writeExtraInfo("HitchannelID_" + std::to_string(nklmhits - 1), channelID);
        //part->writeExtraInfo("HitsensorID_" + std::to_string(nklmhits - 1), sensorID);
        //part->writeExtraInfo("HitXposition_" + std::to_string(nklmhits - 1), hit_Xposition);
        //part->writeExtraInfo("HitYposition_" + std::to_string(nklmhits - 1), hit_Yposition);
        //part->writeExtraInfo("HitZposition_" + std::to_string(nklmhits - 1), hit_Zposition);
        //part->writeExtraInfo("HitRposition_" + std::to_string(nklmhits - 1), hit_Rposition);
        //part->writeExtraInfo("Hittime_" + std::to_string(nklmhits - 1), hit_time);

        //part->writeExtraInfo("HitMinStripXposition_" + std::to_string(nklmhits - 1), hit_MinStripXposition);
        //part->writeExtraInfo("HitMinStripYposition_" + std::to_string(nklmhits - 1), hit_MinStripYposition);
        //part->writeExtraInfo("HitMinStripZposition_" + std::to_string(nklmhits - 1), hit_MinStripZposition);

        //int matchedHits = GetMatchedHits(klmhit, hit_inBKLM, mcpart);
        //part->writeExtraInfo("HitTruth_" + std::to_string(nklmhits - 1), matchedHits);
        //part->writeExtraInfo("HitonTrack_" + std::to_string(nklmhits - 1), ontrack);

        //if (matchedHits > -0.5 && matchedHits < 2.5) hitTruthlist[matchedHits]++;
        //if (matchedHits == 2) nklmontrackhitistrue += ontrack;

        // for hit pattern creation
        //if (!ontrack) continue;
        //if (hit_inBKLM && (hit_layer > maxBKLMlayer)) continue;
        int efflayer = hit_layer + maxBKLMlayer * (1 - hit_inBKLM);
        //if (efflayer == previouslayer) continue;
        //previouslayer = efflayer;

        float width;
        if (hit_inBKLM) {
          float phiwidth = sqrt(pow(hit_Xposition - hit_MinStripXposition, 2) + pow(hit_Yposition - hit_MinStripYposition, 2)) + 2;
          width = sqrt(pow(phiwidth, 2) + pow(fabs(hit_Zposition - hit_MinStripZposition) + 2, 2));
        } else {
          width = sqrt(pow(fabs(hit_Xposition - hit_MinStripXposition) + 2, 2) + pow(fabs(hit_Yposition - hit_MinStripYposition) + 2, 2));
        }

        float steplength = 0.;
        if (nklmhits > 1) { // do not apply any hit selection because of this criteria
          steplength = sqrt(pow((hit_Xposition - previoushitposition[0]), 2) + pow((hit_Yposition - previoushitposition[1]),
                            2) + pow((hit_Zposition - previoushitposition[2]), 2));
        }
        std::vector<float> hitattribute = {hit_Xposition, hit_Yposition, hit_Zposition, width, steplength};
        m_attributelist_hit.push_back(hitattribute);
        m_layerinfo_hit.push_back(efflayer);
        previoushitposition[0] = hit_Xposition;
        previoushitposition[1] = hit_Yposition;
        previoushitposition[2] = hit_Zposition;

        //} // loop of hit in cluster
        //} // loop of KLMCluster
      } // loop of hit on track

      part->writeExtraInfo("nklmhits", nklmhits);
      //part->writeExtraInfo("nklmclusters", nklmclusters);
      //part->writeExtraInfo("hittruth0", hitTruthlist[0]);
      //part->writeExtraInfo("hittruth1", hitTruthlist[1]);
      //part->writeExtraInfo("hittruth2", hitTruthlist[2]);
      //part->writeExtraInfo("nklmontrackhitistrue", nklmontrackhitistrue);
      //part->writeExtraInfo("nklmontrackhits", nklmontrackhits);
      //for (int ii=0; ii < 480; ii ++){
      //  part->writeExtraInfo("BKLM_hitmap_"+std::to_string(ii), stripmaps[ii]); // do not work. Force unsigned long int to double
      //}
      //part->writeExtraInfo("Zstripmin", zstripmin);
      //part->writeExtraInfo("Zstripmax", zstripmax);
      //part->writeExtraInfo("Zpositionmin", Zmin);
      //part->writeExtraInfo("Zpositionmax", Zmax);
      //part->writeExtraInfo("Phistripmax", phistripmax);
      //part->writeExtraInfo("Phistripmin", phistripmin);
      //std::cout << "Number of hits in this KLMCluster is " << nklmhits << std::endl;

      int nklmexthits = 0;
      //double lastHitRposition = 0;

      std::map<int, ExtHit*> ExtHitMap;

      for (ExtHit& exthit : track->getRelationsTo<ExtHit>()) {

        if (exthit.getDetectorID() < 0x100) continue; // BKLM = 0x107, EKLM = 0x207

        int layer;
        int inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
        int copyid = exthit.getCopyID();


        if (inBKLM) {
          int section, sector, plane, strip;
          BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);
        } else {
          int section, sector, plane, strip;
          EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyid, &section, &layer, &sector, &plane, &strip);
        }

        int currentlayer = layer + maxBKLMlayer * (1 - inBKLM);
        ExtHitMap[currentlayer] = &exthit; // only keep the last ext hit in each layer
      }

      for (auto itermap = ExtHitMap.begin(); itermap != ExtHitMap.end(); itermap ++) {

        ExtHit exthit = *(itermap->second);
        int currentlayer = itermap->first;

        ROOT::Math::XYZVector vector = exthit.getPosition(); // for release 8
        //TVector3 vector = exthit.getPosition(); // for release 7
        double  hit_Zposition = vector.Z();
        double  hit_Xposition = vector.X();
        double  hit_Yposition = vector.Y();
        //double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
        int inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
        //int copyid = exthit.getCopyID();
        int layer = currentlayer - maxBKLMlayer * (1 - inBKLM);
        //int currentlayer = layer + maxBKLMlayer * (1 - inBKLM);

        //if (hit_Rposition - lastHitRposition < 5) continue; // skip the too small step extrapolation

        //lastHitRposition = hit_Rposition;

        nklmexthits += 1;

        //part->writeExtraInfo("ExtXposition_" + std::to_string(nklmexthits - 1), hit_Xposition);
        //part->writeExtraInfo("ExtYposition_" + std::to_string(nklmexthits - 1), hit_Yposition);
        //part->writeExtraInfo("ExtZposition_" + std::to_string(nklmexthits - 1), hit_Zposition);
        //part->writeExtraInfo("ExtRposition_" + std::to_string(nklmexthits - 1), hit_Rposition);
        part->writeExtraInfo("Extcopyid_" + std::to_string(nklmexthits - 1), (layer + inBKLM * 100));

        std::vector<float> attributelist;
        std::vector<float> hitattribute = {hit_Xposition, hit_Yposition, hit_Zposition};
        m_attributelist_ext.push_back(hitattribute);
        m_layerinfo_ext.push_back(currentlayer);

      }
      part->writeExtraInfo("nklmexthits", nklmexthits);


      ExtHitMap.clear();


      //int nklmmuidhits = 0;

      //for (KLMMuidHit& muidhit : track->getRelationsTo<KLMMuidHit>()) {

      //  nklmmuidhits  += 1;
      //  int muidhitlayer = muidhit.getLayer(); // use more reliable criteria in the future?
      //  double muidhitchi2 =  muidhit.getChiSquared();
      //  part->writeExtraInfo("Muidhitchi2_" + std::to_string(nklmmuidhits - 1), muidhitchi2);
      //  part->writeExtraInfo("Muidhitlayer_" + std::to_string(nklmmuidhits - 1), muidhitlayer);
      //}
      //part->writeExtraInfo("nklmmuidhits", nklmmuidhits);

      getApplicationVariables(part);
    }

  }

  // for (auto& klmll : m_klmMuidLikelihoods) {

  //   B2INFO("Local calculated PDF : " <<  builder->getPDF(&klmll));
  //   B2INFO("Stored PDF : " << klmll.getPDFValue(13) );

  //   B2INFO("LongitudinalPDF : " <<  builder->getLongitudinalPDF(&klmll));
  //   B2INFO("TransversePDF : " <<  builder->getTransversePDF(&klmll));

  // }


}

void KLMMuIDGetterModule::getStripPosition()
{

  std::ofstream outFile;
  outFile.open("KLMpixelGeo.csv", std::ios::out);
  outFile << "section\tsector\tlayer\tphistrip\tzstrip\tphistripwidth\tzstripwidth\tlocalx\tlocaly\tlocalz\tglobalx\tglobaly\tglobalz"
          << std::endl;

  // get the module. para: section, sector, layer
  for (int layer = 1; layer <= 15; layer++) {
    for (int sector = 1; sector <= 8; sector++) {
      for (int section = 0; section <= 1; section++) {
        const bklm::Module* Bmodule = m_bklmGeoPar->findModule(section, sector, layer);
        int phistripmin = Bmodule -> getPhiStripMin();
        int phistripmax = Bmodule -> getPhiStripMax();
        int zstripmin = Bmodule -> getZStripMin();
        int zstripmax = Bmodule -> getZStripMax();
        //std::cout << "************************************" << std::endl;
        //std::cout << "section = " << section << ", sector = " << sector << ", layer = " << layer << std::endl;
        //std::cout << "phistrip = " << phistripmin << " ~ " << phistripmax << std::endl;
        //std::cout << "zstrip = " << zstripmin << " ~ " << zstripmax << std::endl;
        //std::cout << "************************************" << std::endl;
        for (int phistrip = phistripmin; phistrip <= phistripmax; phistrip++) {
          for (int zstrip = zstripmin; zstrip <= zstripmax; zstrip++) {
            CLHEP::Hep3Vector local = Bmodule->getLocalPosition(phistrip, zstrip);
            CLHEP::Hep3Vector global = Bmodule->localToGlobal(local + Bmodule->getLocalReconstructionShift(), true);
            double phistripwidth = Bmodule->getPhiStripWidth();
            double zstripwidth = Bmodule->getZStripWidth();
            outFile << section << "\t" << sector << "\t" << layer << "\t" << phistrip << "\t" << zstrip << "\t" << phistripwidth << "\t" <<
                    zstripwidth << "\t" << local.x() << "\t" << local.y() << "\t" << local.z() << "\t" << global.x() << "\t" << global.y() << "\t" <<
                    global.z() << std::endl;
            //std::cout << "phi,z = " << phistrip << "," << zstrip <<": local = " << local.x() << ", " << local.y() << ", " << local.z() << std::endl;
            //std::cout << "phi,z = " << phistrip << "," << zstrip <<": global = " << global.x() << ", " << global.y() << ", " << global.z() << std::endl;
          }
        }
      }
    }
  }
  outFile.close();
}

int KLMMuIDGetterModule::GetMatchedHits(KLMHit2d& klmhit, int hit_inBKLM, const MCParticle* mcpart)
{
  int matchedHits = 0;
  if (mcpart) {
    //std::cout << "mcpart = " << mcpart << std::endl;
    //std::cout << "new hit in layer " << klmhit.getLayer() << ", inbklm = " << hit_inBKLM << std::endl;
    if (hit_inBKLM) {
      RelationVector<BKLMHit1d> hit1drelations = klmhit.getRelationsTo<BKLMHit1d>();
      BKLMHit1d* hit1d1, *hit1d2;
      hit1d1 = hit1drelations[0];
      hit1d2 = hit1drelations[1];
      RelationVector<KLMDigit> digitrelations1 = hit1d1->getRelationsTo<KLMDigit>();
      RelationVector<KLMDigit> digitrelations2 = hit1d2->getRelationsTo<KLMDigit>();
      CheckMCParticle(digitrelations1, mcpart, matchedHits);
      CheckMCParticle(digitrelations2, mcpart, matchedHits);
    } else {
      RelationVector<KLMDigit> digitrelations = klmhit.getRelationsTo<KLMDigit>();
      KLMDigit* digit1, *digit2;
      digit1 = digitrelations[0];
      digit2 = digitrelations[1];
      CheckMCParticle(digit1, mcpart, matchedHits);
      CheckMCParticle(digit2, mcpart, matchedHits);
    }
    return matchedHits;
  } else {
    return -1;
  }

}

void KLMMuIDGetterModule::CheckMCParticle(RelationVector<KLMDigit>& digitrelations, const MCParticle* mcpart, int& matchedHits)
{
  // for BKLM
  for (KLMDigit& digit : digitrelations) {
    RelationVector<KLMSimHit> simhitrelations = digit.getRelationsTo<KLMSimHit>();
    for (KLMSimHit& simhit : simhitrelations) {
      MCParticle* mcpart1 = simhit.getRelatedFrom<MCParticle>();
      if (IterateMCMother(mcpart1, mcpart)) {
        matchedHits++;
        return;
      }
    }
  }
}

void KLMMuIDGetterModule::CheckMCParticle(KLMDigit* digit, const MCParticle* mcpart, int& matchedHits)
{
  // for EKLM
  RelationVector<KLMSimHit> simhitrelations = digit->getRelationsTo<KLMSimHit>();
  for (KLMSimHit& simhit : simhitrelations) {
    MCParticle* mcpart1 = simhit.getRelatedFrom<MCParticle>();
    if (IterateMCMother(mcpart1, mcpart)) {
      matchedHits++;
      return;
    }
  }
}

int KLMMuIDGetterModule::IterateMCMother(MCParticle* mcpart1, const MCParticle* mcpart)
{
  if (!mcpart1) return 0;
  if (mcpart1 == mcpart) {
    return 1;
  } else {
    return IterateMCMother(mcpart1->getMother(), mcpart);
  }
}


int KLMMuIDGetterModule::HitOnTrack(KLMHit2d* klmhit, const Track* track)
{
  for (KLMHit2d& klmhit2d : track->getRelationsTo<KLMHit2d>()) {
    if (&klmhit2d == klmhit) {
      return 1;
    }
  }
  return 0;
}

void KLMMuIDGetterModule::getApplicationVariables(Particle* part)
{
  part->writeExtraInfo("nninput_0", part->getExtraInfo("KLMchi2"));
  part->writeExtraInfo("nninput_1", part->getExtraInfo("KLMndof"));
  part->writeExtraInfo("nninput_2", Variable::particleP(part));
  part->writeExtraInfo("nninput_3", Variable::particlePt(part));
  part->writeExtraInfo("nninput_4", Variable::particleTheta(part));
  part->writeExtraInfo("nninput_5", Variable::particlePhi(part));

  int lasthitlayer = 0, lastextlayer = 0;
  if (!m_layerinfo_hit.empty()) {lasthitlayer = m_layerinfo_hit.back();}
  if (!m_layerinfo_ext.empty()) {lastextlayer = m_layerinfo_ext.back();}

  part->writeExtraInfo("nninput_6", lastextlayer - lasthitlayer);
  part->writeExtraInfo("nninput_7", lastextlayer);

  int MAXLAYER = 29;
  int nthhit = 0;
  int nthext = 0;

  for (int layer = 0; layer < MAXLAYER; layer ++) {

    int currentlayer = layer + 1;
    int hashit = 0;
    int hasext = 0;

    if (nthhit < m_layerinfo_hit.size()) {
      hashit = (int)(m_layerinfo_hit[nthhit] == currentlayer);
    }
    if (nthext < m_layerinfo_ext.size()) {
      hasext = (int)(m_layerinfo_ext[nthext] == currentlayer);
    }

    if (hashit) {
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 0), m_attributelist_hit[nthhit][3]); // width
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 1), m_attributelist_hit[nthhit][4]); // steplength
    } else {
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 0), 0.); // width
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 1), 0.); // steplength
    }

    if (hasext && hashit) {
      float Xdiff = m_attributelist_hit[nthhit][0] -  m_attributelist_ext[nthext][0];
      float Ydiff = m_attributelist_hit[nthhit][1] -  m_attributelist_ext[nthext][1];
      float Zdiff = m_attributelist_hit[nthhit][2] -  m_attributelist_ext[nthext][2];
      float distance = sqrt(pow(Xdiff, 2) + pow(Ydiff, 2) + pow(Zdiff, 2));
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 2), distance); // distance
    } else {
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 2), 0.); // distance
    }

    if (hashit) nthhit++;
    if (hasext) {
      nthext++;
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 3), 1); // hasext
    } else {
      part->writeExtraInfo("nninput_" + std::to_string(8 + 4 * layer + 3), 0); // hasext
    }

  }
}


