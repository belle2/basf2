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

#include <klm/modules/KLMHitStudy/KLMHitStudyModule.h>

#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
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

REG_MODULE(KLMHitStudy);

KLMHitStudyModule::KLMHitStudyModule() : Module()//, m_bklmGeoPar(nullptr)
{
  // Set module properties
  setDescription(R"DOC(Get information from KLMMuIDLikelihood)DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName,
           "list of input ParticleList name", std::string(""));
  addParam("hitChiCut", m_hitChiCut,
           "Hit chi cut", 3.5);
}

void KLMHitStudyModule::initialize()
{
  //m_klmMuidLikelihoods.isRequired();

  StoreObjPtr<ParticleList>().isRequired(m_inputListName);
  m_klmHit2ds.isRequired();

  m_bklmGeoPar = bklm::GeometryPar::instance();
  //getStripPosition();
}

void KLMHitStudyModule::event()
{
  // auto builder = new MuidBuilder(13);
  mubuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(13));
  pibuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(211));

  const StoreObjPtr<ParticleList> plist(m_inputListName);

  const unsigned int nParticles = plist->getListSize();

  for (unsigned int iPart = 0; iPart < nParticles; iPart++) {

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

      // auto klmll = part->getTrack()->getRelatedTo<KLMMuidLikelihood>();
      auto track = part->getTrack();

      auto klmll = track->getRelatedTo<KLMMuidLikelihood>();

      if (klmll) {

        unsigned int extoutcome = klmll->getOutcome();

        part->writeExtraInfo("KLM_hitlayer", klmll->getHitLayer());
        part->writeExtraInfo("KLM_extlayer", klmll->getExtLayer());
        part->writeExtraInfo("KLM_outlayerdiff", klmll->getExtLayer() - klmll->getHitLayer());
        //part->writeExtraInfo("KLM_hitlayerpattern", klmll->getHitLayerPattern());
        //part->writeExtraInfo("KLM_extlayerpattern", klmll->getExtLayerPattern());

        //part->writeExtraInfo("BKLM_hitlayer", klmll->getBarrelHitLayer());
        //part->writeExtraInfo("BKLM_extlayer", barrelExtLayer);
        part->writeExtraInfo("extOutcome", extoutcome);
        //part->writeExtraInfo("BKLM_outlayerdiff", klmll->getBarrelExtLayer() - klmll->getBarrelHitLayer());

        //part->writeExtraInfo("EKLM_hitlayer", klmll->getEndcapHitLayer());
        //part->writeExtraInfo("EKLM_extlayer", endcapExtLayer);
        //part->writeExtraInfo("EKLM_outlayerdiff", klmll->getEndcapExtLayer() - klmll->getEndcapHitLayer());

        double chiSquared = klmll->getChiSquared();
        int ndof = klmll->getDegreesOfFreedom();
        part->writeExtraInfo("KLMchi2", chiSquared);
        part->writeExtraInfo("KLMndof", ndof);

        // initialize hit pattern arrays
        for (int layer = 0; layer < 29; layer++) {
          m_hitpattern_distance[layer] = -1;
          m_hitpattern_steplength[layer] = -1;
          m_hitpattern_width[layer] = -1;
          m_hitpattern_chi2[layer] = -1;
          m_hitpattern_nhits[layer] = 0;
          m_hitpattern_hasext[layer] = 0;
        }

        std::map<int, ExtHit*> ExtHitMap;
        int maxBKLMlayer = 0;

        for (ExtHit& exthit : track->getRelationsTo<ExtHit>()) {

          if (exthit.getDetectorID() < 0x100) continue; // BKLM = 0x107, EKLM = 0x207

          int layer;
          int inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
          int copyid = exthit.getCopyID();

          int section, sector, plane, strip;

          if (inBKLM) {
            BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);
            maxBKLMlayer = maxBKLMlayer < layer ? layer : maxBKLMlayer;
            m_hitpattern_hasext[layer - 1] = 1;
          } else {
            EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyid, &section, &layer, &sector, &plane, &strip);
            m_hitpattern_hasext[15 + layer - 1] = 1;
          }

          //int index = inBKLM*10000 + section*1000 + sector*100 + layer;
          //int index = inBKLM*1000 + section*100 + layer;
          int index = (1 - inBKLM) * 100 + layer; // make sure BKLM hits is in front of EKLM hits
          ExtHitMap[index] = &exthit; // only keep the last ext hit in each layer
        }
        part->writeExtraInfo("BKLM_extlayer", maxBKLMlayer);

        int nklmexthits = 0;
        int lastextlayer = 0;
        float extdepth = 0.;
        float previousextposition[3] = {0.};
        for (auto itermap = ExtHitMap.begin(); itermap != ExtHitMap.end(); itermap ++) {

          ExtHit exthit = *(itermap->second);
          int index = itermap->first;

          ROOT::Math::XYZVector vector = exthit.getPosition(); // for release 8
          double  hit_Zposition = vector.Z();
          double  hit_Xposition = vector.X();
          double  hit_Yposition = vector.Y();
          //double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
          int inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
          //int copyid = exthit.getCopyID();
          //int layer = currentlayer - maxBKLMlayer * (1 - inBKLM);
          int layer = index % 100;
          lastextlayer = layer + maxBKLMlayer * (1 - inBKLM);

          //if (hit_Rposition - lastHitRposition < 5) continue; // skip the too small step extrapolation

          //lastHitRposition = hit_Rposition;

          nklmexthits += 1;

          // extrapolation penetration depth calculation.
          if (nklmexthits > 1) {
            extdepth += sqrt(pow((hit_Xposition - previousextposition[0]), 2) + pow((hit_Yposition - previousextposition[1]),
                             2) + pow((hit_Zposition - previousextposition[2]), 2));
          }
          previousextposition[0] = hit_Xposition;
          previousextposition[1] = hit_Yposition;
          previousextposition[2] = hit_Zposition;

          //part->writeExtraInfo("ExtXposition_" + std::to_string(nklmexthits - 1), hit_Xposition);
          //part->writeExtraInfo("ExtYposition_" + std::to_string(nklmexthits - 1), hit_Yposition);
          //part->writeExtraInfo("ExtZposition_" + std::to_string(nklmexthits - 1), hit_Zposition);
          part->writeExtraInfo("Extcopyid_" + std::to_string(nklmexthits - 1), (index));
        }
        part->writeExtraInfo("nklmexthits", nklmexthits);
        part->writeExtraInfo("lastextlayer", lastextlayer);
        part->writeExtraInfo("extdepth", extdepth);

        std::multimap<int, KLMHit2d*> Hit2dMap;
        //for (KLMCluster& klmcluster : track->getRelationsTo<KLMCluster>()) {
        //  for (KLMHit2d& klmhit : klmcluster.getRelationsTo<KLMHit2d>()) {

        //    unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
        //    unsigned long int hit_layer = klmhit.getLayer();
        //    unsigned long int hit_sector = klmhit.getSector();
        //    unsigned long int hit_section = klmhit.getSection();

        //    //int index = (1-hit_inBKLM)*10000 + hit_section*1000 + hit_sector*100 + hit_layer;
        //    //int index = hit_inBKLM*1000 + hit_section*100 + hit_layer;
        //    int index = (1-hit_inBKLM)*100 + hit_layer; // BKLM hits are in front of EKLM hits
        //    Hit2dMap.insert(std::pair<int, KLMHit2d*>{index,  &klmhit}); // multimap is used here so all hits are kept even if they have the same index
        //  }
        //}

        for (KLMHit2d& klmhit : track->getRelationsTo<KLMHit2d>("", "chidimx")) {
          unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
          unsigned long int hit_layer = klmhit.getLayer();
          unsigned long int hit_sector = klmhit.getSector();
          unsigned long int hit_section = klmhit.getSection();

          //int index = (1-hit_inBKLM)*10000 + hit_section*1000 + hit_sector*100 + hit_layer;
          //int index = hit_inBKLM*1000 + hit_section*100 + hit_layer;
          int index = (1 - hit_inBKLM) * 100 + hit_layer; // BKLM hits are in front of EKLM hits
          Hit2dMap.insert(std::pair<int, KLMHit2d*> {index,  &klmhit}); // multimap is used here so all hits are kept even if they have the same index
        }

        int nklmhits = 0;
        int nklmontrackhits = 0;
        int nklmontrackhitistrue = 0;
        //int nklmclusters = 0;
        int hitTruthlist[3] = {0};
        float penetrationdepth = 0.;
        float previoushitposition[3] = {0.};
        int previouslayer = 0;
        RelationVector<KLMHit2d> KLMHit2drelationx = track->getRelationsTo<KLMHit2d>("", "chidimx");
        RelationVector<KLMHit2d> KLMHit2drelationy = track->getRelationsTo<KLMHit2d>("", "chidimy");
        RelationVector<KLMHit2d> KLMHit2drelationchi2 = track->getRelationsTo<KLMHit2d>("", "intersectchisq");
        float KFtotalchi2 = 0;
        int KFndof = 0;
        for (auto itermap = Hit2dMap.begin(); itermap != Hit2dMap.end(); itermap ++) {

          KLMHit2d* klmhitadd = itermap->second;
          KLMHit2d klmhit = *klmhitadd;
          int index = itermap->first;

          nklmhits += 1;
          double  hit_Xposition = klmhit.getPositionX();
          double  hit_Yposition = klmhit.getPositionY();
          double  hit_Zposition = klmhit.getPositionZ();
          double  hit_MinStripXposition = klmhit.getPositionXOfMinStrip();
          double  hit_MinStripYposition = klmhit.getPositionYOfMinStrip();
          double  hit_MinStripZposition = klmhit.getPositionZOfMinStrip();
          double  hit_time = klmhit.getTime();
          unsigned long int hit_Phistripmin;
          unsigned long int hit_Phistripmax;
          unsigned long int hit_Zstripmin  ;
          unsigned long int hit_Zstripmax  ;
          int ontrack = HitOnTrack(klmhitadd, track);
          nklmontrackhits += ontrack;

          unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
          if (hit_inBKLM) {
            hit_Phistripmin = klmhit.getPhiStripMin();
            hit_Phistripmax = klmhit.getPhiStripMax();
            hit_Zstripmin   = klmhit.getZStripMin();
            hit_Zstripmax   = klmhit.getZStripMax();
          } else {
            hit_Phistripmin = klmhit.getXStripMin();
            hit_Phistripmax = klmhit.getXStripMax();
            hit_Zstripmin = klmhit.getYStripMin();
            hit_Zstripmax = klmhit.getYStripMax();
          }
          unsigned long int hit_layer = klmhit.getLayer();
          //unsigned long int hit_sector = klmhit.getSector();
          //unsigned long int hit_section = klmhit.getSection();

          // find the hit index in relations
          int targethit = -1;
          float KFchix = 999.;
          float KFchiy = 999.;
          float KFchi2 = 999.;
          for (unsigned int iHit = 0; iHit < KLMHit2drelationx.size(); iHit++) {
            KLMHit2d* klmhit1 = KLMHit2drelationx[iHit];
            if (klmhitadd == klmhit1) {
              targethit = iHit;
              break;
            }
          }
          if (targethit > -1) {
            KFchix = KLMHit2drelationx.weight(targethit);
            KFchiy = KLMHit2drelationy.weight(targethit);
            KFchi2 = KLMHit2drelationchi2.weight(targethit);
            part->writeExtraInfo("Hitchix_" + std::to_string(nklmhits - 1), KFchix);
            part->writeExtraInfo("Hitchiy_" + std::to_string(nklmhits - 1), KFchiy);
            part->writeExtraInfo("Hitchi2_" + std::to_string(nklmhits - 1), KFchi2);
          }

          /*
           * bit 1...6 for min Z strip, bit 7...12 for min phi strip, bit 13...18 for max Z strip, bit 19...24 for max phi strip
           */
          unsigned long int channelID = (hit_Zstripmin << 0) + (hit_Phistripmin << 6) + (hit_Zstripmax << 12) + (hit_Phistripmax << 18);
          int isMultiStripHit = (abs(hit_Phistripmax - hit_Phistripmin) || (hit_Zstripmax - hit_Zstripmin));
          int hit_inRPC = (int) klmhit.inRPC();
          int isScintMulti = (int)((!hit_inRPC) and isMultiStripHit);
          part->writeExtraInfo("HitchannelID_" + std::to_string(nklmhits - 1), channelID);
          part->writeExtraInfo("HitsensorID_" + std::to_string(nklmhits - 1), index);
          part->writeExtraInfo("HitXposition_" + std::to_string(nklmhits - 1), hit_Xposition);
          part->writeExtraInfo("HitYposition_" + std::to_string(nklmhits - 1), hit_Yposition);
          part->writeExtraInfo("HitZposition_" + std::to_string(nklmhits - 1), hit_Zposition);
          part->writeExtraInfo("Hittime_" + std::to_string(nklmhits - 1), hit_time);
          part->writeExtraInfo("Hitmulti_" + std::to_string(nklmhits - 1), isMultiStripHit);
          part->writeExtraInfo("HitinRPC_" + std::to_string(nklmhits - 1), hit_inRPC);
          part->writeExtraInfo("HitScintMulti_" + std::to_string(nklmhits - 1), isScintMulti);

          part->writeExtraInfo("HitMinStripXposition_" + std::to_string(nklmhits - 1), hit_MinStripXposition);
          part->writeExtraInfo("HitMinStripYposition_" + std::to_string(nklmhits - 1), hit_MinStripYposition);
          part->writeExtraInfo("HitMinStripZposition_" + std::to_string(nklmhits - 1), hit_MinStripZposition);

          int matchedHits = GetMatchedHits(klmhitadd, hit_inBKLM, mcpart);
          part->writeExtraInfo("HitTruth_" + std::to_string(nklmhits - 1), matchedHits);
          part->writeExtraInfo("HitonTrack_" + std::to_string(nklmhits - 1), ontrack);

          if (matchedHits > -0.5 && matchedHits < 2.5) hitTruthlist[matchedHits]++;
          if (matchedHits == 2) nklmontrackhitistrue += ontrack;

          // for hit pattern creation
          //if (!ontrack) continue;


          float width;
          if (hit_inBKLM) {
            float phiwidth = sqrt(pow(hit_Xposition - hit_MinStripXposition, 2) + pow(hit_Yposition - hit_MinStripYposition, 2)) + 2;
            width = sqrt(pow(phiwidth, 2) + pow(fabs(hit_Zposition - hit_MinStripZposition) + 2, 2));
          } else {
            width = sqrt(pow(fabs(hit_Xposition - hit_MinStripXposition) + 2, 2) + pow(fabs(hit_Yposition - hit_MinStripYposition) + 2, 2));
          }
          part->writeExtraInfo("Hitwidth_" + std::to_string(nklmhits - 1), width);

          // calculate distance
          float distance = -9999;
          if (ExtHitMap.count(index)) {
            ExtHit exthit = *(ExtHitMap[index]);

            ROOT::Math::XYZVector vector = exthit.getPosition(); // for release 8
            double  ext_Zposition = vector.Z();
            double  ext_Xposition = vector.X();
            double  ext_Yposition = vector.Y();

            float Xdiff = ext_Xposition - hit_Xposition;
            float Ydiff = ext_Yposition - hit_Yposition;
            float Zdiff = ext_Zposition - hit_Zposition;
            distance = sqrt(pow(Xdiff, 2) + pow(Ydiff, 2) + pow(Zdiff, 2));
          } else {

            // find the distance to the closest exthit
            for (auto iterext = ExtHitMap.begin(); iterext != ExtHitMap.end(); iterext ++) {

              ExtHit exthit = *(iterext->second);

              ROOT::Math::XYZVector vector = exthit.getPosition(); // for release 8
              double  ext_Zposition = vector.Z();
              double  ext_Xposition = vector.X();
              double  ext_Yposition = vector.Y();

              float Xdiff = ext_Xposition - hit_Xposition;
              float Ydiff = ext_Yposition - hit_Yposition;
              float Zdiff = ext_Zposition - hit_Zposition;
              float tempdistance = (-1) * sqrt(pow(Xdiff, 2) + pow(Ydiff, 2) + pow(Zdiff,
                                               2)); // set distance to minus if there's no ext hit in this layer
              if (distance < tempdistance) distance = tempdistance;
            }

          }
          part->writeExtraInfo("Hitdistance_" + std::to_string(nklmhits - 1), distance);

          int Hit_selected = (int)(KFchix < m_hitChiCut && KFchiy < m_hitChiCut);
          part->writeExtraInfo("Hitselected_" + std::to_string(nklmhits - 1), Hit_selected);

          if (not Hit_selected) continue;
          //int efflayer = hit_layer + maxBKLMlayer*(1-hit_inBKLM);
          //if (efflayer == previouslayer) continue; // only the first hit is used in case there are > 1 hits in one layer
          //previouslayer = efflayer;
          KFtotalchi2 += KFchi2;
          KFndof += 2;
          // penetration depth calculation. only the first hit on each are used
          float steplength = 0.;
          if (KFndof > 2) {
            steplength = sqrt(pow((hit_Xposition - previoushitposition[0]), 2) + pow((hit_Yposition - previoushitposition[1]),
                              2) + pow((hit_Zposition - previoushitposition[2]), 2));
            penetrationdepth += steplength;
          }
          previoushitposition[0] = hit_Xposition;
          previoushitposition[1] = hit_Yposition;
          previoushitposition[2] = hit_Zposition;
          part->writeExtraInfo("Hitstep_" + std::to_string(nklmhits - 1), steplength);

          // hit pattern creation. All selected hits will be used
          int hitpatternindex = hit_layer - 1 + 15 * (1 - hit_inBKLM);
          m_hitpattern_nhits[hitpatternindex] += 1;
          m_hitpattern_distance[hitpatternindex] = fabs(distance);
          m_hitpattern_chi2[hitpatternindex] = KFchi2;
          m_hitpattern_steplength[hitpatternindex] = steplength;
          m_hitpattern_width[hitpatternindex] = width;
        } // loop of Hit2dMap

        part->writeExtraInfo("nklmhits", nklmhits);
        part->writeExtraInfo("hittruth0", hitTruthlist[0]);
        part->writeExtraInfo("hittruth1", hitTruthlist[1]);
        part->writeExtraInfo("hittruth2", hitTruthlist[2]);
        part->writeExtraInfo("nklmontrackhitistrue", nklmontrackhitistrue);
        part->writeExtraInfo("nklmontrackhits", nklmontrackhits);
        part->writeExtraInfo("penetrationdepth", penetrationdepth);
        part->writeExtraInfo("lasthitlayer", previouslayer);
        part->writeExtraInfo("KFtotalchi2", KFtotalchi2);
        part->writeExtraInfo("KFndof", KFndof);

        ExtHitMap.clear();
        Hit2dMap.clear();

        getApplicationVariables(part);
      } // has klmll
    } // has mcpart
  } // loop of particles
}

