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
#include <klm/dataobjects/KLMMuidHit.h>
//#include <klm/bklm/geometry/Module.h>
#include <klm/muid/MuidElementNumbers.h>

// #include <klm/muid/MuidBuilder.h>
#include <klm/muidgnn/MuidBuilder_fixed.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

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
  m_klmMuidLikelihoods.isRequired();

  StoreObjPtr<ParticleList>().isRequired(m_inputListName);
  m_klmHit2ds.isRequired();

  m_bklmGeoPar = bklm::GeometryPar::instance();
  getStripPosition();
}

void KLMMuIDGetterModule::event()
{
  // auto builder = new MuidBuilder(13);
  mubuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(13));
  pibuilder = std::unique_ptr<MuidBuilder_fixed>(new MuidBuilder_fixed(211));

  const StoreObjPtr<ParticleList> plist(m_inputListName);

  const unsigned int nParticles = plist->getListSize();

  for (unsigned int iPart = 0; iPart < nParticles; iPart++) {
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
      part->writeExtraInfo("KLM_mu_L_Likelihood", mubuilder->getLongitudinalPDF(klmll));
      part->writeExtraInfo("KLM_mu_T_Likelihood", mubuilder->getTransversePDF(klmll));
      part->writeExtraInfo("KLM_pi_L_Likelihood", pibuilder->getLongitudinalPDF(klmll));
      part->writeExtraInfo("KLM_pi_T_Likelihood", pibuilder->getTransversePDF(klmll));

      for (int ii = 0; ii < 15; ii ++) {
        part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_efficiency", double(mubuilder->getUnhitLayerEfficiency(klmll, 1, ii)));
      }
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

      for (int ii = 0; ii < 14; ii ++) {
        //part->writeExtraInfo("BKLM_muPDF_layer"+std::to_string(ii), builder->getLayerPDF(muhypo, klmll, ii));
        //part->writeExtraInfo("BKLM_piPDF_layer"+std::to_string(ii), builder->getLayerPDF(pihypo, klmll, ii));
        part->writeExtraInfo("BKLM_muPDF_layer" + std::to_string(ii), mubuilder->getLayerPDF(klmll, ii));
        part->writeExtraInfo("BKLM_piPDF_layer" + std::to_string(ii), pibuilder->getLayerPDF(klmll, ii));
      }

      part->writeExtraInfo("KLM_hitlayer", klmll->getHitLayer());
      part->writeExtraInfo("KLM_extlayer", klmll->getExtLayer());
      part->writeExtraInfo("KLM_outlayerdiff", klmll->getExtLayer() - klmll->getHitLayer());
      part->writeExtraInfo("KLM_hitlayerpattern", klmll->getHitLayerPattern());
      part->writeExtraInfo("KLM_extlayerpattern", klmll->getExtLayerPattern());

      part->writeExtraInfo("BKLM_hitlayer", klmll->getBarrelHitLayer());
      part->writeExtraInfo("BKLM_extlayer", klmll->getBarrelExtLayer());
      part->writeExtraInfo("BKLM_outlayerdiff", klmll->getBarrelExtLayer() - klmll->getBarrelHitLayer());

      part->writeExtraInfo("EKLM_hitlayer", klmll->getEndcapHitLayer());
      part->writeExtraInfo("EKLM_extlayer", klmll->getEndcapExtLayer());
      part->writeExtraInfo("EKLM_outlayerdiff", klmll->getEndcapExtLayer() - klmll->getEndcapHitLayer());

      // see hit pattern
      unsigned int hitpattern = klmll->getHitLayerPattern();
      unsigned int extpattern = klmll->getExtLayerPattern();

      for (int ii = 0; ii < 14; ii ++) {
        part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_hashit", get_binary(hitpattern, ii));
        part->writeExtraInfo("BKLM_layer" + std::to_string(ii) + "_hasext", get_binary(extpattern, ii));
      }

      double chiSquared = klmll->getChiSquared();
      int ndof = klmll->getDegreesOfFreedom();
      part->writeExtraInfo("KLMchi2", chiSquared);
      part->writeExtraInfo("KLMndof", ndof);

    }


    int nklmallhits = 0;
    // for all klm hit
    for (int hh = 0; hh < m_klmHit2ds.getEntries(); ++hh) {
      nklmallhits += 1;
      KLMHit2d* klmhit = m_klmHit2ds[hh];

      unsigned long int hit_Zstripmin = klmhit->getZStripMin();
      unsigned long int hit_Zstripmax = klmhit->getZStripMax();
      double  hit_Zposition = klmhit->getPositionZ();
      double  hit_Xposition = klmhit->getPositionX();
      double  hit_Yposition = klmhit->getPositionY();
      double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
      unsigned long int hit_Phistripmin = klmhit->getPhiStripMin();
      unsigned long int hit_Phistripmax = klmhit->getPhiStripMax();
      unsigned long int hit_layer = klmhit->getLayer();
      unsigned long int hit_sector = klmhit->getSector();
      unsigned long int hit_section = klmhit->getSection();

      unsigned long int channelID = (hit_Zstripmin << 0) + (hit_Phistripmin << 6) + (hit_Zstripmax << 12) + (hit_Phistripmax << 18) +
                                    (hit_layer << 24) + (hit_sector << 29) + (hit_section << 32);
      part->writeExtraInfo("AllHitchannelID_" + std::to_string(nklmallhits - 1), channelID);
      part->writeExtraInfo("AllHitZposition_" + std::to_string(nklmallhits - 1), hit_Zposition);
      part->writeExtraInfo("AllHitRposition_" + std::to_string(nklmallhits - 1), hit_Rposition);

    }
    part->writeExtraInfo("nklmallhits", nklmallhits);


    // for KLM cluster hits
    int nklmhits = 0;
    int zstripmin = 999;
    int zstripmax = 0;
    int nklmclusters = 0;
    float Zmin = 999;
    float Zmax = 0;
    //int phistripmin = 999;
    //int phistripmax = 0;

    ///*
    // * 0...239 for section 0 (BB) and 240...479 for section 1 (BF);
    // * (mod 240) 0...29 for sector 1, 30...59 for sector 2, 60...89 for sector 3, 90...119 for sector 4, 120...149 for sector 5, 150...179 for sector 6, 180...209 for sector 7, 210...239 for sector 8;
    // * (mod 30) 0...1 for layer 1, 2...3 for layer 2, ...;
    // * (mod 2) 0 for Z plane, 1 for phi plane;
    //*/
    //unsigned long int stripmaps[480] = {0};
    for (KLMCluster& klmcluster : track->getRelationsTo<KLMCluster>()) {

      // releation weight?
      nklmclusters += 1;

      for (KLMHit2d& klmhit : klmcluster.getRelationsTo<KLMHit2d>()) {
        nklmhits += 1;
        //const bklm::Module* bklmgeo = m_bklmGeoPar->findModule(klmhit.getSection(), klmhit.getSector(), klmhit.getLayer());
        ////std::cout << "has KLM hit" << std::endl;
        //CLHEP::Hep3Vector local  = bklmgeo->getLocalPosition(klmhit.getPhiStripMin(), klmhit.getZStripMin());
        //CLHEP::Hep3Vector global = bklmgeo->localToGlobal(local);
        Zmin = klmhit.getPositionZ() < Zmin ? klmhit.getPositionZ() : Zmin;
        Zmax = klmhit.getPositionZ() > Zmax ? klmhit.getPositionZ() : Zmax;
        zstripmin = klmhit.getZStripMin() < zstripmin ? klmhit.getZStripMin() : zstripmin;
        zstripmax = klmhit.getZStripMax() > zstripmax ? klmhit.getZStripMax() : zstripmax;
        //phistripmin = klmhit.getPhiStripMin();
        //phistripmax = klmhit.getPhiStripMax();
        ////std::cout << "Zstrip min = " << zstripmin << ", max = " << zstripmax << std::endl;
        //
        unsigned long int hit_Zstripmin = klmhit.getZStripMin();
        unsigned long int hit_Zstripmax = klmhit.getZStripMax();
        double  hit_Xposition = klmhit.getPositionX();
        double  hit_Yposition = klmhit.getPositionY();
        double  hit_Zposition = klmhit.getPositionZ();
        double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);
        double  hit_MinStripXposition = klmhit.getPositionXOfMinStrip();
        double  hit_MinStripYposition = klmhit.getPositionYOfMinStrip();
        double  hit_MinStripZposition = klmhit.getPositionZOfMinStrip();
        unsigned long int hit_Phistripmin = klmhit.getPhiStripMin();
        unsigned long int hit_Phistripmax = klmhit.getPhiStripMax();
        unsigned long int hit_layer = klmhit.getLayer();
        unsigned long int hit_sector = klmhit.getSector();
        unsigned long int hit_section = klmhit.getSection();

        //unsigned long int Zstripmap = 0;
        //unsigned long int Phistripmap = 0;
        //for (int ii = hit_Zstripmin; ii <= hit_Zstripmax; ii ++){ Zstripmap = Zstripmap | (1 << (ii - 1)); }
        //for (int ii = hit_Phistripmin; ii <= hit_Phistripmax; ii ++){ Phistripmap = Phistripmap | (1 << (ii - 1)); }

        //int Zstrip_position = hit_section * 240 + (hit_sector - 1) * 30 + (hit_layer - 1) * 2 + 0;
        //int Phistrip_position = hit_section * 240 + (hit_sector - 1) * 30 + (hit_layer - 1) * 2 + 1;

        //stripmaps[Zstrip_position] = stripmaps[Zstrip_position] | Zstripmap;
        //stripmaps[Phistrip_position] = stripmaps[Phistrip_position] | Phistripmap;

        /*
         * bit 1...6 for min Z strip, bit 7...12 for min phi strip, bit 13...18 for max Z strip, bit 19...24 for max phi strip
         * bit 25...29 for layer, bit 29...32 for sector, bit 33 for section
         */
        unsigned long int channelID = (hit_Zstripmin << 0) + (hit_Phistripmin << 6) + (hit_Zstripmax << 12) + (hit_Phistripmax << 18) +
                                      (hit_layer << 24) + (hit_sector << 29) + (hit_section << 32);
        part->writeExtraInfo("HitchannelID_" + std::to_string(nklmhits - 1), channelID);
        part->writeExtraInfo("HitXposition_" + std::to_string(nklmhits - 1), hit_Xposition);
        part->writeExtraInfo("HitYposition_" + std::to_string(nklmhits - 1), hit_Yposition);
        part->writeExtraInfo("HitZposition_" + std::to_string(nklmhits - 1), hit_Zposition);
        part->writeExtraInfo("HitRposition_" + std::to_string(nklmhits - 1), hit_Rposition);

        part->writeExtraInfo("HitMinStripXposition_" + std::to_string(nklmhits - 1), hit_MinStripXposition);
        part->writeExtraInfo("HitMinStripYposition_" + std::to_string(nklmhits - 1), hit_MinStripYposition);
        part->writeExtraInfo("HitMinStripZposition_" + std::to_string(nklmhits - 1), hit_MinStripZposition);


      }
    }
    //for (int ii=0; ii < 480; ii ++){
    //  part->writeExtraInfo("BKLM_hitmap_"+std::to_string(ii), stripmaps[ii]); // do not work. Force unsigned long int to double
    //}
    part->writeExtraInfo("Zstripmin", zstripmin);
    part->writeExtraInfo("Zstripmax", zstripmax);
    part->writeExtraInfo("Zpositionmin", Zmin);
    part->writeExtraInfo("Zpositionmax", Zmax);
    part->writeExtraInfo("nklmhits", nklmhits);
    part->writeExtraInfo("nklmclusters", nklmclusters);
    //part->writeExtraInfo("Phistripmax", phistripmax);
    //part->writeExtraInfo("Phistripmin", phistripmin);
    //std::cout << "Number of hits in this KLMCluster is " << nklmhits << std::endl;

    int nklmexthits = 0;
    //double lastHitRposition = 0;

    for (ExtHit& exthit : track->getRelationsTo<ExtHit>()) {

      if (exthit.getDetectorID() != Const::EDetector::BKLM) continue;

      ROOT::Math::XYZVector vector = exthit.getPosition(); // for release 8
      //TVector3 vector = exthit.getPosition(); // for release 7
      double  hit_Zposition = vector.Z();
      double  hit_Xposition = vector.X();
      double  hit_Yposition = vector.Y();
      double  hit_Rposition = sqrt(hit_Xposition * hit_Xposition + hit_Yposition * hit_Yposition);

      //if (hit_Rposition - lastHitRposition < 5) continue; // skip the too small step extrapolation

      //lastHitRposition = hit_Rposition;

      nklmexthits += 1;

      part->writeExtraInfo("ExtXposition_" + std::to_string(nklmexthits - 1), hit_Xposition);
      part->writeExtraInfo("ExtYposition_" + std::to_string(nklmexthits - 1), hit_Yposition);
      part->writeExtraInfo("ExtZposition_" + std::to_string(nklmexthits - 1), hit_Zposition);
      part->writeExtraInfo("ExtRposition_" + std::to_string(nklmexthits - 1), hit_Rposition);

    }
    part->writeExtraInfo("nklmexthits", nklmexthits);


    int nklmmuidhits = 0;

    for (KLMMuidHit& muidhit : track->getRelationsTo<KLMMuidHit>()) {

      nklmmuidhits  += 1;
      int muidhitlayer = muidhit.getLayer(); // use more reliable criteria in the future?
      double muidhitchi2 =  muidhit.getChiSquared();
      part->writeExtraInfo("Muidhitchi2_" + std::to_string(nklmmuidhits - 1), muidhitchi2);
      part->writeExtraInfo("Muidhitlayer_" + std::to_string(nklmmuidhits - 1), muidhitlayer);
    }
    part->writeExtraInfo("nklmmuidhits", nklmmuidhits);


    // B2INFO("Local calculated PDF : " <<  builder->getPDF(klmll));
    // B2INFO("Stored PDF : " << klmll->getPDFValue(13) );

    // B2INFO("LongitudinalPDF : " <<  builder->getLongitudinalPDF(klmll));
    // B2INFO("TransversePDF : " <<  builder->getTransversePDF(klmll));

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


