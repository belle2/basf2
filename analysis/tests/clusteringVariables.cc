/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe, Torben Ferber, Giacomo De Pietro           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Unit tests for all variables related to clustering and clustering
// subdetectors (KLM and ECL variables, track <--> cluster matching etc)

#include <gtest/gtest.h>
#include <TRandom3.h>

#include <analysis/ParticleCombiner/ParticleCombiner.h>

// VariableManager and particle(list)
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// mdst dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>

// framework - set up mock events
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Gearbox.h>

using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** ECL variable test fixture */
  class ECLVariableTest : public ::testing::Test {
  protected:
    /** register Particle and ECLCluster arrays. */
    void SetUp() override
    {
      // setup the DataStore
      DataStore::Instance().setInitializeActive(true);

      // particles (to be filled)
      StoreArray<Particle> particles;
      particles.registerInDataStore();

      // mock up mdst objects
      StoreArray<Track> tracks;
      tracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFits;
      trackFits.registerInDataStore();
      StoreArray<ECLCluster> eclclusters;
      eclclusters.registerInDataStore();

      // tracks can be matched to clusters
      tracks.registerRelationTo(eclclusters);

      // we're done setting up the datastore
      DataStore::Instance().setInitializeActive(false);

      // add some tracks the zeroth one is not going to be matched
      tracks.appendNew(Track());
      const Track* t1 = tracks.appendNew(Track());
      const Track* t2 = tracks.appendNew(Track());
      const Track* t3 = tracks.appendNew(Track());
      const Track* t4 = tracks.appendNew(Track());
      tracks.appendNew(Track());
      tracks.appendNew(Track());

      // mock up some TrackFits for them (all pions)
      TRandom3 generator;
      TMatrixDSym cov6(6);
      auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

      for (int i = 0; i < tracks.getEntries(); ++i) {
        int charge = (i % 2 == 0) ? +1 : -1;
        TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
        TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
        TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
        trackFits.appendNew(position, momentum, cov6, charge, Const::pion, 0.5, 1.5, CDCValue, 16777215, 0);
        tracks[i]->setTrackFitResultIndex(Const::pion, i);
      }

      // add some ECL clusters
      ECLCluster* e1 = eclclusters.appendNew(ECLCluster());
      e1->setEnergy(0.3);
      e1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e1->setClusterId(1);
      e1->setTime(1);
      e1->setDeltaTime99(0.1);
      e1->setConnectedRegionId(1);
      // leave this guy with default theta and phi
      ECLCluster* e2 = eclclusters.appendNew(ECLCluster());
      e2->setEnergy(0.6);
      e2->setTheta(1.0); // somewhere in the barrel
      e2->setPhi(2.0);
      e2->setR(148.5);
      e2->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e2->setClusterId(2);
      e2->setTime(2);
      e2->setDeltaTime99(0.2);
      e2->setConnectedRegionId(2);
      ECLCluster* e3 = eclclusters.appendNew(ECLCluster());
      e3->setEnergy(0.15);
      e3->setTheta(0.2); // somewhere in the fwd endcap
      e3->setPhi(1.5);
      e3->setR(200.0);
      e3->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e3->addHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron);
      // let's suppose this cluster could also be due to a neutral hadron. In
      // this case, the c_neutralHadron hypothesis bit would hopefully also have
      // been set by the reconstruction... arbitrarily choose cluster 3
      e3->setClusterId(3);
      e3->setTime(3);
      e3->setDeltaTime99(0.3);
      e3->setConnectedRegionId(1); // shares the connected region with cluster 1

      // aaand add clusters related to the tracks
      ECLCluster* e4 = eclclusters.appendNew(ECLCluster());
      e4->setEnergy(0.2);
      e4->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e4->setClusterId(4);
      t1->addRelationTo(e4);
      e4->setIsTrack(true);

      ECLCluster* e5 = eclclusters.appendNew(ECLCluster());
      e5->setEnergy(0.3);
      e5->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e5->setClusterId(5);
      t2->addRelationTo(e5);
      e5->setIsTrack(true);

      ECLCluster* e6 = eclclusters.appendNew(ECLCluster());
      e6->setEnergy(0.2);
      e6->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      e6->setClusterId(6);
      t3->addRelationTo(e6);
      t4->addRelationTo(e6);
      // two tracks are related to this cluster this can happen due to real
      // physics and we should be able to cope
      e6->setIsTrack(true);

    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(ECLVariableTest, b2bKinematicsTest)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;
    StoreArray<Track> tracks;

    // connect gearbox for CMS boosting etc
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // get the zeroth track in the array (is not associated to a cluster)
    const Particle* noclustertrack = particles.appendNew(Particle(tracks[0], Const::pion));

    // grab variables for testing
    const Manager::Var* b2bClusterTheta = Manager::Instance().getVariable("b2bClusterTheta");
    const Manager::Var* b2bClusterPhi = Manager::Instance().getVariable("b2bClusterPhi");

    EXPECT_EQ(gammalist->getListSize(), 3);

    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(0)), 3.0276606);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(0)), 0.0);
    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(1)), 1.6036042);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(1)), -1.0607308);
    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(2)), 2.7840068);
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(2)), -1.3155469);

    // track (or anything without a cluster) should be nan
    ASSERT_TRUE(std::isnan(b2bClusterTheta->function(noclustertrack)));
    ASSERT_TRUE(std::isnan(b2bClusterPhi->function(noclustertrack)));

    // the "normal" (not cluster based) variables should be the same for photons
    // (who have no track information)
    const Manager::Var* b2bTheta = Manager::Instance().getVariable("b2bTheta");
    const Manager::Var* b2bPhi = Manager::Instance().getVariable("b2bPhi");

    EXPECT_FLOAT_EQ(b2bClusterTheta->function(gammalist->getParticle(0)),
                    b2bTheta->function(gammalist->getParticle(0)));
    EXPECT_FLOAT_EQ(b2bClusterPhi->function(gammalist->getParticle(0)),
                    b2bPhi->function(gammalist->getParticle(0)));
  }

  TEST_F(ECLVariableTest, clusterKinematicsTest)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;
    StoreArray<Track> tracks;

    // connect gearbox for CMS boosting etc
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // grab variables for testing
    const Manager::Var* clusterPhi = Manager::Instance().getVariable("clusterPhi");
    const Manager::Var* clusterPhiCMS = Manager::Instance().getVariable("useCMSFrame(clusterPhi)");
    const Manager::Var* clusterTheta = Manager::Instance().getVariable("clusterTheta");
    const Manager::Var* clusterThetaCMS = Manager::Instance().getVariable("useCMSFrame(clusterTheta)");

    EXPECT_FLOAT_EQ(clusterPhi->function(gammalist->getParticle(1)), 2.0);
    EXPECT_FLOAT_EQ(clusterPhiCMS->function(gammalist->getParticle(1)), 2.042609);
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(1)), 1.0);
    EXPECT_FLOAT_EQ(clusterThetaCMS->function(gammalist->getParticle(1)), 1.2599005);

    // test cluster quantities directly (lab system only)
    EXPECT_FLOAT_EQ(clusterPhi->function(gammalist->getParticle(0)), eclclusters[0]->getPhi());
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(0)), eclclusters[0]->getTheta());
  }

  TEST_F(ECLVariableTest, HypothesisVariables)
  {
    // we need the particles and ECLClusters arrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    // register in the datastore
    StoreObjPtr<ParticleList> gammalist("gamma");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i)
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }

    // grab variables for testing
    const Manager::Var* vHasNPhotons = Manager::Instance().getVariable("clusterHasNPhotons");
    const Manager::Var* vHasNeutHadr = Manager::Instance().getVariable("clusterHasNeutralHadron");

    // check that the hypotheses are correctly propagated to the VM.
    for (size_t i = 0; i < gammalist->getListSize(); ++i) {
      EXPECT_FLOAT_EQ(vHasNPhotons->function(gammalist->getParticle(i)), 1.0);
      if (i == 2) { // third cluster arbitrarily chosen to test the behaviour of dual hypothesis clusters
        EXPECT_FLOAT_EQ(vHasNeutHadr->function(gammalist->getParticle(i)), 1.0);
      } else {
        EXPECT_FLOAT_EQ(vHasNeutHadr->function(gammalist->getParticle(i)), 0.0);
      }
    } // end loop over test list
  }

  TEST_F(ECLVariableTest, IsFromECL)
  {
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    const Manager::Var* vIsFromECL = Manager::Instance().getVariable("isFromECL");
    const Manager::Var* vIsFromKLM = Manager::Instance().getVariable("isFromKLM");
    const Manager::Var* vIsFromTrack = Manager::Instance().getVariable("isFromTrack");
    const Manager::Var* vIsFromV0 = Manager::Instance().getVariable("isFromV0");

    for (int i = 0; i < eclclusters.getEntries(); ++i)
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        EXPECT_TRUE(vIsFromECL->function(p));
        EXPECT_FALSE(vIsFromKLM->function(p));
        EXPECT_FALSE(vIsFromTrack->function(p));
        EXPECT_FALSE(vIsFromV0->function(p));
      }
  }

  TEST_F(ECLVariableTest, ECLThetaAndPhiId)
  {
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> clusters{};
    StoreArray<ECLCluster> eclclusters;
    // make a particle from cluster #1
    const Particle* p = particles.appendNew(Particle(eclclusters[0]));

    // get the variables to test
    const Manager::Var* clusterThetaID = Manager::Instance().getVariable("clusterThetaID");
    const Manager::Var* clusterPhiID = Manager::Instance().getVariable("clusterPhiID");

    {
      clusters[0]->setMaxECellId(1);
      EXPECT_FLOAT_EQ(clusterThetaID->function(p), 0);
      EXPECT_FLOAT_EQ(clusterPhiID->function(p), 0);
    }
    {
      clusters[0]->setMaxECellId(6903);
      EXPECT_FLOAT_EQ(clusterThetaID->function(p), 52);
      EXPECT_FLOAT_EQ(clusterPhiID->function(p), 134);
    }
    {
      clusters[0]->setMaxECellId(8457);
      EXPECT_FLOAT_EQ(clusterThetaID->function(p), 65);
      EXPECT_FLOAT_EQ(clusterPhiID->function(p), 8);
    }
  }


  TEST_F(ECLVariableTest, WholeEventClosure)
  {
    // we need the particles, tracks, and ECLClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<ECLCluster> eclclusters;

    // create a photon (clusters) and pion (tracks) lists
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    StoreObjPtr<ParticleList> pionslist("pi+:testPionAllList");
    StoreObjPtr<ParticleList> apionslist("pi-:testPionAllList");

    // register the lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    pionslist.registerInDataStore(DataStore::c_DontWriteOut);
    apionslist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());
    pionslist.create();
    pionslist->initialize(211, pionslist.getName());
    apionslist.create();
    apionslist->initialize(-211, apionslist.getName());
    apionslist->bindAntiParticleList(*(pionslist));

    // make the photons from clusters (and sum up the total ecl energy)
    double eclEnergy = 0.0;
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      eclEnergy += eclclusters[i]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }


    // make the pions from tracks
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Particle* p = particles.appendNew(Particle(tracks[i], Const::pion));
      pionslist->addParticle(p);
    }

    // grab variables
    const Manager::Var* vClusterE = Manager::Instance().getVariable("clusterE");
    const Manager::Var* vClNTrack = Manager::Instance().getVariable("nECLClusterTrackMatches");

    // calculate the total neutral energy from the particle list --> VM
    double totalNeutralClusterE = 0.0;
    for (size_t i = 0; i < gammalist->getListSize(); ++i)
      totalNeutralClusterE += vClusterE->function(gammalist->getParticle(i));

    // calculate the total track-matched cluster energy from the particle list --> VM
    double totalTrackClusterE = 0.0;
    for (size_t i = 0; i < pionslist->getListSize(); ++i) { // includes antiparticles
      double clusterE = vClusterE->function(pionslist->getParticle(i));
      double nOtherCl = vClNTrack->function(pionslist->getParticle(i));
      if (nOtherCl > 0)
        totalTrackClusterE += clusterE / nOtherCl;
    }

    EXPECT_FLOAT_EQ(totalNeutralClusterE + totalTrackClusterE, eclEnergy);
  }

  TEST_F(ECLVariableTest, eclClusterOnlyInvariantMass)
  {
    // declare all the array we need
    StoreArray<Particle> particles, particles_noclst;
    std::vector<int> daughterIndices, daughterIndices_noclst;

    //proxy initialize where to declare the needed array
    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclclusters_new;
    eclclusters_new.registerInDataStore();
    particles.registerRelationTo(eclclusters_new);
    DataStore::Instance().setInitializeActive(false);

    // create two Lorentz vectors
    const float px_0 = 2.;
    const float py_0 = 1.;
    const float pz_0 = 3.;
    const float px_1 = 1.5;
    const float py_1 = 1.5;
    const float pz_1 = 2.5;
    float E_0, E_1;
    E_0 = sqrt(pow(px_0, 2) + pow(py_0, 2) + pow(pz_0, 2));
    E_1 = sqrt(pow(px_1, 2) + pow(py_1, 2) + pow(pz_1, 2));
    TLorentzVector momentum;
    TLorentzVector dau0_4vec(px_0, py_0, pz_0, E_0), dau1_4vec(px_1, py_1, pz_1, E_1);

    // add the two photons as the two daughters of some particle and create the latter
    Particle dau0_noclst(dau0_4vec, 22);
    momentum += dau0_noclst.get4Vector();
    Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    daughterIndices_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    Particle dau1_noclst(dau1_4vec, 22);
    momentum += dau1_noclst.get4Vector();
    Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    daughterIndices_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    const Particle* par_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices_noclst);

    // grab variables
    const Manager::Var* var = Manager::Instance().getVariable("eclClusterOnlyInvariantMass");

    // when no relations are set between the particles and the eclClusters, nan is expected to be returned
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(par_noclst)));

    // set relations between particles and eclClusters
    ECLCluster* eclst0 = eclclusters_new.appendNew(ECLCluster());
    eclst0->setEnergy(dau0_4vec.E());
    eclst0->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst0->setClusterId(1);
    eclst0->setTheta(dau0_4vec.Theta());
    eclst0->setPhi(dau0_4vec.Phi());
    eclst0->setR(148.4);
    ECLCluster* eclst1 = eclclusters_new.appendNew(ECLCluster());
    eclst1->setEnergy(dau1_4vec.E());
    eclst1->setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    eclst1->setClusterId(2);
    eclst1->setTheta(dau1_4vec.Theta());
    eclst1->setPhi(dau1_4vec.Phi());
    eclst1->setR(148.5);

    // use these new-created clusters rather than the 6 default ones
    const Particle* newDaughter0 = particles.appendNew(Particle(eclclusters_new[6]));
    daughterIndices.push_back(newDaughter0->getArrayIndex());
    const Particle* newDaughter1 = particles.appendNew(Particle(eclclusters_new[7]));
    daughterIndices.push_back(newDaughter1->getArrayIndex());

    const Particle* par = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices);

    //now we expect non-nan results
    EXPECT_FLOAT_EQ(var->function(par), 0.73190731);
  }

  TEST_F(ECLVariableTest, averageECLTimeQuantities)
  {
    // we need the particles, and ECLClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;

    // create a photon (clusters) and a pi0 list
    StoreObjPtr<ParticleList> gammalist("gamma:testGammaAllList");
    StoreObjPtr<ParticleList> pionslist("pi0:testPizAllList");

    // register the lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    pionslist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());
    pionslist.create();
    pionslist->initialize(111, pionslist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // make the pi0s as combinations of photons
    ParticleGenerator combiner_pi0("pi0:testPizAllList -> gamma:testGammaAllList gamma:testGammaAllList");
    combiner_pi0.init();
    while (combiner_pi0.loadNext()) {
      const Particle& particle = combiner_pi0.getCurrentParticle();

      particles.appendNew(particle);
      int iparticle = particles.getEntries() - 1;

      pionslist->addParticle(iparticle, particle.getPDGCode(), particle.getFlavorType());
    }

    // grab variables
    const Manager::Var* weightedAverageECLTime = Manager::Instance().getVariable("weightedAverageECLTime");
    const Manager::Var* maxDist = Manager::Instance().getVariable("maxWeightedDistanceFromAverageECLTime");

    // particles without daughters should have NaN as weighted average
    EXPECT_TRUE(std::isnan(weightedAverageECLTime->function(gammalist->getParticle(0))));

    // check that weighted average of first pi0 is correct
    EXPECT_FLOAT_EQ(weightedAverageECLTime->function(pionslist->getParticle(0)), 1.2);

    // check that maximal difference to weighted average in units of uncertainty is calculated correctly
    EXPECT_FLOAT_EQ(maxDist->function(pionslist->getParticle(0)), 4.0);
  }

  TEST_F(ECLVariableTest, photonHasOverlap)
  {
    // declare StoreArrays of Particles and ECLClusters
    StoreArray<Particle> particles;
    StoreArray<ECLCluster> eclclusters;
    StoreArray<Track> tracks;

    // create a photon and the pion lists
    StoreObjPtr<ParticleList> gammalist("gamma:all");
    StoreObjPtr<ParticleList> pionlist("pi+:all");
    StoreObjPtr<ParticleList> piminuslist("pi-:all");

    // register the particle lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(DataStore::c_DontWriteOut);
    pionlist.registerInDataStore(DataStore::c_DontWriteOut);
    piminuslist.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the photon list
    gammalist.create();
    gammalist->initialize(22, gammalist.getName());

    // make the photons from clusters
    for (int i = 0; i < eclclusters.getEntries(); ++i) {
      if (!eclclusters[i]->isTrack()) {
        const Particle* p = particles.appendNew(Particle(eclclusters[i]));
        gammalist->addParticle(p);
      }
    }

    // initialize the pion lists
    pionlist.create();
    pionlist->initialize(211, pionlist.getName());
    piminuslist.create();
    piminuslist->initialize(-211, piminuslist.getName());
    piminuslist->bindAntiParticleList(*(pionlist));

    // fill the pion list with the tracks
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Particle* p = particles.appendNew(Particle(tracks[i], Const::pion));
      pionlist->addParticle(p);
    }

    // check overlap without any arguments
    const Manager::Var* photonHasOverlapNoArgs = Manager::Instance().getVariable("photonHasOverlap()");
    // the track list e-:all is missing so NaN is returned
    EXPECT_TRUE(std::isnan(photonHasOverlapNoArgs->function(particles[0])));

    // check overlap without any requirement on other photons
    const Manager::Var* photonHasOverlapAll = Manager::Instance().getVariable("photonHasOverlap(, gamma:all, pi+:all)");
    // cluster 3 and cluster 1 share the connected region
    EXPECT_TRUE(photonHasOverlapAll->function(particles[0]));
    // photonHasOverlap is designed for photons, so calling it for a pion returns NaN
    EXPECT_TRUE(std::isnan(photonHasOverlapAll->function(particles[3])));

    // check overlap with photons in barrel
    const Manager::Var* photonHasOverlapBarrel = Manager::Instance().getVariable("photonHasOverlap(clusterReg==2, gamma:all, pi+:all)");
    // cluster 3 is in the forward end cap so it doesn't matter that it has the same connected region like cluster 1
    EXPECT_FALSE(photonHasOverlapBarrel->function(particles[0]));
  }

  class KLMVariableTest : public ::testing::Test {
  protected:
    /** register Particle and KLMCluster arrays. */
    void SetUp() override
    {
      // setup the DataStore
      DataStore::Instance().setInitializeActive(true);

      // particles (to be filled)
      StoreArray<Particle> particles;
      particles.registerInDataStore();

      // mock up mdst objects
      StoreArray<Track> tracks;
      tracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFits;
      trackFits.registerInDataStore();
      StoreArray<KLMCluster> klmClusters;
      klmClusters.registerInDataStore();

      // tracks can be matched to clusters
      tracks.registerRelationTo(klmClusters);

      // we're done setting up the datastore
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(KLMVariableTest, WholeEventClosure)
  {
    // we need the Particles, Tracks, TrackFitResults and KLMClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFits;
    StoreArray<KLMCluster> klmClusters;

    // create a KLong (clusters) and muon (tracks) lists
    StoreObjPtr<ParticleList> kLongList("K0_L:testKLong");
    StoreObjPtr<ParticleList> muonsList("mu-:testMuons");
    StoreObjPtr<ParticleList> amuonsList("mu+:testMuons");

    // register the lists in the datastore
    DataStore::Instance().setInitializeActive(true);
    kLongList.registerInDataStore(DataStore::c_DontWriteOut);
    muonsList.registerInDataStore(DataStore::c_DontWriteOut);
    amuonsList.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    // initialise the lists
    kLongList.create();
    kLongList->initialize(Const::Klong.getPDGCode(), kLongList.getName());
    muonsList.create();
    muonsList->initialize(Const::muon.getPDGCode(), muonsList.getName());
    amuonsList.create();
    amuonsList->initialize(-Const::muon.getPDGCode(), amuonsList.getName());
    amuonsList->bindAntiParticleList(*(muonsList));

    // add some tracks
    const Track* t1 = tracks.appendNew(Track());
    const Track* t2 = tracks.appendNew(Track());
    const Track* t3 = tracks.appendNew(Track());
    tracks.appendNew(Track());
    tracks.appendNew(Track());

    // mock up some TrackFits for them (all muons)
    TRandom3 generator;
    TMatrixDSym cov6(6);
    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    for (int i = 0; i < tracks.getEntries(); ++i) {
      int charge = (i % 2 == 0) ? +1 : -1;
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
      trackFits.appendNew(position, momentum, cov6, charge, Const::muon, 0.5, 1.5, CDCValue, 16777215, 0);
      tracks[i]->setTrackFitResultIndex(Const::muon, i);
    }

    // add some clusters
    KLMCluster* klm1 = klmClusters.appendNew(KLMCluster());
    klm1->setTime(1.1);
    klm1->setClusterPosition(1.1, 1.1, 1.0);
    klm1->setLayers(1);
    klm1->setInnermostLayer(1);
    klm1->setMomentumMag(1.0);
    KLMCluster* klm2 = klmClusters.appendNew(KLMCluster());
    klm2->setTime(1.2);
    klm2->setClusterPosition(1.2, 1.2, 2.0);
    klm2->setLayers(2);
    klm2->setInnermostLayer(2);
    klm2->setMomentumMag(1.0);
    KLMCluster* klm3 = klmClusters.appendNew(KLMCluster());
    klm3->setTime(1.3);
    klm3->setClusterPosition(1.3, 1.3, 3.0);
    klm3->setLayers(3);
    klm3->setInnermostLayer(3);
    klm3->setMomentumMag(1.0);

    // and add clusters related to the tracks
    // case 1: 1 track --> 1 cluster
    KLMCluster* klm4 = klmClusters.appendNew(KLMCluster());
    klm4->setTime(1.4);
    klm4->setClusterPosition(-1.4, -1.4, 1.0);
    klm4->setLayers(4);
    klm4->setInnermostLayer(4);
    klm4->setMomentumMag(1.0);
    t1->addRelationTo(klm4);

    // case 2: 2 tracks --> 1 cluster
    KLMCluster* klm5 = klmClusters.appendNew(KLMCluster());
    klm5->setTime(1.5);
    klm5->setClusterPosition(-1.5, -1.5, 1.0);
    klm5->setLayers(5);
    klm5->setInnermostLayer(5);
    klm5->setMomentumMag(1.0);
    t2->addRelationTo(klm5);
    t3->addRelationTo(klm5);

    // case 3: 1 track --> 2 clusters
    // not possible

    // make the KLong from clusters (and sum up the total KLM momentum magnitude)
    double klmMomentum = 0.0;
    for (int i = 0; i < klmClusters.getEntries(); ++i) {
      klmMomentum += klmClusters[i]->getMomentumMag();
      if (!klmClusters[i]->getAssociatedTrackFlag()) {
        const Particle* p = particles.appendNew(Particle(klmClusters[i]));
        kLongList->addParticle(p);
      }
    }

    // make the muons from tracks
    for (int i = 0; i < tracks.getEntries(); ++i) {
      const Particle* p = particles.appendNew(Particle(tracks[i], Const::muon));
      muonsList->addParticle(p);
    }

    // grab variables
    const Manager::Var* vClusterP = Manager::Instance().getVariable("klmClusterMomentum");
    const Manager::Var* vClNTrack = Manager::Instance().getVariable("nKLMClusterTrackMatches");

    // calculate the total KLM momentum from the KLong list --> VM
    double totalKLongMomentum = 0.0;
    for (size_t i = 0; i < kLongList->getListSize(); ++i)
      totalKLongMomentum += vClusterP->function(kLongList->getParticle(i));

    // calculate the total KLM momentum from muon-matched list --> VM
    double totalMuonMomentum = 0.0;
    for (size_t i = 0; i < muonsList->getListSize(); ++i) { // includes antiparticles
      double muonMomentum = vClusterP->function(muonsList->getParticle(i));
      double nOtherCl = vClNTrack->function(muonsList->getParticle(i));
      if (nOtherCl > 0)
        totalMuonMomentum += muonMomentum / nOtherCl;
    }

    EXPECT_FLOAT_EQ(5.0, klmMomentum);
    EXPECT_FLOAT_EQ(totalKLongMomentum + totalMuonMomentum, klmMomentum);
  }

  TEST_F(KLMVariableTest, TrackToKLMClusterMatchingTest)
  {
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFits;
    StoreArray<KLMCluster> klmClusters;

    // add a TrackFitResult
    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);
    TMatrixDSym cov6(6);
    const int charge = 1;
    const float pValue = 0.5;
    const float bField = 1.5;
    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);
    trackFits.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215, 0);

    // add one Track
    Track myTrack;
    myTrack.setTrackFitResultIndex(Const::muon, 0);
    Track* muonTrack = tracks.appendNew(myTrack);

    // add two KLMClusters
    KLMCluster* klm1 = klmClusters.appendNew(KLMCluster());
    klm1->setTime(1.1);
    klm1->setClusterPosition(1.1, 1.1, 1.0);
    klm1->setLayers(1);
    klm1->setInnermostLayer(1);
    klm1->setMomentumMag(1.0);
    KLMCluster* klm2 = klmClusters.appendNew(KLMCluster());
    klm2->setTime(1.2);
    klm2->setClusterPosition(1.2, 1.2, 2.0);
    klm2->setLayers(2);
    klm2->setInnermostLayer(2);
    klm2->setMomentumMag(1.0);

    // and add a weighted relationship between the track and both clusters
    // only the relation with klm1 must be returned
    // in reconstruction we set a relation to only one cluster (if any),
    // so here we test that getKLMCluster() returns the first cluster
    // stored in the RelationVector
    float distance1 = 11.1;
    muonTrack->addRelationTo(klm1, 1. / distance1);
    float distance2 = 2.2;
    muonTrack->addRelationTo(klm2, 1. / distance2);

    // add a Particle
    const Particle* muon = particles.appendNew(Particle(muonTrack, Const::muon));

    // grab variables
    const Manager::Var* vTrNClusters = Manager::Instance().getVariable("nMatchedKLMClusters");
    const Manager::Var* vClusterInnermostLayer = Manager::Instance().getVariable("klmClusterInnermostLayer");
    const Manager::Var* vClusterTrackDistance = Manager::Instance().getVariable("klmClusterTrackDistance");

    EXPECT_POSITIVE(vTrNClusters->function(muon));
    EXPECT_FLOAT_EQ(1.0, vClusterInnermostLayer->function(muon));
    EXPECT_FLOAT_EQ(distance1, vClusterTrackDistance->function(muon));

    // add a Pion - no clusters matched here
    trackFits.appendNew(position, momentum, cov6, charge, Const::pion, pValue, bField, CDCValue, 16777215, 0);
    Track mySecondTrack;
    mySecondTrack.setTrackFitResultIndex(Const::pion, 0);
    Track* pionTrack = tracks.appendNew(mySecondTrack);
    const Particle* pion = particles.appendNew(Particle(pionTrack, Const::pion));

    EXPECT_FLOAT_EQ(0.0, vTrNClusters->function(pion));
  }
}
