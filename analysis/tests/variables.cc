
#include <analysis/variables/Variables.h>
#include <analysis/variables/EventVariables.h>
#include <analysis/variables/FlightInfoVariables.h>
#include <analysis/variables/VertexVariables.h>
#include <analysis/variables/PIDVariables.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/ROEVariables.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <gtest/gtest.h>

#include <TMatrixFSym.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  /** test kinematic Variable. */
  TEST(KinematicVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    {
      Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);

      TMatrixFSym error(7);
      error.Zero();
      error(0, 0) = 0.05;
      error(1, 1) = 0.2;
      error(2, 2) = 0.4;
      error(0, 1) = -0.1;
      error(0, 2) = 0.9;
      p.setMomentumVertexErrorMatrix(error);

      EXPECT_FLOAT_EQ(0.9, particleP(&p));
      EXPECT_FLOAT_EQ(1.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.1, particlePx(&p));
      EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
      EXPECT_FLOAT_EQ(0.8, particlePz(&p));
      EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
      EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

      EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
      EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
      EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
      EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));


      {
        UseReferenceFrame<CMSFrame> dummy;
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.058562335, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54898131, particlePz(&p));
        EXPECT_FLOAT_EQ(0.40426421, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80522972, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4254233, particlePhi(&p));

        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      }

      {
        UseReferenceFrame<RestFrame> dummy(&p);
        EXPECT_ALL_NEAR(particleP(&p), 0.0, 1e-9);
        EXPECT_FLOAT_EQ(0.4358899, particleE(&p));
        EXPECT_ALL_NEAR(0.0, particlePx(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePy(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePz(&p), 1e-9);
        EXPECT_ALL_NEAR(0.0, particlePt(&p), 1e-9);

      }

      {
        UseReferenceFrame<LabFrame> dummy;
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
        EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
        EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
        EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));
      }

      {
        UseReferenceFrame<RotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 1, 0), TVector3(0, 0, 1));
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePzErr(&p));
        EXPECT_FLOAT_EQ(0.488093530, particlePtErr(&p));
        EXPECT_FLOAT_EQ(0.156402664, particleCosThetaErr(&p));
        EXPECT_FLOAT_EQ(0.263066820, particlePhiErr(&p));

        const auto& frame = ReferenceFrame::GetCurrent();
        EXPECT_FLOAT_EQ(-0.1, frame.getMomentumErrorMatrix(&p)(0, 1));
        EXPECT_FLOAT_EQ(0.9, frame.getMomentumErrorMatrix(&p)(0, 2));
      }

      {
        UseReferenceFrame<RotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 0, -1), TVector3(0, 1, 0));
        EXPECT_FLOAT_EQ(0.9, particleP(&p));
        EXPECT_FLOAT_EQ(1.0, particleE(&p));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.8, particlePy(&p));
        EXPECT_FLOAT_EQ(-0.4, particlePz(&p));

        EXPECT_FLOAT_EQ(0.737446378, particlePErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.05), particlePxErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.4), particlePyErr(&p));
        EXPECT_FLOAT_EQ(sqrt(0.2), particlePzErr(&p));

        const auto& frame = ReferenceFrame::GetCurrent();
        EXPECT_FLOAT_EQ(-0.9, frame.getMomentumErrorMatrix(&p)(0, 1));
        EXPECT_FLOAT_EQ(-0.1, frame.getMomentumErrorMatrix(&p)(0, 2));
      }

      {
        UseReferenceFrame<CMSRotationFrame> dummy(TVector3(1, 0, 0), TVector3(0, 1, 0), TVector3(0, 0, 1));
        EXPECT_FLOAT_EQ(0.68176979, particleP(&p));
        EXPECT_FLOAT_EQ(0.80920333, particleE(&p));
        EXPECT_FLOAT_EQ(0.058562335, particlePx(&p));
        EXPECT_FLOAT_EQ(-0.40000001, particlePy(&p));
        EXPECT_FLOAT_EQ(0.54898131, particlePz(&p));
        EXPECT_FLOAT_EQ(0.40426421, particlePt(&p));
        EXPECT_FLOAT_EQ(0.80522972, particleCosTheta(&p));
        EXPECT_FLOAT_EQ(-1.4254233, particlePhi(&p));

        EXPECT_FLOAT_EQ(sqrt(0.2), particlePyErr(&p));
      }

      {
        Particle pinv({ -0.1 , 0.4, -0.8, 1.0 }, 11);
        UseReferenceFrame<RestFrame> dummy(&pinv);
        Particle p2({ 0.0 , 0.0, 0.0, 0.4358899}, 11);
        EXPECT_FLOAT_EQ(0.9, particleP(&p2));
        EXPECT_FLOAT_EQ(1.0, particleE(&p2));
        EXPECT_FLOAT_EQ(0.1, particlePx(&p2));
        EXPECT_FLOAT_EQ(-0.4, particlePy(&p2));
        EXPECT_FLOAT_EQ(0.8, particlePz(&p2));
        EXPECT_FLOAT_EQ(0.412310562, particlePt(&p2));
        EXPECT_FLOAT_EQ(0.8 / 0.9, particleCosTheta(&p2));
        EXPECT_FLOAT_EQ(-1.325817664, particlePhi(&p2));
      }
    }

    {
      Particle p({ 0.0 , 0.0, 0.0, 0.0 }, 11);
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));

      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(0.0, particleP(&p));
      EXPECT_FLOAT_EQ(0.0, particleE(&p));
      EXPECT_FLOAT_EQ(0.0, particlePx(&p));
      EXPECT_FLOAT_EQ(0.0, particlePy(&p));
      EXPECT_FLOAT_EQ(0.0, particlePz(&p));
      EXPECT_FLOAT_EQ(0.0, particlePt(&p));
      EXPECT_FLOAT_EQ(1.0, particleCosTheta(&p));
      EXPECT_FLOAT_EQ(0.0, particlePhi(&p));
    }

    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle> particles;
      particles.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
      PCmsLabTransform T;
      TLorentzVector vec0 = {0.0, 0.0, 0.0, T.getCMSEnergy()};
      TLorentzVector vec1 = {0.0, +0.332174566, 0.0, T.getCMSEnergy() / 2.};
      TLorentzVector vec2 = {0.0, -0.332174566, 0.0, T.getCMSEnergy() / 2.};
      Particle* p0 = particles.appendNew(Particle(T.rotateCmsToLab() * vec0, 22));
      Particle* p1 = particles.appendNew(Particle(T.rotateCmsToLab() * vec1, 22, Particle::c_Unflavored, Particle::c_Undefined, 1));
      Particle* p2 = particles.appendNew(Particle(T.rotateCmsToLab() * vec2, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));

      p0->appendDaughter(p1->getArrayIndex());
      p0->appendDaughter(p2->getArrayIndex());

      EXPECT_ALL_NEAR(m2RecoilSignalSide(p0), 0.0, 1e-7);
    }


  }


  TEST(VertexVariableTest, Variable)
  {

    // Connect gearbox for CMS variables

    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setPValue(0.5);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    EXPECT_FLOAT_EQ(1.0, particleDX(&p));
    EXPECT_FLOAT_EQ(2.0, particleDY(&p));
    EXPECT_FLOAT_EQ(2.0, particleDZ(&p));
    EXPECT_FLOAT_EQ(std::sqrt(5.0), particleDRho(&p));
    EXPECT_FLOAT_EQ(3.0, particleDistance(&p));
    EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));

    {
      UseReferenceFrame<CMSFrame> dummy;
      EXPECT_FLOAT_EQ(1.0261739, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(2.256825, particleDZ(&p));
      EXPECT_FLOAT_EQ(std::sqrt(2.0 * 2.0 + 1.0261739 * 1.0261739), particleDRho(&p));
      EXPECT_FLOAT_EQ(3.1853244, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    {
      Particle p2({ 0.1 , -0.4, 0.8, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 2.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(0.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(0.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(0.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }

    /* Test with a distance between mother and daughter vertex. One
     * has to calculate the result by hand to test the code....

    {
      Particle p2({ 0.0 , 1.0, 0.0, 1.0 }, 11);
      p2.setPValue(0.5);
      p2.setVertex(TVector3(1.0, 0.0, 2.0));

      UseReferenceFrame<RestFrame> dummy(&p2);
      EXPECT_FLOAT_EQ(0.0, particleDX(&p));
      EXPECT_FLOAT_EQ(2.0, particleDY(&p));
      EXPECT_FLOAT_EQ(0.0, particleDZ(&p));
      EXPECT_FLOAT_EQ(2.0, particleDRho(&p));
      EXPECT_FLOAT_EQ(2.0, particleDistance(&p));
      EXPECT_FLOAT_EQ(0.5, particlePvalue(&p));
    }
         */

  }

  TEST(TrackVariablesTest, Variable)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> myResults;
    StoreArray<Track> myTracks;
    StoreArray<Particle> myParticles;
    myResults.registerInDataStore();
    myTracks.registerInDataStore();
    myParticles.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    TRandom3 generator;

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());

    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    myResults.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* savedTrack = myTracks.appendNew(mytrack);

    Particle* part = myParticles.appendNew(savedTrack, Const::ChargedStable(11));

    EXPECT_FLOAT_EQ(0.5, trackPValue(part));
    EXPECT_FLOAT_EQ(position.Z(), trackZ0(part));
    EXPECT_FLOAT_EQ(sqrt(pow(position.X(), 2) + pow(position.Y(), 2)), trackD0(part));
    EXPECT_FLOAT_EQ(3, trackNCDCHits(part));
    EXPECT_FLOAT_EQ(24, trackNSVDHits(part));
    EXPECT_FLOAT_EQ(12, trackNPXDHits(part));

  }

  class ROEVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {

      StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
      DataStore::Instance().setInitializeActive(true);
      pi0ParticleList.registerInDataStore(DataStore::c_DontWriteOut);
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<KLMCluster> myKLMClusters;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<RestOfEvent> myROEs;
      StoreArray<PIDLikelihood> myPIDLikelihoods;
      myECLClusters.registerInDataStore();
      myKLMClusters.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      myROEs.registerInDataStore();
      myPIDLikelihoods.registerInDataStore();
      myParticles.registerRelationTo(myROEs);
      myTracks.registerRelationTo(myPIDLikelihoods);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };
//
// TODO: redo all ROE variable tests
//

  TEST_F(ROEVariablesTest, Variable)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);
    StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
    StoreArray<ECLCluster> myECLClusters;
    StoreArray<KLMCluster> myKLMClusters;
    StoreArray<TrackFitResult> myTFRs;
    StoreArray<Track> myTracks;
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    StoreArray<PIDLikelihood> myPIDLikelihoods;

    pi0ParticleList.create();
    pi0ParticleList->initialize(111, "pi0:vartest");

    // Neutral ECLCluster on reconstructed side
    ECLCluster myECL;
    myECL.setIsTrack(false);
    float eclREC = 0.5;
    myECL.setEnergy(eclREC);
    myECL.setHypothesisId(5);
    ECLCluster* savedECL = myECLClusters.appendNew(myECL);

    // Particle on reconstructed side from ECLCluster
    Particle p(savedECL);
    Particle* part = myParticles.appendNew(p);

    // Create ECLCluster on ROE side
    ECLCluster myROEECL;
    myROEECL.setIsTrack(false);
    float eclROE = 1.0;
    myROEECL.setEnergy(eclROE);
    myROEECL.setHypothesisId(5);
    ECLCluster* savedROEECL = myECLClusters.appendNew(myROEECL);
    Particle* roeECLParticle = myParticles.appendNew(savedROEECL);
    // Create KLMCluster on ROE side
    KLMCluster myROEKLM;
    KLMCluster* savedROEKLM = myKLMClusters.appendNew(myROEKLM);
    Particle* roeKLMParticle = myParticles.appendNew(savedROEKLM);

    // Create Track on ROE side
    // - create TFR

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);

    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    myTFRs.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215);

    // - create Track
    Track myROETrack;
    myROETrack.setTrackFitResultIndex(Const::muon, 0);
    Track* savedROETrack = myTracks.appendNew(myROETrack);
    // - create PID information, add relation
    PIDLikelihood myPID;
    myPID.setLogLikelihood(Const::TOP, Const::muon, 0.15);
    myPID.setLogLikelihood(Const::ARICH, Const::muon, 0.152);
    myPID.setLogLikelihood(Const::ECL, Const::muon, 0.154);
    myPID.setLogLikelihood(Const::CDC, Const::muon, 0.156);
    myPID.setLogLikelihood(Const::SVD, Const::muon, 0.158);
    myPID.setLogLikelihood(Const::TOP, Const::pion, 0.5);
    myPID.setLogLikelihood(Const::ARICH, Const::pion, 0.52);
    myPID.setLogLikelihood(Const::ECL, Const::pion, 0.54);
    myPID.setLogLikelihood(Const::CDC, Const::pion, 0.56);
    myPID.setLogLikelihood(Const::SVD, Const::pion, 0.58);
    PIDLikelihood* savedPID = myPIDLikelihoods.appendNew(myPID);

    savedROETrack->addRelationTo(savedPID);
    Particle* roeTrackParticle = myParticles.appendNew(savedROETrack, Const::muon);

    // Create ROE object, append tracks, clusters, add relation to particle
    //TODO: make particles
    RestOfEvent roe;
    vector<const Particle*> roeParticlesToAdd;
    roeParticlesToAdd.push_back(roeTrackParticle);
    roeParticlesToAdd.push_back(roeECLParticle);
    roeParticlesToAdd.push_back(roeKLMParticle);
    roe.addParticles(roeParticlesToAdd);
    RestOfEvent* savedROE = myROEs.appendNew(roe);
    /*
    std::map<std::string, std::map<unsigned int, bool>> tMasks;
    std::map<std::string, std::map<unsigned int, bool>> cMasks;
    std::map<std::string, std::vector<double>> fracs;

    std::map<unsigned int, bool> tMask1;
    std::map<unsigned int, bool> tMask2;
    tMask1[savedROETrack->getArrayIndex()] = true;
    tMask2[savedROETrack->getArrayIndex()] = false;

    std::map<unsigned int, bool> cMask1;
    std::map<unsigned int, bool> cMask2;
    cMask1[savedROEECL->getArrayIndex()] = true;
    cMask2[savedROEECL->getArrayIndex()] = false;

    std::vector<double> frac1 = {0, 0, 1, 0, 0, 0};
    std::vector<double> frac2 = {1, 1, 1, 1, 1, 1};

    tMasks["mask1"] = tMask1;
    tMasks["mask2"] = tMask2;

    cMasks["mask1"] = cMask1;
    cMasks["mask2"] = cMask2;

    fracs["mask1"] = frac1;
    fracs["mask2"] = frac2;

    savedROE->appendTrackMasks(tMasks);
    savedROE->appendECLClusterMasks(cMasks);
    savedROE->appendChargedStableFractionsSet(fracs);
    */
    savedROE->initializeMask("mask1", "test");
    std::shared_ptr<Variable::Cut> trackSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    std::shared_ptr<Variable::Cut> eclSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    savedROE->updateMaskWithCuts("mask1");
    savedROE->initializeMask("mask2", "test");
    savedROE->updateMaskWithCuts("mask2",  trackSelection,  eclSelection);
    part->addRelationTo(savedROE);

    // ROE variables
    PCmsLabTransform T;
    float E0 = T.getCMSEnergy() / 2;
    B2INFO("E0 is " << E0);
    //*/
    TLorentzVector pTrack_ROE_Lab(momentum, TMath::Sqrt(Const::muon.getMass()*Const::muon.getMass() + 1.0 /*momentum.Mag2()*/));
    pTrack_ROE_Lab = roeTrackParticle->get4Vector();
    TLorentzVector pECL_ROE_Lab(0, 0, eclROE, eclROE);
    TLorentzVector pECL_REC_Lab(0, 0, eclREC, eclREC);

    TLorentzVector rec4vec;
    rec4vec.SetE(pECL_REC_Lab.E());
    rec4vec.SetVect(pECL_REC_Lab.Vect());

    TLorentzVector roe4vec;
    roe4vec.SetE(pTrack_ROE_Lab.E() + pECL_ROE_Lab.E());
    roe4vec.SetVect(pTrack_ROE_Lab.Vect() + pECL_ROE_Lab.Vect());

    TLorentzVector rec4vecCMS = T.rotateLabToCms() * rec4vec;
    TLorentzVector roe4vecCMS = T.rotateLabToCms() * roe4vec;

    TVector3 pB = - roe4vecCMS.Vect();
    pB.SetMag(0.340);

    TLorentzVector m4v0;
    m4v0.SetE(2 * E0 - (rec4vecCMS.E() + roe4vecCMS.E()));
    m4v0.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v1;
    m4v1.SetE(E0 - rec4vecCMS.E());
    m4v1.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v2;
    m4v2.SetE(E0 - rec4vecCMS.E());
    m4v2.SetVect(- rec4vecCMS.Vect());

    TLorentzVector m4v3;
    m4v3.SetE(E0 - rec4vecCMS.E());
    m4v3.SetVect(pB - rec4vecCMS.Vect());

    TLorentzVector neutrino4vecCMS;
    neutrino4vecCMS.SetVect(- (roe4vecCMS.Vect() + rec4vecCMS.Vect()));
    neutrino4vecCMS.SetE(neutrino4vecCMS.Vect().Mag());

    TLorentzVector corrRec4vecCMS = rec4vecCMS + neutrino4vecCMS;
    B2INFO("roe4vecCMS.E() = " << roe4vecCMS.E());
    // TESTS FOR ROE STRUCTURE
    //EXPECT_B2FATAL(savedROE->getTrackMask("noSuchMask"));
    //EXPECT_B2FATAL(savedROE->getECLClusterMask("noSuchMask"));
    //double fArray[6];
    //EXPECT_B2FATAL(savedROE->fillFractions(fArray, "noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithCuts("noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithV0("noSuchMask", part));
    EXPECT_B2FATAL(savedROE->hasParticle(part, "noSuchMask"));

    // TESTS FOR ROE VARIABLES

    const Manager::Var* var = Manager::Instance().getVariable("nROE_Charged(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 13)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 211)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralHadrons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ParticlesInList(pi0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("ROE_charge(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("ROE_charge(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("ROE_eextra(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), savedROEECL->getEnergy());

    var = Manager::Instance().getVariable("ROE_eextra(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("ROE_deltae(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("ROE_deltae(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -E0);

    var = Manager::Instance().getVariable("ROE_mbc(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - roe4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("ROE_mbc(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("WE_deltae(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), corrRec4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("WE_deltae(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), rec4vecCMS.E() + rec4vecCMS.Vect().Mag() - E0);

    var = Manager::Instance().getVariable("WE_mbc(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - corrRec4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("WE_mbc(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("WE_MissM2(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), m4v0.Mag2());

    var = Manager::Instance().getVariable("WE_MissM2(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), (2 * E0 - rec4vecCMS.E()) * (2 * E0 - rec4vecCMS.E()) - rec4vecCMS.Vect().Mag2());

  }


  class EventVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(EventVariableTest, ExperimentRunEventDateAndTime)
  {
    const Manager::Var* exp = Manager::Instance().getVariable("expNum");
    const Manager::Var* run = Manager::Instance().getVariable("runNum");
    const Manager::Var* evt = Manager::Instance().getVariable("evtNum");
    const Manager::Var* date = Manager::Instance().getVariable("date");
    const Manager::Var* year = Manager::Instance().getVariable("year");
    const Manager::Var* time = Manager::Instance().getVariable("eventTimeSeconds");

    // there is no EventMetaData so expect nan
    EXPECT_FALSE(date->function(NULL) == date->function(NULL));
    EXPECT_FALSE(year->function(NULL) == year->function(NULL));
    EXPECT_FALSE(time->function(NULL) == time->function(NULL));

    DataStore::Instance().setInitializeActive(true);
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    evtMetaData.create();
    evtMetaData->setExperiment(1337);
    evtMetaData->setRun(12345);
    evtMetaData->setEvent(54321);
    evtMetaData->setTime(1288569600e9);
    // 01/11/2010 is the date TDR was uploaded to arXiv ... experiment's birthday?


    // -
    EXPECT_FLOAT_EQ(exp->function(NULL), 1337.);
    EXPECT_FLOAT_EQ(run->function(NULL), 12345.);
    EXPECT_FLOAT_EQ(evt->function(NULL), 54321.);
    EXPECT_FLOAT_EQ(date->function(NULL), 20101101.);
    EXPECT_FLOAT_EQ(year->function(NULL), 2010.);
    EXPECT_FLOAT_EQ(time->function(NULL), 1288569600);
  }

  TEST_F(EventVariableTest, TestGlobalCounters)
  {
    StoreArray<MCParticle> mcParticles; // empty
    const Manager::Var* var = Manager::Instance().getVariable("nMCParticles");
    EXPECT_FLOAT_EQ(var->function(NULL), 0.0);

    for (unsigned i = 0; i < 10; ++i)
      mcParticles.appendNew();

    EXPECT_FLOAT_EQ(var->function(NULL), 10.0);

    // TODO: add other counters nTracks etc in here
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForContinuumEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(mcParticle);

    const Manager::Var* var = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 1.0);
    EXPECT_FLOAT_EQ(var->function(p2), 1.0);
    const Manager::Var* varN = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(varN, nullptr);
    EXPECT_FLOAT_EQ(varN->function(p1), 0.0);
    EXPECT_FLOAT_EQ(varN->function(p2), 0.0);
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForUpsilon4SEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles2;
    StoreArray<Particle> particles2;
    particles2.registerRelationTo(mcParticles2);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles2.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p3 = particles2.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(mcParticle);

    mcParticle = mcParticles2.appendNew();
    mcParticle->setPDG(300553);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p4 = particles2.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 300553);
    p4->addRelationTo(mcParticle);

    const Manager::Var* var2 = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var2, nullptr);
    EXPECT_FLOAT_EQ(var2->function(p3), 0.0);
    EXPECT_FLOAT_EQ(var2->function(p4), 0.0);
    const Manager::Var* var2N = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(var2N, nullptr);
    EXPECT_FLOAT_EQ(var2N->function(p3), 1.0);
    EXPECT_FLOAT_EQ(var2N->function(p4), 1.0);
  }

  TEST_F(EventVariableTest, TestIfContinuumEvent_ForWrongReconstructedUpsilon4SEvent)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles3;
    StoreArray<Particle> particles3;
    particles3.registerRelationTo(mcParticles3);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles3.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p5 = particles3.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p5->addRelationTo(mcParticle);

    mcParticle = mcParticles3.appendNew();
    mcParticle->setPDG(300553);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p6 = particles3.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 15);
    p6->addRelationTo(mcParticle);

    const Manager::Var* var3 = Manager::Instance().getVariable("isContinuumEvent");
    ASSERT_NE(var3, nullptr);
    EXPECT_FLOAT_EQ(var3->function(p5), 0.0);
    EXPECT_FLOAT_EQ(var3->function(p6), 0.0);
    const Manager::Var* var3N = Manager::Instance().getVariable("isNotContinuumEvent");
    ASSERT_NE(var3N, nullptr);
    EXPECT_FLOAT_EQ(var3N->function(p5), 1.0);
    EXPECT_FLOAT_EQ(var3N->function(p6), 1.0);
  }


  class MetaVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      StoreObjPtr<EventExtraInfo>().registerInDataStore();
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(MetaVariableTest, countDaughters)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 0, 0, 3.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("countDaughters(charge > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 3.0);

    var = Manager::Instance().getVariable("countDaughters(abs(charge) > 0)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 6.0);

  }

  TEST_F(MetaVariableTest, useRestFrame)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useRestFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-9);

    var = Manager::Instance().getVariable("useRestFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.4358899);

    var = Manager::Instance().getVariable("useRestFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.0);
  }

  TEST_F(MetaVariableTest, useLabFrame)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useLabFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("useLabFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("useLabFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);
  }

  TEST_F(MetaVariableTest, useCMSFrame)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.setVertex(TVector3(1.0, 2.0, 2.0));

    const Manager::Var* var = Manager::Instance().getVariable("p");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.9);

    var = Manager::Instance().getVariable("E");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("distance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.0);

    var = Manager::Instance().getVariable("useCMSFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.68176979);

    var = Manager::Instance().getVariable("useCMSFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.80920333);

    var = Manager::Instance().getVariable("useCMSFrame(distance)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.1853244);
  }

  TEST_F(MetaVariableTest, extraInfo)
  {
    Particle p({ 0.1 , -0.4, 0.8, 1.0 }, 11);
    p.addExtraInfo("pi", 3.14);

    const Manager::Var* var = Manager::Instance().getVariable("extraInfo(pi)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 3.14);

    // If nullptr is given event extra info should be returned
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();
    eventExtraInfo->addExtraInfo("pi", 3.15);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.15);
  }

  TEST_F(MetaVariableTest, eventExtraInfo)
  {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    if (not eventExtraInfo.isValid())
      eventExtraInfo.create();
    eventExtraInfo->addExtraInfo("pi", 3.14);
    const Manager::Var* var = Manager::Instance().getVariable("eventExtraInfo(pi)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
  }

  TEST_F(MetaVariableTest, eventCached)
  {
    const Manager::Var* var = Manager::Instance().getVariable("eventCached(constant(3.14))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    EXPECT_TRUE(eventExtraInfo.isValid());
    EXPECT_TRUE(eventExtraInfo->hasExtraInfo("__constant__bo3__pt14__bc"));
    EXPECT_FLOAT_EQ(eventExtraInfo->getExtraInfo("__constant__bo3__pt14__bc"), 3.14);
    eventExtraInfo->addExtraInfo("__eventExtraInfo__bopi__bc", 3.14);
    var = Manager::Instance().getVariable("eventCached(eventExtraInfo(pi))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 3.14);
  }

  TEST_F(MetaVariableTest, particleCached)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    const Manager::Var* var = Manager::Instance().getVariable("particleCached(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);
    EXPECT_TRUE(p.hasExtraInfo("__px"));
    EXPECT_FLOAT_EQ(p.getExtraInfo("__px"), 0.1);
    p.addExtraInfo("__py", -0.5); // NOT -0.4 because we want to see if the cache is used instead of py!
    var = Manager::Instance().getVariable("particleCached(py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.5);
  }

  TEST_F(MetaVariableTest, formula)
  {
    // see also unit tests in framework/formula_parser.cc
    //
    // keep particle-based tests here, and operator precidence tests (etc) in
    // framework with the parser itself

    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("formula(px + py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.3);

    var = Manager::Instance().getVariable("formula(px - py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.5);

    var = Manager::Instance().getVariable("formula(px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -0.04);

    var = Manager::Instance().getVariable("formula(py / px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -4.0);

    var = Manager::Instance().getVariable("formula(px ^ E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.01);

    var = Manager::Instance().getVariable("formula(px * py + pz)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);

    var = Manager::Instance().getVariable("formula(pz + px * py)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.76, 1e-6);

    var = Manager::Instance().getVariable("formula(pt)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.41231057);
    double pt = var->function(&p);

    var = Manager::Instance().getVariable("formula((px**2 + py**2)**(1/2))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), pt);

    var = Manager::Instance().getVariable("formula(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -1.0);

    var = Manager::Instance().getVariable("formula(charge**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("formula(charge^2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);

    var = Manager::Instance().getVariable("formula(PDG * charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -11.0);

    var = Manager::Instance().getVariable("formula(PDG**2 * charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -121.0);

    var = Manager::Instance().getVariable("formula(10.58 - (px + py + pz - E)**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 8.33);

    var = Manager::Instance().getVariable("formula(-10.58 + (px + py + pz - E)**2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -8.33);

    var = Manager::Instance().getVariable("formula(-1.0 * PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), -11);
  }

  TEST_F(MetaVariableTest, passesCut)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    const Manager::Var* var = Manager::Instance().getVariable("passesCut(E < 3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0);
    EXPECT_FLOAT_EQ(var->function(nullptr), -999);

  }

  TEST_F(MetaVariableTest, nCleanedTracks)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> track_fit_results;
    StoreArray<Track> tracks;
    track_fit_results.registerInDataStore();
    tracks.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.1, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.15, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.4, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);
    track_fit_results.appendNew(TVector3(0.1, 0.1, 0.1), TVector3(0.6, 0.0, 0.0),
                                TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0);

    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 0);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 1);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 2);
    tracks.appendNew()->setTrackFitResultIndex(Const::pion, 3);

    const Manager::Var* var1 = Manager::Instance().getVariable("nCleanedTracks(p > 0.5)");
    EXPECT_FLOAT_EQ(var1->function(nullptr), 1);

    const Manager::Var* var2 = Manager::Instance().getVariable("nCleanedTracks(p > 0.2)");
    EXPECT_FLOAT_EQ(var2->function(nullptr), 2);

    const Manager::Var* var3 = Manager::Instance().getVariable("nCleanedTracks()");
    EXPECT_FLOAT_EQ(var3->function(nullptr), 4);


  }

  TEST_F(MetaVariableTest, NumberOfMCParticlesInEvent)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ 0.1 , -0.4, 0.8, 4.0 }, 11);

    StoreArray<MCParticle> mcParticles;
    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);


    const Manager::Var* var = Manager::Instance().getVariable("NumberOfMCParticlesInEvent(11)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 2);

  }

  TEST_F(MetaVariableTest, daughterInvariantMass)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(2, 2, 2, 4.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterInvariantMass(6)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -999.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 2.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0, 1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 4.0);

    var = Manager::Instance().getVariable("daughterInvariantMass(0, 1, 2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 6.0);
  }

  TEST_F(MetaVariableTest, daughter)
  {
    TLorentzVector momentum;
    const int nDaughters = 6;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(i * 1.0, 1, 1, 1), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughter(6, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -999.0);

    var = Manager::Instance().getVariable("daughter(0, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(p), 0.0, 1e-6);

    var = Manager::Instance().getVariable("daughter(1, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 1.0);

    var = Manager::Instance().getVariable("daughter(2, px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 2.0);
  }

  TEST_F(MetaVariableTest, mcDaughter)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for B -> (muon -> electron + muon_neutrino) + anti_muon_neutrino
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(-521);
    graphParticleMother.setPDG(13);
    graphParticleAunt.setPDG(-14);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(14);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);
    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* pGrandMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), -521);
    pGrandMother->addRelationTo(mcGrandMother);

    auto* pMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);
    pMother->addRelationTo(mcMother);

    // Test for particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);

    // Test for particle that has MC match, but MC match has no daughter
    auto* p_noDaughter = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p_noDaughter->addRelationTo(mcDaughter1);

    const Manager::Var* var = Manager::Instance().getVariable("mcDaughter(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 13);
    EXPECT_FLOAT_EQ(var->function(pMother), 11);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -999);
    EXPECT_FLOAT_EQ(var->function(p_noDaughter), -999);
    var = Manager::Instance().getVariable("mcDaughter(1, PDG)");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), -14);
    EXPECT_FLOAT_EQ(var->function(pMother), 14);
    // Test for particle where mc daughter index is out of range of mc daughters
    var = Manager::Instance().getVariable("mcDaughter(2, PDG)");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), -999);
    EXPECT_FLOAT_EQ(var->function(pMother), -999);
    // Test nested application of mcDaughter
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(0, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 11);
    EXPECT_FLOAT_EQ(var->function(pMother), -999);
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(1, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), 14);
    var = Manager::Instance().getVariable("mcDaughter(0, mcDaughter(2, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), -999);
    var = Manager::Instance().getVariable("mcDaughter(1, mcDaughter(0, PDG))");
    EXPECT_FLOAT_EQ(var->function(pGrandMother), -999);
  }

  TEST_F(MetaVariableTest, mcMother)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerInDataStore();
    mcParticles.registerInDataStore();
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    // Create MC graph for B -> (muon -> electron + muon_neutrino) + anti_muon_neutrino
    MCParticleGraph mcGraph;

    MCParticleGraph::GraphParticle& graphParticleGrandMother = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleMother = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleAunt = mcGraph.addParticle();

    MCParticleGraph::GraphParticle& graphParticleDaughter1 = mcGraph.addParticle();
    MCParticleGraph::GraphParticle& graphParticleDaughter2 = mcGraph.addParticle();

    graphParticleGrandMother.setPDG(-521);
    graphParticleMother.setPDG(13);
    graphParticleAunt.setPDG(-14);
    graphParticleDaughter1.setPDG(11);
    graphParticleDaughter2.setPDG(14);

    graphParticleMother.comesFrom(graphParticleGrandMother);
    graphParticleAunt.comesFrom(graphParticleGrandMother);
    graphParticleDaughter1.comesFrom(graphParticleMother);
    graphParticleDaughter2.comesFrom(graphParticleMother);

    mcGraph.generateList();

    // Get MC Particles from StoreArray
    auto* mcGrandMother = mcParticles[0];
    mcGrandMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcMother = mcParticles[1];
    mcMother->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcAunt = mcParticles[2];
    mcAunt->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter1 = mcParticles[3];
    mcDaughter1->setStatus(MCParticle::c_PrimaryParticle);

    auto* mcDaughter2 = mcParticles[4];
    mcDaughter2->setStatus(MCParticle::c_PrimaryParticle);

    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcDaughter1);

    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 14);
    p2->addRelationTo(mcDaughter2);

    auto* pMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 13);
    pMother->addRelationTo(mcMother);

    // For test of particle that has no MC match
    auto* p_noMC = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);

    // For test of particle that has MC match, but MC match has no mother
    auto* p_noMother = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), -521);
    p_noMother->addRelationTo(mcGrandMother);

    const Manager::Var* var = Manager::Instance().getVariable("mcMother(PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), 13);
    EXPECT_FLOAT_EQ(var->function(p2), 13);
    EXPECT_FLOAT_EQ(var->function(pMother), -521);
    EXPECT_FLOAT_EQ(var->function(p_noMC), -999);
    EXPECT_FLOAT_EQ(var->function(p_noMother), -999);

    // Test if nested calls of mcMother work correctly
    var = Manager::Instance().getVariable("mcMother(mcMother(PDG))");
    EXPECT_FLOAT_EQ(var->function(p1), -521);
  }

  TEST_F(MetaVariableTest, daughterProductOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterProductOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 24.0);
  }

  TEST_F(MetaVariableTest, daughterSumOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? 211 : -211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterSumOf(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 10.0);

  }

  TEST_F(MetaVariableTest, daughterDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterDiffOf(0, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -222);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 222);

    var = Manager::Instance().getVariable("daughterDiffOf(0, 1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -200);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

    var = Manager::Instance().getVariable("daughterDiffOf(1, 3, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

    var = Manager::Instance().getVariable("daughterDiffOf(0, 2, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0);

  }

  TEST_F(MetaVariableTest, daughterClusterAngleInBetween)
  {
    // declare all the array we need
    StoreArray<Particle> particles, particles_noclst;
    std::vector<int> daughterIndices, daughterIndices_noclst;

    //proxy initialize where to declare the needed array
    DataStore::Instance().setInitializeActive(true);
    StoreArray<ECLCluster> eclclusters;
    eclclusters.registerInDataStore();
    particles.registerRelationTo(eclclusters);
    DataStore::Instance().setInitializeActive(false);

    // create two Lorentz vectors that are back to back in the CMS and boost them to the Lab frame
    const float px_CM = 2.;
    const float py_CM = 1.;
    const float pz_CM = 3.;
    float E_CM;
    E_CM = sqrt(pow(px_CM, 2) + pow(py_CM, 2) + pow(pz_CM, 2));
    TLorentzVector momentum, momentum_noclst;
    TLorentzVector dau0_4vec_CM(px_CM, py_CM, pz_CM, E_CM), dau1_4vec_CM(-px_CM, -py_CM, -pz_CM, E_CM);
    TLorentzVector dau0_4vec_Lab, dau1_4vec_Lab;
    dau0_4vec_Lab = PCmsLabTransform::cmsToLab(
                      dau0_4vec_CM); //why is eveybody using the extendend method when there are the functions that do all the steps for us?
    dau1_4vec_Lab = PCmsLabTransform::cmsToLab(dau1_4vec_CM);

    // add the two photons (now in the Lab frame) as the two daughters of some particle and create the latter
    Particle dau0_noclst(dau0_4vec_Lab, 22);
    momentum += dau0_noclst.get4Vector();
    Particle* newDaughter0_noclst = particles.appendNew(dau0_noclst);
    daughterIndices_noclst.push_back(newDaughter0_noclst->getArrayIndex());
    Particle dau1_noclst(dau1_4vec_Lab, 22);
    momentum += dau1_noclst.get4Vector();
    Particle* newDaughter1_noclst = particles.appendNew(dau1_noclst);
    daughterIndices_noclst.push_back(newDaughter1_noclst->getArrayIndex());
    const Particle* par_noclst = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices_noclst);

    // grab variables
    const Manager::Var* var = Manager::Instance().getVariable("daughterClusterAngleInBetween(0, 1)");
    const Manager::Var* varCMS = Manager::Instance().getVariable("useCMSFrame(daughterClusterAngleInBetween(0, 1))");

    // when no relations are set between the particles and the eclClusters, nan is expected to be returned
    ASSERT_NE(var, nullptr);
    EXPECT_TRUE(std::isnan(var->function(par_noclst)));

    // set relations between particles and eclClusters
    ECLCluster* eclst0 = eclclusters.appendNew(ECLCluster());
    eclst0->setEnergy(dau0_4vec_Lab.E());
    eclst0->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
    eclst0->setClusterId(1);
    eclst0->setTheta(dau0_4vec_Lab.Theta());
    eclst0->setPhi(dau0_4vec_Lab.Phi());
    eclst0->setR(148.4);
    ECLCluster* eclst1 = eclclusters.appendNew(ECLCluster());
    eclst1->setEnergy(dau1_4vec_Lab.E());
    eclst1->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
    eclst1->setClusterId(2);
    eclst1->setTheta(dau1_4vec_Lab.Theta());
    eclst1->setPhi(dau1_4vec_Lab.Phi());
    eclst1->setR(148.5);

    const Particle* newDaughter0 = particles.appendNew(Particle(eclclusters[0]));
    daughterIndices.push_back(newDaughter0->getArrayIndex());
    const Particle* newDaughter1 = particles.appendNew(Particle(eclclusters[1]));
    daughterIndices.push_back(newDaughter1->getArrayIndex());

    const Particle* par = particles.appendNew(momentum, 111, Particle::c_Unflavored, daughterIndices);

    //now we expect non-nan results
    EXPECT_FLOAT_EQ(var->function(par), 2.8614323);
    EXPECT_FLOAT_EQ(varCMS->function(par), M_PI);
  }

  TEST_F(MetaVariableTest, daughterNormDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterNormDiffOf(0, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -222 / 200.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 222 / 200.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(0, 1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -200 / 222.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), -0 / 22.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(1, 3, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0 / 22.);

    var = Manager::Instance().getVariable("daughterNormDiffOf(0, 2, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 0 / 422.);

  }

  TEST_F(MetaVariableTest, daughterMotherDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterMotherDiffOf(1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 422);

    var = Manager::Instance().getVariable("daughterMotherDiffOf(1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 400);

    var = Manager::Instance().getVariable("daughterMotherDiffOf(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 200);

  }

  TEST_F(MetaVariableTest, daughterMotherNormDiffOf)
  {
    TLorentzVector momentum;
    const int nDaughters = 4;
    StoreArray<Particle> particles;
    std::vector<int> daughterIndices;
    for (int i = 0; i < nDaughters; i++) {
      Particle d(TLorentzVector(1, 1, 1, i * 1.0 + 1.0), (i % 2) ? -11 : 211);
      momentum += d.get4Vector();
      Particle* newDaughters = particles.appendNew(d);
      daughterIndices.push_back(newDaughters->getArrayIndex());
    }
    const Particle* p = particles.appendNew(momentum, 411, Particle::c_Unflavored, daughterIndices);

    const Manager::Var* var = Manager::Instance().getVariable("daughterMotherNormDiffOf(1, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 422 / 400.);

    var = Manager::Instance().getVariable("daughterMotherNormDiffOf(1, abs(PDG))");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 400 / 422.);

    var = Manager::Instance().getVariable("daughterMotherNormDiffOf(0, PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p), 200 / 622.);

  }

  TEST_F(MetaVariableTest, constant)
  {

    const Manager::Var* var = Manager::Instance().getVariable("constant(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 1.0);

    var = Manager::Instance().getVariable("constant(0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(nullptr), 0.0);

  }

  TEST_F(MetaVariableTest, abs)
  {
    Particle p({ 0.1 , -0.4, 0.8, 2.0 }, 11);
    Particle p2({ -0.1 , -0.4, 0.8, 4.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("abs(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 0.1);
    EXPECT_FLOAT_EQ(var->function(&p2), 0.1);

  }

  TEST_F(MetaVariableTest, sin)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("sin(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(&p), 1.0);
    EXPECT_ALL_NEAR(var->function(&p2), 0.0, 1e-6);

  }

  TEST_F(MetaVariableTest, cos)
  {
    Particle p({ 3.14159265359 / 2.0 , -0.4, 0.8, 1.0}, 11);
    Particle p2({ 0.0 , -0.4, 0.8, 1.0 }, -11);

    const Manager::Var* var = Manager::Instance().getVariable("cos(px)");
    ASSERT_NE(var, nullptr);
    EXPECT_ALL_NEAR(var->function(&p), 0.0, 1e-6);
    EXPECT_FLOAT_EQ(var->function(&p2), 1.0);

  }

  TEST_F(MetaVariableTest, NBDeltaIfMissingDeathTest)
  {
    //Variable got removed, test for absence
    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(TOP, 11)"));
    EXPECT_B2FATAL(Manager::Instance().getVariable("NBDeltaIfMissing(ARICH, 11)"));
  }

  TEST_F(MetaVariableTest, matchedMC)
  {
    DataStore::Instance().setInitializeActive(true);
    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> particles;
    particles.registerRelationTo(mcParticles);
    DataStore::Instance().setInitializeActive(false);

    auto* mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p1 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p1->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(-11);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p2 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p2->addRelationTo(mcParticle);

    mcParticle = mcParticles.appendNew();
    mcParticle->setPDG(22);
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
    auto* p3 = particles.appendNew(TLorentzVector({ 0.0 , -0.4, 0.8, 1.0}), 11);
    p3->addRelationTo(mcParticle);

    // Test if matchedMC also works for particle which already is an MCParticle.
    auto* p4 = particles.appendNew(mcParticle);

    const Manager::Var* var = Manager::Instance().getVariable("matchedMC(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -1);
    EXPECT_FLOAT_EQ(var->function(p2), 1);
    EXPECT_FLOAT_EQ(var->function(p3), 0);
    EXPECT_FLOAT_EQ(var->function(p4), 0);
  }

  TEST_F(MetaVariableTest, countInList)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    particles.appendNew(Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));
    particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));
    particles.appendNew(Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));
    particles.appendNew(Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 5));
    particles.appendNew(Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 6));

    outputList->addParticle(0, 22, Particle::c_Unflavored);
    outputList->addParticle(1, 22, Particle::c_Unflavored);
    outputList->addParticle(2, 22, Particle::c_Unflavored);
    outputList->addParticle(3, 22, Particle::c_Unflavored);
    outputList->addParticle(4, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("countInList(pList1, E < 0.85)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 2);

    var = Manager::Instance().getVariable("countInList(pList1)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 5);

    var = Manager::Instance().getVariable("countInList(pList1, E > 5)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 0);

    var = Manager::Instance().getVariable("countInList(pList1, E < 5)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(nullptr), 5);
  }

  TEST_F(MetaVariableTest, isInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // mock up two photons
    Particle goingin({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0);
    Particle notgoingin({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1);
    auto* inthelist = particles.appendNew(goingin);
    auto* notinthelist = particles.appendNew(notgoingin);

    // put the the zeroth one in the list the first on not in the list
    gammalist->addParticle(0, 22, Particle::c_Unflavored);

    // get the variables
    const Manager::Var* vnonsense = Manager::Instance().getVariable("isInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable("isInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(notinthelist));
    EXPECT_FLOAT_EQ(vsensible->function(inthelist), 1.0);
    EXPECT_FLOAT_EQ(vsensible->function(notinthelist), 0.0);
  }

  TEST_F(MetaVariableTest, totalEnergyOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalEnergyOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalEnergyOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 4.3);
  }
  TEST_F(MetaVariableTest, totalPxOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPxOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPxOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 2.2);
  }
  TEST_F(MetaVariableTest, totalPyOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPyOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPyOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 1.5);
  }
  TEST_F(MetaVariableTest, totalPzOfParticlesInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "totalPzOfParticlesInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "totalPzOfParticlesInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), 3.1);
  }
  TEST_F(MetaVariableTest, maxPtInList)
  {
    // we need the particles StoreArray
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    // create a photon list for testing
    StoreObjPtr<ParticleList> gammalist("testGammaList");
    DataStore::Instance().setInitializeActive(true);
    gammalist.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    gammalist.create();
    gammalist->initialize(22, "testGammaList");

    // create some photons in an stdvector
    std::vector<Particle> gammavector = {
      Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 0),
      Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 1),
      Particle({0.4 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2),
      Particle({0.5 , 0.4 , 0.8 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3),
      Particle({0.3 , 0.3 , 0.4 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4)
    };

    // put the photons in the StoreArray
    for (const auto g : gammavector)
      particles.appendNew(g);

    // put the photons in the test list
    for (size_t i = 0; i < gammavector.size(); i++)
      gammalist->addParticle(i, 22, Particle::c_Unflavored);

    // get their total energy
    const Manager::Var* vnonsense = Manager::Instance().getVariable(
                                      "maxPtInList(NONEXISTANTLIST)");
    const Manager::Var* vsensible = Manager::Instance().getVariable(
                                      "maxPtInList(testGammaList)");

    // -
    EXPECT_B2FATAL(vnonsense->function(nullptr));
    EXPECT_FLOAT_EQ(vsensible->function(nullptr), sqrt(0.5 * 0.5 + 0.4 * 0.4));
  }


  TEST_F(MetaVariableTest, numberOfNonOverlappingParticles)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    auto* p1 = particles.appendNew(Particle({0.5 , 0.4 , 0.5 , 0.8}, 22, Particle::c_Unflavored, Particle::c_Undefined, 2));
    auto* p2 = particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));
    auto* p3 = particles.appendNew(Particle({0.5 , 0.2 , 0.7 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));

    outputList->addParticle(0, 22, Particle::c_Unflavored);
    outputList->addParticle(1, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("numberOfNonOverlappingParticles(pList1)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p1), 1);
    EXPECT_DOUBLE_EQ(var->function(p2), 1);
    EXPECT_DOUBLE_EQ(var->function(p3), 2);

  }

  TEST_F(MetaVariableTest, veto)
  {
    StoreArray<Particle> particles;
    DataStore::EStoreFlags flags = DataStore::c_DontWriteOut;

    const Particle* p = particles.appendNew(Particle({0.8 , 0.8 , 1.131370849898476039041351 , 1.6}, 22,
                                                     Particle::c_Unflavored, Particle::c_Undefined, 1));

    StoreObjPtr<ParticleList> outputList("pList1");
    DataStore::Instance().setInitializeActive(true);
    outputList.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList.create();
    outputList->initialize(22, "pList1");

    particles.appendNew(Particle({0.5 , 0.4953406774856531014212777 , 0.5609256753154148484773173 , 0.9}, 22,
                                 Particle::c_Unflavored, Particle::c_Undefined, 2));         //m=0.135
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 3));    //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 4));    //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 5));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 6));    //m=0.0036

    outputList->addParticle(1, 22, Particle::c_Unflavored);
    outputList->addParticle(2, 22, Particle::c_Unflavored);
    outputList->addParticle(3, 22, Particle::c_Unflavored);
    outputList->addParticle(4, 22, Particle::c_Unflavored);
    outputList->addParticle(5, 22, Particle::c_Unflavored);

    StoreObjPtr<ParticleList> outputList2("pList2");
    DataStore::Instance().setInitializeActive(true);
    outputList2.registerInDataStore(flags);
    DataStore::Instance().setInitializeActive(false);
    outputList2.create();
    outputList2->initialize(22, "pList2");

    particles.appendNew(Particle({0.5 , -0.4 , 0.63246 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 7));    //m=1.1353
    particles.appendNew(Particle({0.5 , 0.2 , 0.72111 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 8));     //m=0.3582
    particles.appendNew(Particle({0.4 , 0.2 , 0.78102 , 0.9}, 22, Particle::c_Unflavored, Particle::c_Undefined, 9));     //m=0.3908
    particles.appendNew(Particle({0.5 , 0.4 , 0.89443 , 1.1}, 22, Particle::c_Unflavored, Particle::c_Undefined, 10));    //m=0.2369
    particles.appendNew(Particle({0.3 , 0.3 , 0.42426 , 0.6}, 22, Particle::c_Unflavored, Particle::c_Undefined, 11));    //m=0.0036

    outputList2->addParticle(6, 22, Particle::c_Unflavored);
    outputList2->addParticle(7, 22, Particle::c_Unflavored);
    outputList2->addParticle(8, 22, Particle::c_Unflavored);
    outputList2->addParticle(9, 22, Particle::c_Unflavored);
    outputList2->addParticle(10, 22, Particle::c_Unflavored);

    const Manager::Var* var = Manager::Instance().getVariable("veto(pList1, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 1);

    var = Manager::Instance().getVariable("veto(pList2, 0.130 < M < 0.140)");
    ASSERT_NE(var, nullptr);
    EXPECT_DOUBLE_EQ(var->function(p), 0);

  }

  class PIDVariableTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<ParticleExtraInfoMap> peim;
      StoreArray<TrackFitResult> tfrs;
      StoreArray<MCParticle> mcparticles;
      StoreArray<PIDLikelihood> likelihood;
      StoreArray<Particle> particles;
      StoreArray<Track> tracks;
      peim.registerInDataStore();
      tfrs.registerInDataStore();
      mcparticles.registerInDataStore();
      likelihood.registerInDataStore();
      particles.registerInDataStore();
      tracks.registerInDataStore();
      particles.registerRelationTo(likelihood);
      tracks.registerRelationTo(likelihood);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(PIDVariableTest, LogLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> tfrs;

    // create tracks and trackFitResutls
    TRandom3 generator;
    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);
    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);
    tfrs.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* allTrack = tracks.appendNew(mytrack);
    Track* noPIDTrack = tracks.appendNew(mytrack);
    Track* dEdxTrack = tracks.appendNew(mytrack);

    // Fill by hand likelihood values for all the detectors and hypothesis
    // This is clearly not a phyisical case, since a particle cannot leave good
    // signals in both TOP and ARICH
    auto* lAll = likelihood.appendNew();
    lAll->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    lAll->setLogLikelihood(Const::ARICH, Const::electron, 0.16);
    lAll->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    lAll->setLogLikelihood(Const::CDC, Const::electron, 0.12);
    lAll->setLogLikelihood(Const::SVD, Const::electron, 0.1);

    lAll->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    lAll->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    lAll->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    lAll->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    lAll->setLogLikelihood(Const::SVD, Const::pion, 0.28);

    lAll->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    lAll->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    lAll->setLogLikelihood(Const::ECL, Const::kaon, 0.34);
    lAll->setLogLikelihood(Const::CDC, Const::kaon, 0.36);
    lAll->setLogLikelihood(Const::SVD, Const::kaon, 0.38);

    lAll->setLogLikelihood(Const::TOP, Const::proton, 0.4);
    lAll->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    lAll->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    lAll->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    lAll->setLogLikelihood(Const::SVD, Const::proton, 0.48);

    lAll->setLogLikelihood(Const::TOP, Const::muon, 0.5);
    lAll->setLogLikelihood(Const::ARICH, Const::muon, 0.52);
    lAll->setLogLikelihood(Const::ECL, Const::muon, 0.54);
    lAll->setLogLikelihood(Const::CDC, Const::muon, 0.56);
    lAll->setLogLikelihood(Const::SVD, Const::muon, 0.58);

    lAll->setLogLikelihood(Const::TOP, Const::deuteron, 0.6);
    lAll->setLogLikelihood(Const::ARICH, Const::deuteron, 0.62);
    lAll->setLogLikelihood(Const::ECL, Const::deuteron, 0.64);
    lAll->setLogLikelihood(Const::CDC, Const::deuteron, 0.66);
    lAll->setLogLikelihood(Const::SVD, Const::deuteron, 0.68);


    // Likelihoods for a dEdx only case
    auto* ldEdx = likelihood.appendNew();
    ldEdx->setLogLikelihood(Const::CDC, Const::electron, 0.12);
    ldEdx->setLogLikelihood(Const::SVD, Const::electron, 0.1);

    ldEdx->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    ldEdx->setLogLikelihood(Const::SVD, Const::pion, 0.28);

    ldEdx->setLogLikelihood(Const::CDC, Const::kaon, 0.36);
    ldEdx->setLogLikelihood(Const::SVD, Const::kaon, 0.38);

    ldEdx->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    ldEdx->setLogLikelihood(Const::SVD, Const::proton, 0.48);

    ldEdx->setLogLikelihood(Const::CDC, Const::muon, 0.56);
    ldEdx->setLogLikelihood(Const::SVD, Const::muon, 0.58);

    ldEdx->setLogLikelihood(Const::CDC, Const::deuteron, 0.66);
    ldEdx->setLogLikelihood(Const::SVD, Const::deuteron, 0.68);


    allTrack->addRelationTo(lAll);
    dEdxTrack->addRelationTo(ldEdx);

    // Table with the sum(LogL) for several cases
    //      All  dEdx
    // e    0.7  0.22
    // mu   2.7  1.14
    // pi   1.2  0.54
    // k    1.7  0.74
    // p    2.2  0.94
    // d    3.2  1.34

    auto* particleAll = particles.appendNew(allTrack, Const::pion);
    auto* particledEdx = particles.appendNew(dEdxTrack, Const::pion);
    auto* particleNoID = particles.appendNew(noPIDTrack, Const::pion);

    double numsumexp = std::exp(0.7) + std::exp(2.7) + std::exp(1.2) + std::exp(1.7) + std::exp(2.2) + std::exp(3.2);

    // Basic PID quantities. Currently just wrappers for global probability.
    EXPECT_FLOAT_EQ(electronID(particleAll), std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(muonID(particleAll),     std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(pionID(particleAll),     std::exp(1.2) / numsumexp);
    EXPECT_FLOAT_EQ(kaonID(particleAll),     std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(protonID(particleAll),   std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(deuteronID(particleAll), std::exp(3.2) / numsumexp);

    // smart PID that takes the hypothesis into account
    auto* particleMuonAll = particles.appendNew(allTrack, Const::muon);
    auto* particleKaonAll = particles.appendNew(allTrack, Const::kaon);
    auto* particleElectronAll = particles.appendNew(allTrack, Const::electron);
    auto* particleProtonAll = particles.appendNew(allTrack, Const::proton);
    auto* particleDeuteronAll = particles.appendNew(allTrack, Const::deuteron);
    EXPECT_FLOAT_EQ(particleID(particleAll), std::exp(1.2) / numsumexp); // there's already a pion
    EXPECT_FLOAT_EQ(particleID(particleMuonAll), std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleKaonAll), std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleElectronAll), std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleProtonAll),   std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(particleID(particleDeuteronAll), std::exp(3.2) / numsumexp);

    // Check what hapens if no Likelihood is available
    EXPECT_TRUE(std::isnan(electronID(particleNoID)));
    EXPECT_TRUE(std::isnan(muonID(particleNoID)));
    EXPECT_TRUE(std::isnan(pionID(particleNoID)));
    EXPECT_TRUE(std::isnan(kaonID(particleNoID)));
    EXPECT_TRUE(std::isnan(protonID(particleNoID)));
    EXPECT_TRUE(std::isnan(deuteronID(particleNoID)));

    //expert stuff: LogL values
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP)")->function(particleAll), 0.18);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, ALL)")->function(particleAll), 0.70);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(2212, TOP, CDC)")->function(particleAll), 0.86);

    // global probability
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(1000010020, ALL)")->function(particleAll),
                    std::exp(3.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(2212, ALL)")->function(particleAll),
                    std::exp(2.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particleAll),
                    std::exp(1.2) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(321, ALL)")->function(particleAll),
                    std::exp(1.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(13, ALL)")->function(particleAll),
                    std::exp(2.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(11, ALL)")->function(particleAll),
                    std::exp(0.7) / numsumexp);
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particledEdx),
                    std::exp(0.54) / (std::exp(0.22) + std::exp(1.14) + std::exp(0.54) + std::exp(0.74) + std::exp(0.94) + std::exp(1.34)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(particledEdx),
                    Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, SVD)")->function(particleAll));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(211, CDC)")->function(particledEdx),
                    Manager::Instance().getVariable("pidProbabilityExpert(211, CDC)")->function(particleAll));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidProbabilityExpert(321, CDC)")->function(particleAll),
                    std::exp(0.36) / (std::exp(0.12) + std::exp(0.26) + std::exp(0.36) + std::exp(0.46) + std::exp(0.56) + std::exp(0.66)));

    // binary probability
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ALL)")->function(particleAll),
                    1.0 / (1.0 + std::exp(2.2 - 1.7)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ALL)")->function(particledEdx),
                    1.0 / (1.0 + std::exp(0.94 - 0.74)));
    EXPECT_FLOAT_EQ(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, SVD)")->function(particleAll),
                    1.0 / (1.0 + std::exp(0.94 - 0.74)));

    // No likelihood available
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, KLM)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP, CDC, SVD)")->function(particleNoID)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidLogLikelihoodValueExpert(11, TOP)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, KLM)")->function(particledEdx)));
    EXPECT_TRUE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ECL, TOP, ARICH)")->function(
                             particledEdx)));
    EXPECT_FALSE(std::isnan(Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, ECL, TOP, ARICH, SVD)")->function(
                              particledEdx)));
  }

  TEST_F(PIDVariableTest, MissingLikelihood)
  {
    StoreArray<PIDLikelihood> likelihood;
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> tfrs;

    // create tracks and trackFitResutls
    TRandom3 generator;
    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);
    // Generate a random put orthogonal pair of vectors in the r-phi plane
    TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
    d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
    // Add a random z component
    TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
    TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);
    tfrs.appendNew(position, momentum, cov6, charge, Const::electron, pValue, bField, CDCValue, 16777215);
    Track mytrack;
    mytrack.setTrackFitResultIndex(Const::electron, 0);
    Track* savedTrack1 = tracks.appendNew(mytrack);
    Track* savedTrack2 = tracks.appendNew(mytrack);
    Track* savedTrack3 = tracks.appendNew(mytrack);
    Track* savedTrack4 = tracks.appendNew(mytrack);

    auto* l1 = likelihood.appendNew();
    l1->setLogLikelihood(Const::TOP, Const::electron, 0.18);
    l1->setLogLikelihood(Const::ECL, Const::electron, 0.14);
    savedTrack1->addRelationTo(l1);

    auto* electron = particles.appendNew(savedTrack1, Const::electron);

    auto* l2 = likelihood.appendNew();
    l2->setLogLikelihood(Const::TOP, Const::pion, 0.2);
    l2->setLogLikelihood(Const::ARICH, Const::pion, 0.22);
    l2->setLogLikelihood(Const::ECL, Const::pion, 0.24);
    l2->setLogLikelihood(Const::CDC, Const::pion, 0.26);
    l2->setLogLikelihood(Const::SVD, Const::pion, 0.28);
    savedTrack2->addRelationTo(l2);

    auto* pion = particles.appendNew(savedTrack2, Const::pion);

    auto* l3 = likelihood.appendNew();
    l3->setLogLikelihood(Const::TOP, Const::kaon, 0.3);
    l3->setLogLikelihood(Const::ARICH, Const::kaon, 0.32);
    savedTrack3->addRelationTo(l3);

    auto* kaon = particles.appendNew(savedTrack3, Const::kaon);

    auto* l4 = likelihood.appendNew();
    l4->setLogLikelihood(Const::ARICH, Const::proton, 0.42);
    l4->setLogLikelihood(Const::ECL, Const::proton, 0.44);
    l4->setLogLikelihood(Const::CDC, Const::proton, 0.46);
    l4->setLogLikelihood(Const::SVD, Const::proton, 0.48);
    savedTrack4->addRelationTo(l4);

    auto* proton = particles.appendNew(savedTrack4, Const::proton);

    const Manager::Var* varMissECL = Manager::Instance().getVariable("pidMissingProbabilityExpert(ECL)");
    const Manager::Var* varMissTOP = Manager::Instance().getVariable("pidMissingProbabilityExpert(TOP)");
    const Manager::Var* varMissARICH = Manager::Instance().getVariable("pidMissingProbabilityExpert(ARICH)");


    EXPECT_FLOAT_EQ(varMissTOP->function(electron), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(kaon), 0.0);
    EXPECT_FLOAT_EQ(varMissTOP->function(proton), 1.0);

    EXPECT_FLOAT_EQ(varMissARICH->function(electron), 1.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(kaon), 0.0);
    EXPECT_FLOAT_EQ(varMissARICH->function(proton), 0.0);

    EXPECT_FLOAT_EQ(varMissECL->function(electron), 0.0);
    EXPECT_FLOAT_EQ(varMissECL->function(pion), 0.0);
    EXPECT_FLOAT_EQ(varMissECL->function(kaon), 1.0);
    EXPECT_FLOAT_EQ(varMissECL->function(proton), 0.0);
  }


  class ECLVariableTest : public ::testing::Test {
  protected:
    /** register Particle and ECLCluster arrays. */
    virtual void SetUp()
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
      unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);

      for (int i = 0; i < tracks.getEntries(); ++i) {
        int charge = (i % 2 == 0) ? +1 : -1;
        TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
        d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
        TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
        TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
        trackFits.appendNew(position, momentum, cov6, charge, Const::pion, 0.5, 1.5, CDCValue, 16777215);
        tracks[i]->setTrackFitResultIndex(Const::pion, i);
      }

      // add some ECL clusters
      ECLCluster* e1 = eclclusters.appendNew(ECLCluster());
      e1->setEnergy(0.3);
      e1->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e1->setClusterId(1);
      // leave this guy with default theta and phi
      ECLCluster* e2 = eclclusters.appendNew(ECLCluster());
      e2->setEnergy(0.6);
      e2->setTheta(1.0); // somewhere in the barrel
      e2->setPhi(2.0);
      e2->setR(148.5);
      e2->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e2->setClusterId(2);
      ECLCluster* e3 = eclclusters.appendNew(ECLCluster());
      e3->setEnergy(0.15);
      e3->setTheta(0.2); // somewhere in the fwd encap
      e3->setPhi(1.5);
      e3->setR(200.0);
      e3->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e3->setClusterId(3);

      // aaand add clusters related to the tracks
      ECLCluster* e4 = eclclusters.appendNew(ECLCluster());
      e4->setEnergy(0.2);
      e4->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e4->setClusterId(4);
      t1->addRelationTo(e4);
      e4->setIsTrack(true);

      ECLCluster* e5 = eclclusters.appendNew(ECLCluster());
      e5->setEnergy(0.3);
      e5->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e5->setClusterId(5);
      t2->addRelationTo(e5);
      e5->setIsTrack(true);

      ECLCluster* e6 = eclclusters.appendNew(ECLCluster());
      e6->setEnergy(0.2);
      e6->setHypothesisId(ECLCluster::Hypothesis::c_nPhotons);
      e6->setClusterId(6);
      t3->addRelationTo(e6);
      t4->addRelationTo(e6);
      // two tracks are related to this cluster this can happen due to real
      // physics and we should be able to cope
      e6->setIsTrack(true);

    }

    /** clear datastore */
    virtual void TearDown()
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
    EXPECT_FLOAT_EQ(clusterPhiCMS->function(gammalist->getParticle(1)), 2.0442522);
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(1)), 1.0);
    EXPECT_FLOAT_EQ(clusterThetaCMS->function(gammalist->getParticle(1)), 1.2625268);

    // test cluster quantities directly (lab system only)
    EXPECT_FLOAT_EQ(clusterPhi->function(gammalist->getParticle(0)), eclclusters[0]->getPhi());
    EXPECT_FLOAT_EQ(clusterTheta->function(gammalist->getParticle(0)), eclclusters[0]->getTheta());


  }


  TEST_F(ECLVariableTest, WholeEventClosure)
  {
    // we need the particles, tracks, and ECLClusters StoreArrays
    StoreArray<Particle> particles;
    StoreArray<Track> tracks; StoreArray<ECLCluster> eclclusters;

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
      eclEnergy += eclclusters[i]->getEnergy();
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

  class KLMVariableTest : public ::testing::Test {
  protected:
    /** register Particle and KLMCluster arrays. */
    virtual void SetUp()
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
    virtual void TearDown()
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
    kLongList->initialize(130, kLongList.getName());
    muonsList.create();
    muonsList->initialize(13, muonsList.getName());
    amuonsList.create();
    amuonsList->initialize(-13, amuonsList.getName());
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
    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    for (int i = 0; i < tracks.getEntries(); ++i) {
      int charge = (i % 2 == 0) ? +1 : -1;
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));
      trackFits.appendNew(position, momentum, cov6, charge, Const::muon, 0.5, 1.5, CDCValue, 16777215);
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
    // possible case, but not covered

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

  TEST_F(KLMVariableTest, MoreClustersToOneTrack)
  {
    StoreArray<Particle> particles;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFits;
    StoreArray<KLMCluster> klmClusters;

    // add a TrackFitResult
    TRandom3 generator;

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);

    unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    trackFits.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215);

    // add one Track
    Track myTrack;
    myTrack.setTrackFitResultIndex(Const::muon, 0);
    Track* muonTrack = tracks.appendNew(myTrack);

    // add two KLMClusters
    KLMCluster* klm1 = klmClusters.appendNew(KLMCluster());
    klm1->setTime(1.1);
    klm1->setClusterPosition(1.1, 1.1, 1.0);
    klm1->setLayers(5);
    klm1->setInnermostLayer(1);
    klm1->setMomentumMag(1.0);
    KLMCluster* klm2 = klmClusters.appendNew(KLMCluster());
    klm2->setTime(1.2);
    klm2->setClusterPosition(1.2, 1.2, 2.0);
    klm2->setLayers(10);
    klm2->setInnermostLayer(2);
    klm2->setMomentumMag(1.0);

    // and add a relationship between the track and both clusters
    muonTrack->addRelationTo(klm1);
    muonTrack->addRelationTo(klm2);

    // add a Particle
    const Particle* muon = particles.appendNew(Particle(muonTrack, Const::muon));

    // grab variables
    const Manager::Var* vTrNClusters = Manager::Instance().getVariable("nMatchedKLMClusters");
    const Manager::Var* vClusterInnermostLayer = Manager::Instance().getVariable("klmClusterInnermostLayer");

    EXPECT_POSITIVE(vTrNClusters->function(muon));
    EXPECT_FLOAT_EQ(2.0, vClusterInnermostLayer->function(muon));

    // add a Pion - no clusters matched here
    trackFits.appendNew(position, momentum, cov6, charge, Const::pion, pValue, bField, CDCValue, 16777215);
    Track mySecondTrack;
    mySecondTrack.setTrackFitResultIndex(Const::pion, 0);
    Track* pionTrack = tracks.appendNew(mySecondTrack);
    const Particle* pion = particles.appendNew(Particle(pionTrack, Const::pion));

    EXPECT_FLOAT_EQ(0.0, vTrNClusters->function(pion));

  }

  class FlightInfoTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      StoreArray<MCParticle> mcParticles;
      StoreArray<Particle> particles;
      particles.registerRelationTo(mcParticles);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);


      // Insert MC particle logic here
      MCParticle mcKs;
      mcKs.setPDG(310);
      mcKs.setDecayVertex(4.0, 5.0, 0.0);
      mcKs.setMassFromPDG();
      mcKs.setMomentum(1.164, 1.55200, 0);
      mcKs.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCKs = mcParticles.appendNew(mcKs);



      MCParticle mcDp;
      mcDp.setPDG(411);
      mcDp.setDecayVertex(1.0, 1.0, 0.0);
      mcDp.setMassFromPDG();
      mcDp.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCDp = mcParticles.appendNew(mcDp);

      // Insert Reco particle logic here
      TLorentzVector momentum;
      TMatrixFSym error(7);
      error.Zero();
      error(0, 0) = 0.05;
      error(1, 1) = 0.2;
      error(2, 2) = 0.4;
      error(3, 3) = 0.01;
      error(4, 4) = 0.04;
      error(5, 5) = 0.01;
      error(6, 6) = 0.01;
      Particle pi(TLorentzVector(1.59607, 1.19705, 0, 2), 211);
      momentum += pi.get4Vector();
      Particle* newpi = particles.appendNew(pi);


      Particle Ks(TLorentzVector(1.164, 1.55200, 0, 2), 310);
      Ks.setVertex(TVector3(4.0, 5.0, 0.0));
      Ks.setMomentumVertexErrorMatrix(error);   // (order: px,py,pz,E,x,y,z)
      momentum += Ks.get4Vector();
      Ks.addExtraInfo("prodVertX", 1.0);
      Ks.addExtraInfo("prodVertY", 1.0);
      Ks.addExtraInfo("prodVertZ", 0.0);
      Ks.addExtraInfo("prodVertSxx", 0.04);
      Ks.addExtraInfo("prodVertSxy", 0.0);
      Ks.addExtraInfo("prodVertSxz", 0.0);
      Ks.addExtraInfo("prodVertSyx", 0.0);
      Ks.addExtraInfo("prodVertSyy", 0.0075);
      Ks.addExtraInfo("prodVertSyz", 0.0);
      Ks.addExtraInfo("prodVertSzx", 0.0);
      Ks.addExtraInfo("prodVertSzy", 0.0);
      Ks.addExtraInfo("prodVertSzz", 0.01);
      Particle* newKs = particles.appendNew(Ks);
      newKs->addRelationTo(newMCKs);


      Particle Dp(momentum, 411);
      Dp.appendDaughter(newpi);
      Dp.appendDaughter(newKs);
      TVector3 motherVtx(1.0, 1.0, 0.0);
      Dp.setVertex(motherVtx);
      Dp.setMomentumVertexErrorMatrix(error);   // (order: px,py,pz,E,x,y,z)
      Particle* newDp = particles.appendNew(Dp);
      newDp->addRelationTo(newMCDp);

    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };
  TEST_F(FlightInfoTest, flightDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0);
  }
  TEST_F(FlightInfoTest, flightDistanceErr)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceErr");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newKs), 0.0);
  }
  TEST_F(FlightInfoTest, flightTime)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("flightTime");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0 / Const::speedOfLight * newKs->getPDGMass() / newKs->getP());
  }

  TEST_F(FlightInfoTest, flightTimeErr)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[1]; //  Ks should have positive flight distance uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeErr");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newKs), 0.0);
  }


  TEST_F(FlightInfoTest, flightDistanceOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), 5.0);

    var = Manager::Instance().getVariable("flightDistanceOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }
  TEST_F(FlightInfoTest, flightDistanceOfDaughterErr)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks should have positive flight distance uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightDistanceOfDaughterErr(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newDp), 0.0);

    var = Manager::Instance().getVariable("flightDistanceOfDaughterErr(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }
  TEST_F(FlightInfoTest, flightTimeOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    const Particle* Ks = newDp->getDaughter(1);

    EXPECT_FLOAT_EQ(var->function(newDp), 5.0 / Const::speedOfLight * Ks->getPDGMass() / Ks->getP());

    var = Manager::Instance().getVariable("flightTimeOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }
  TEST_F(FlightInfoTest, flightTimeOfDaughterErr)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks should have positive flight time uncertainty

    const Manager::Var* var = Manager::Instance().getVariable("flightTimeOfDaughterErr(1)");
    ASSERT_NE(var, nullptr);
    EXPECT_GT(var->function(newDp), 0.0);

    var = Manager::Instance().getVariable("flightTimeOfDaughterErr(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }
  TEST_F(FlightInfoTest, mcFlightDistanceOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight distance of 5 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcFlightDistanceOfDaughter(1)");
    ASSERT_NE(var, nullptr);

    EXPECT_FLOAT_EQ(var->function(newDp), 5.0);

    var = Manager::Instance().getVariable("mcFlightDistanceOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }
  TEST_F(FlightInfoTest, mcFlightTimeOfDaughter)
  {
    StoreArray<Particle> particles;
    const Particle* newDp = particles[2]; // Get D+, its daughter Ks had flight time of 0.0427 us (t = d/c * m/p)

    const Manager::Var* var = Manager::Instance().getVariable("mcFlightTimeOfDaughter(1)");
    ASSERT_NE(var, nullptr);
    auto* Ks = newDp->getDaughter(1)->getRelatedTo<MCParticle>();
    double p = sqrt(Ks->getMomentum().X() * Ks->getMomentum().X() + Ks->getMomentum().Y() *
                    Ks->getMomentum().Y() + Ks->getMomentum().Z() * Ks->getMomentum().Z());
    EXPECT_FLOAT_EQ(var->function(newDp), 5.0 / Const::speedOfLight * Ks->getMass() / p);

    var = Manager::Instance().getVariable("mcFlightTimeOfDaughter(3)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newDp), -999.0);
  }

  TEST_F(FlightInfoTest, vertexDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 5.0);
  }

  TEST_F(FlightInfoTest, vertexDistanceError)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 0.2);
  }

  TEST_F(FlightInfoTest, vertexDistanceSignificance)
  {
    StoreArray<Particle> particles;
    const Particle* newKS = particles[1]; // Get KS, as it has both a production and decay vertex

    const Manager::Var* var = Manager::Instance().getVariable("vertexDistanceSignificance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKS), 25);
  }

  class VertexVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Particle>().registerInDataStore();
      StoreArray<MCParticle>().registerInDataStore();
      StoreArray<MCParticle> mcParticles;
      StoreArray<Particle> particles;
      particles.registerRelationTo(mcParticles);
      StoreObjPtr<ParticleExtraInfoMap>().registerInDataStore();
      DataStore::Instance().setInitializeActive(false);


      // Insert MC particle logic here
      MCParticle mcKs;
      mcKs.setPDG(310);
      mcKs.setDecayVertex(4.0, 5.0, 0.0);
      mcKs.setProductionVertex(TVector3(1.0, 2.0, 3.0));
      mcKs.setMassFromPDG();
      mcKs.setMomentum(1.164, 1.55200, 0);
      mcKs.setStatus(MCParticle::c_PrimaryParticle);
      MCParticle* newMCKs = mcParticles.appendNew(mcKs);

      Particle Ks(TLorentzVector(1.164, 1.55200, 0, 2), 310);
      Ks.setVertex(TVector3(4.0, 5.0, 0.0));
      Ks.addExtraInfo("prodVertX", 1.0);
      Ks.addExtraInfo("prodVertY", 2.0);
      Ks.addExtraInfo("prodVertZ", 3.0);
      Ks.addExtraInfo("prodVertSxx", 0.1);
      Ks.addExtraInfo("prodVertSxy", 0.2);
      Ks.addExtraInfo("prodVertSxz", 0.3);
      Ks.addExtraInfo("prodVertSyx", 0.4);
      Ks.addExtraInfo("prodVertSyy", 0.5);
      Ks.addExtraInfo("prodVertSyz", 0.6);
      Ks.addExtraInfo("prodVertSzx", 0.7);
      Ks.addExtraInfo("prodVertSzy", 0.8);
      Ks.addExtraInfo("prodVertSzz", 0.9);
      Particle* newKs = particles.appendNew(Ks);
      newKs->addRelationTo(newMCKs);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  // MC vertex tests
  TEST_F(VertexVariablesTest, mcX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay x is 4.0

    const Manager::Var* var = Manager::Instance().getVariable("mcX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 4.0);
  }

  TEST_F(VertexVariablesTest, mcY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay y is 5.0

    const Manager::Var* var = Manager::Instance().getVariable("mcY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 5.0);
  }

  TEST_F(VertexVariablesTest, mcZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth decay z is 0.0

    const Manager::Var* var = Manager::Instance().getVariable("mcZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 0.0);
  }


  TEST_F(VertexVariablesTest, mcDistance)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth distance of sqrt(41)

    const Manager::Var* var = Manager::Instance().getVariable("mcDistance");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(4.0 * 4.0 + 5.0 * 5.0));
  }

  TEST_F(VertexVariablesTest, mcRho)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had truth rho of sqrt(41)

    const Manager::Var* var = Manager::Instance().getVariable("mcRho");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(4.0 * 4.0 + 5.0 * 5.0));
  }

  TEST_F(VertexVariablesTest, mcProdVertexX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex x of 1.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 1.0);
  }

  TEST_F(VertexVariablesTest, mcProdVertexY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex y of 2.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 2.0);
  }

  TEST_F(VertexVariablesTest, mcProdVertexZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex z of 3.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("mcProdVertexZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 3.0);
  }

  // Production position tests

  TEST_F(VertexVariablesTest, prodVertexX)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex x of 1.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexX");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 1.0);
  }
  TEST_F(VertexVariablesTest, prodVertexY)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex y of 2.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexY");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 2.0);
  }
  TEST_F(VertexVariablesTest, prodVertexZ)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex z of 3.0 cm

    const Manager::Var* var = Manager::Instance().getVariable("prodVertexZ");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 3.0);
  }

  // Production Covariance tests

  TEST_F(VertexVariablesTest, prodVertexCov)
  {
    StoreArray<Particle> particles;
    const Particle* newKs = particles[0]; //  Ks had production vertex covariance xx of .1 cm

    //const Manager::Var* var = Manager::Instance().getVariable("prodVertexCovXX");
    const Manager::Var* var = Manager::Instance().getVariable("prodVertexCov(0,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), 0.1);
    var = Manager::Instance().getVariable("prodVertexCov(0,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.2);
    var = Manager::Instance().getVariable("prodVertexCov(0,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.3);
    var = Manager::Instance().getVariable("prodVertexCov(1,0)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.4);
    var = Manager::Instance().getVariable("prodVertexCov(1,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.5);
    var = Manager::Instance().getVariable("prodVertexCov(1,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.6);
    var = Manager::Instance().getVariable("prodVertexCov(2,0)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.7);
    var = Manager::Instance().getVariable("prodVertexCov(2,1)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.8);
    var = Manager::Instance().getVariable("prodVertexCov(2,2)");
    EXPECT_FLOAT_EQ(var->function(newKs), 0.9);
    var = Manager::Instance().getVariable("prodVertexXErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.1));
    var = Manager::Instance().getVariable("prodVertexYErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.5));
    var = Manager::Instance().getVariable("prodVertexZErr");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(newKs), sqrt(0.9));
  }

}
