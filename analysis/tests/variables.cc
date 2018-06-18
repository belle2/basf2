#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/EventVariables.h>
#include <analysis/VariableManager/PIDVariables.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <analysis/VariableManager/ROEVariables.h>
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

  TEST(ROEVariablesTest, Variable)
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

    // Create KLMCluster on ROE side
    KLMCluster myROEKLM;
    KLMCluster* savedROEKLM = myKLMClusters.appendNew(myROEKLM);

    // Create Track on ROE side
    // - create TFR
    TRandom3 generator;

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
    myROETrack.setTrackFitResultIndex(Const::muon, 1);
    Track* savedROETrack = myTracks.appendNew(myROETrack);

    // - create PID information, add relation
    PIDLikelihood myPID;
    myPID.setLogLikelihood(Const::TOP, Const::muon, 0.5);
    myPID.setLogLikelihood(Const::ARICH, Const::muon, 0.52);
    myPID.setLogLikelihood(Const::ECL, Const::muon, 0.54);
    myPID.setLogLikelihood(Const::CDC, Const::muon, 0.56);
    myPID.setLogLikelihood(Const::SVD, Const::muon, 0.58);
    PIDLikelihood* savedPID = myPIDLikelihoods.appendNew(myPID);

    savedROETrack->addRelationTo(savedPID);

    // Create ROE object, append tracks, clusters, add relation to particle
    RestOfEvent roe;
    roe.addTrack(savedROETrack);
    roe.addECLCluster(savedROEECL);
    roe.addKLMCluster(savedROEKLM);
    RestOfEvent* savedROE = myROEs.appendNew(roe);

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

    part->addRelationTo(savedROE);

    // ROE variables
    PCmsLabTransform T;
    float E0 = T.getCMSEnergy() / 2;

    TLorentzVector pTrack_ROE_Lab(momentum, TMath::Sqrt(Const::pion.getMass()*Const::pion.getMass() + 1.0 /*momentum.Mag2()*/));
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

    // TESTS FOR ROE STRUCTURE
    EXPECT_B2FATAL(savedROE->getTrackMask("noSuchMask"));
    EXPECT_B2FATAL(savedROE->getECLClusterMask("noSuchMask"));
    double fArray[6];
    EXPECT_B2FATAL(savedROE->fillFractions(fArray, "noSuchMask"));

    // TESTS FOR ROE VARIABLES
    const Manager::Var* var = Manager::Instance().getVariable("nROETracks(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROETracks(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROEECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROEECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROENeutralECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROENeutralECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nParticlesInROE(pi0:vartest)");
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

    var = Manager::Instance().getVariable("correctedB_deltae(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), corrRec4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("correctedB_deltae(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), rec4vecCMS.E() + rec4vecCMS.Vect().Mag() - E0);

    var = Manager::Instance().getVariable("correctedB_mbc(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - corrRec4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("correctedB_mbc(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("missM2(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), m4v0.Mag2());

    var = Manager::Instance().getVariable("missM2(mask2,0)");
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

    const Manager::Var* var = Manager::Instance().getVariable("matchedMC(charge)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(p1), -1);
    EXPECT_FLOAT_EQ(var->function(p2), 1);
    EXPECT_FLOAT_EQ(var->function(p3), 0);

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

}