void KLMHitStudyModule::getStripPosition()
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

int KLMHitStudyModule::GetMatchedHits(KLMHit2d* klmhit, int hit_inBKLM, const MCParticle* mcpart)
{
  int matchedHits = 0;
  if (mcpart) {
    //std::cout << "mcpart = " << mcpart << std::endl;
    //std::cout << "new hit in layer " << klmhit.getLayer() << ", inbklm = " << hit_inBKLM << std::endl;
    if (hit_inBKLM) {
      RelationVector<BKLMHit1d> hit1drelations = klmhit->getRelationsTo<BKLMHit1d>();
      if (hit1drelations.size() != 2) return 0;
      BKLMHit1d* hit1d1, *hit1d2;
      hit1d1 = hit1drelations[0];
      hit1d2 = hit1drelations[1];
      RelationVector<KLMDigit> digitrelations1 = hit1d1->getRelationsTo<KLMDigit>();
      RelationVector<KLMDigit> digitrelations2 = hit1d2->getRelationsTo<KLMDigit>();
      CheckMCParticle(digitrelations1, mcpart, matchedHits);
      CheckMCParticle(digitrelations2, mcpart, matchedHits);
    } else {
      RelationVector<KLMDigit> digitrelations = klmhit->getRelationsTo<KLMDigit>();
      if (digitrelations.size() != 2) return 0;
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

void KLMHitStudyModule::CheckMCParticle(RelationVector<KLMDigit>& digitrelations, const MCParticle* mcpart, int& matchedHits)
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

void KLMHitStudyModule::CheckMCParticle(KLMDigit* digit, const MCParticle* mcpart, int& matchedHits)
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

int KLMHitStudyModule::IterateMCMother(MCParticle* mcpart1, const MCParticle* mcpart)
{
  if (!mcpart1) return 0;
  if (mcpart1 == mcpart) {
    return 1;
  } else {
    return IterateMCMother(mcpart1->getMother(), mcpart);
  }
}


int KLMHitStudyModule::HitOnTrack(KLMHit2d* klmhit, const Track* track)
{
  for (KLMHit2d& klmhit2d : track->getRelationsTo<KLMHit2d>()) {
    if (&klmhit2d == klmhit) {
      return 1;
    }
  }
  return 0;
}

void KLMHitStudyModule::getApplicationVariables(Particle* part)
{
  part->writeExtraInfo("nninput_0", part->getExtraInfo("KFtotalchi2"));
  part->writeExtraInfo("nninput_1", part->getExtraInfo("KFndof"));
  //part->writeExtraInfo("nninput_0", part->getExtraInfo("lastextlayer") - part->getExtraInfo("lasthitlayer"));
  part->writeExtraInfo("nninput_2", part->getExtraInfo("KLM_extlayer") - part->getExtraInfo("KLM_hitlayer"));
  part->writeExtraInfo("nninput_3", part->getExtraInfo("KLM_extlayer"));
  part->writeExtraInfo("nninput_4", part->getExtraInfo("penetrationdepth"));
  part->writeExtraInfo("nninput_5", part->getExtraInfo("extdepth"));
  part->writeExtraInfo("nninput_6", Variable::particleP(part));
  part->writeExtraInfo("nninput_7", Variable::particlePt(part));
  part->writeExtraInfo("nninput_8", Variable::particleTheta(part));
  part->writeExtraInfo("nninput_9", Variable::particlePhi(part));

  int MAXLAYER = 29;

  for (int layer = 0; layer < MAXLAYER; layer ++) {

    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 0), m_hitpattern_width[layer]); // width
    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 1), m_hitpattern_steplength[layer]); // steplength
    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 2), m_hitpattern_distance[layer]); // distance
    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 3), m_hitpattern_chi2[layer]); // chi2
    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 4), m_hitpattern_nhits[layer]); // nhits
    part->writeExtraInfo("nninput_" + std::to_string(10 + 6 * layer + 5), m_hitpattern_hasext[layer]); // hasext

  }
}


