#include <ecl/modules/eclChargedPID/ECLChargedPIDModule.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dbobjects/ECLChargedPidPDFs.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>

#include <utility>
#include <gtest/gtest.h>

namespace Belle2 {

  class ECLChargedPIDTest : public ::testing::Test {

  protected:
    /** Set up a few arrays and objects in the datastore */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);

      StoreArray<ECLPidLikelihood> ecl_likelihoods;
      ecl_likelihoods.registerInDataStore();

      StoreArray<ECLPidLikelihood> ecl_likelihoods_plus;
      ecl_likelihoods_plus.registerInDataStore();

      StoreArray<ECLPidLikelihood> ecl_likelihoods_minus;
      ecl_likelihoods_minus.registerInDataStore();

      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }

  };

  /** Test ECLPidLikelihood setters and getters. */
  TEST_F(ECLChargedPIDTest, ECLPidLikelihoodSettersAndGetters)
  {

    StoreArray<ECLPidLikelihood> ecl_likelihoods;

    auto* lk = ecl_likelihoods.appendNew();

    lk->setLogLikelihood(Const::electron, 0.12);
    lk->setLogLikelihood(Const::pion, 0.28);
    lk->setLogLikelihood(Const::kaon, 0.38);
    lk->setLogLikelihood(Const::proton, 0.48);
    lk->setLogLikelihood(Const::muon, 0.58);

    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::electron), 0.12);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::pion), 0.28);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::kaon), 0.38);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::proton), 0.48);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::muon), 0.58);

  } // Testcases for ECLPidLikelihood setters and getters.

  /** Test ECL PDFs parameters and normalisation. Check separately +/- charge hypotheses. */
  TEST_F(ECLChargedPIDTest, TestPDFParamsAndNorm)
  {

    // std::list<const std::string> paramList;
    // const std::string eParams          = FileSystem::findFile("/data/ecl/electrons_N1.dat"); paramList.push_back(eParams);
    // const std::string muParams         = FileSystem::findFile("/data/ecl/muons_N1.dat"); paramList.push_back(muParams);
    // const std::string piParams         = FileSystem::findFile("/data/ecl/pions_N1.dat"); paramList.push_back(piParams);
    // const std::string kaonParams       = FileSystem::findFile("/data/ecl/kaons_N1.dat"); paramList.push_back(kaonParams);
    // const std::string protonParams     = FileSystem::findFile("/data/ecl/protons_N1.dat"); paramList.push_back(protonParams);
    // const std::string eAntiParams      = FileSystem::findFile("/data/ecl/electronsanti_N1.dat"); paramList.push_back(eAntiParams);
    // const std::string muAntiParams     = FileSystem::findFile("/data/ecl/muonsanti_N1.dat"); paramList.push_back(muAntiParams);
    // const std::string piAntiParams     = FileSystem::findFile("/data/ecl/pionsanti_N1.dat"); paramList.push_back(piAntiParams);
    // const std::string kaonAntiParams   = FileSystem::findFile("/data/ecl/kaonsanti_N1.dat"); paramList.push_back(kaonAntiParams);
    // const std::string protonAntiParams = FileSystem::findFile("/data/ecl/protonsanti_N1.dat"); paramList.push_back(protonAntiParams);

    // // Array of ECLAbsPdfs
    // ECL::ECLAbsPdf* eclPdfs[2][Const::ChargedStable::c_SetSize];

    // (eclPdfs[0][Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eAntiParams.c_str());  // e+
    // (eclPdfs[0][Const::muon.getIndex()]     = new ECL::ECLMuonPdf)    ->init(muAntiParams.c_str());  // mu+
    // (eclPdfs[0][Const::pion.getIndex()]     = new ECL::ECLPionPdf)    ->init(piParams.c_str()); // pi+
    // (eclPdfs[0][Const::kaon.getIndex()]     = new ECL::ECLKaonPdf)    ->init(kaonParams.c_str()); // K+
    // (eclPdfs[0][Const::proton.getIndex()]   = new ECL::ECLProtonPdf)  ->init(protonParams.c_str()); // p+
    // (eclPdfs[1][Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str()); // e-
    // (eclPdfs[1][Const::muon.getIndex()]     = new ECL::ECLMuonPdf)    ->init(muParams.c_str()); // mu-
    // (eclPdfs[1][Const::pion.getIndex()]     = new ECL::ECLPionPdf)    ->init(piAntiParams.c_str());  // pi-
    // (eclPdfs[1][Const::kaon.getIndex()]     = new ECL::ECLKaonPdf)    ->init(kaonAntiParams.c_str());  // K-
    // (eclPdfs[1][Const::proton.getIndex()]   = new ECL::ECLProtonPdf)  ->init(protonAntiParams.c_str());  // p-

    // // Set a generic value for E/p, and pick a generic (P,theta) bin
    // double eop = 0.781214;
    // double eop_mu = 0.3;
    // unsigned int idx_p = 2;
    // unsigned int idx_th = 4;

    // // Store the value of the PDFs in here, and pass it to the ECLPidLikelihood object later on.
    // float likelihoods_plus[Const::ChargedStable::c_SetSize];
    // float likelihoods_minus[Const::ChargedStable::c_SetSize];

    // // e-
    // ECL::ECLElectronPdf* pdf_e = dynamic_cast<ECL::ECLElectronPdf*>(eclPdfs[1][Const::electron.getIndex()]);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->fitrange_up, 1.2);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->mu1, 0.903937);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->sigma1, 0.014698);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->fraction, 0.018643);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->mu2, 0.968160);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->sigma2, 0.035553);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->alpha, 1.247767);
    // EXPECT_EQ(pdf_e->pdfParams(idx_p, idx_th)->nn, 3.413941);
    // likelihoods_minus[Const::electron.getIndex()] = pdf_e->pdf(eop, idx_p, idx_th);

    // // e+
    // ECL::ECLElectronPdf* pdf_antie = dynamic_cast<ECL::ECLElectronPdf*>(eclPdfs[0][Const::electron.getIndex()]);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->fitrange_up, 1.2);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->mu1, 0.972624);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->sigma1, 0.030186);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->fraction, 0.536367);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->mu2, 0.951174);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->sigma2, 0.050979);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->alpha, 1.339396);
    // EXPECT_EQ(pdf_antie->pdfParams(idx_p, idx_th)->nn, 2.944292);
    // likelihoods_plus[Const::electron.getIndex()] = pdf_antie->pdf(eop, idx_p, idx_th);

    // // mu-
    // ECL::ECLMuonPdf* pdf_mu = dynamic_cast<ECL::ECLMuonPdf*>(eclPdfs[1][Const::muon.getIndex()]);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->fitrange_up, 0.6);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->mu1, 0.276720);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->sigma1l, 0.010000);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->sigma1r, 0.099117);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->fraction, 0.351964);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->mu2, 0.300000);
    // EXPECT_EQ(pdf_mu->pdfParams(idx_p, idx_th)->sigma2, 0.067621);
    // likelihoods_minus[Const::muon.getIndex()] = pdf_mu->pdf(eop_mu, idx_p, idx_th);

    // // mu+
    // ECL::ECLMuonPdf* pdf_antimu = dynamic_cast<ECL::ECLMuonPdf*>(eclPdfs[0][Const::muon.getIndex()]);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->fitrange_up, 0.6);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->mu1, 0.294864);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->sigma1l, 0.010000);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->sigma1r, 0.084328);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->mu2, 0.275755);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->sigma2, 0.059802);
    // EXPECT_EQ(pdf_antimu->pdfParams(idx_p, idx_th)->fraction, 0.429674);
    // likelihoods_plus[Const::muon.getIndex()] = pdf_antimu->pdf(eop_mu, idx_p, idx_th);

    // // pi+
    // ECL::ECLPionPdf* pdf_pi = dynamic_cast<ECL::ECLPionPdf*>(eclPdfs[0][Const::pion.getIndex()]);
    // EXPECT_EQ(pdf_pi->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_pi->pdfParams(idx_p, idx_th)->fitrange_up, 1.2);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->mu1, 0.293361);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.027424);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.200000);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->fraction, 0.522942);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->mu2, 0.261009);
    // EXPECT_EQ(pdf_pi->pdfParamsMu(idx_p, idx_th)->sigma2, 0.084992);
    // EXPECT_EQ(pdf_pi->pdfParams(idx_p, idx_th)->mu3, 0.693801);
    // EXPECT_EQ(pdf_pi->pdfParams(idx_p, idx_th)->sigma3, 0.156337);
    // EXPECT_EQ(pdf_pi->pdfParams(idx_p, idx_th)->fraction, 0.864891);
    // likelihoods_plus[Const::pion.getIndex()] = pdf_pi->pdf(eop, idx_p, idx_th);

    // // pi-
    // ECL::ECLPionPdf* pdf_antipi = dynamic_cast<ECL::ECLPionPdf*>(eclPdfs[1][Const::pion.getIndex()]);
    // EXPECT_EQ(pdf_antipi->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_antipi->pdfParams(idx_p, idx_th)->fitrange_up, 1.2);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->mu1, 0.307294);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.047530);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.037448);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->fraction, 0.253802);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->mu2, 0.330000);
    // EXPECT_EQ(pdf_antipi->pdfParamsMu(idx_p, idx_th)->sigma2, 0.109461);
    // EXPECT_EQ(pdf_antipi->pdfParams(idx_p, idx_th)->mu3, 0.573200);
    // EXPECT_EQ(pdf_antipi->pdfParams(idx_p, idx_th)->sigma3, 0.183572);
    // EXPECT_EQ(pdf_antipi->pdfParams(idx_p, idx_th)->fraction, 0.574530);
    // likelihoods_minus[Const::pion.getIndex()] = pdf_antipi->pdf(eop, idx_p, idx_th);

    // // K+
    // ECL::ECLKaonPdf* pdf_K = dynamic_cast<ECL::ECLKaonPdf*>(eclPdfs[0][Const::kaon.getIndex()]);
    // EXPECT_EQ(pdf_K->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_K->pdfParams(idx_p, idx_th)->fitrange_up, 1.4);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->mu1, 0.400000);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.070323);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.173608);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->mu2, 0.421330);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->sigma2, 0.073010);
    // EXPECT_EQ(pdf_K->pdfParamsMu(idx_p, idx_th)->fraction, 1.000000);
    // EXPECT_EQ(pdf_K->pdfParams(idx_p, idx_th)->mu3, 0.611566);
    // EXPECT_EQ(pdf_K->pdfParams(idx_p, idx_th)->sigma3, 0.259668);
    // EXPECT_EQ(pdf_K->pdfParams(idx_p, idx_th)->fraction, 0.204180);
    // likelihoods_plus[Const::kaon.getIndex()] = pdf_K->pdf(eop, idx_p, idx_th);

    // // K-
    // ECL::ECLKaonPdf* pdf_antiK = dynamic_cast<ECL::ECLKaonPdf*>(eclPdfs[1][Const::kaon.getIndex()]);
    // EXPECT_EQ(pdf_antiK->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_antiK->pdfParams(idx_p, idx_th)->fitrange_up, 1.4);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->mu1, 0.053065);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.009000);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.199997);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->fraction, 0.256048);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->mu2, 0.387611);
    // EXPECT_EQ(pdf_antiK->pdfParamsMu(idx_p, idx_th)->sigma2, 0.097769);
    // EXPECT_EQ(pdf_antiK->pdfParams(idx_p, idx_th)->mu3, 0.808512);
    // EXPECT_EQ(pdf_antiK->pdfParams(idx_p, idx_th)->sigma3, 0.196950);
    // EXPECT_EQ(pdf_antiK->pdfParams(idx_p, idx_th)->fraction, 0.245737);
    // likelihoods_minus[Const::kaon.getIndex()] = pdf_antiK->pdf(eop, idx_p, idx_th);

    // // p+
    // ECL::ECLProtonPdf* pdf_p = dynamic_cast<ECL::ECLProtonPdf*>(eclPdfs[0][Const::proton.getIndex()]);
    // EXPECT_EQ(pdf_p->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_p->pdfParams(idx_p, idx_th)->fitrange_up, 0.7);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->mu1, 0.269934);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.042444);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.001000);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->fraction, 0.565921);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->mu2, 0.196375);
    // EXPECT_EQ(pdf_p->pdfParamsMu(idx_p, idx_th)->sigma2, 0.085764);
    // EXPECT_EQ(pdf_p->pdfParams(idx_p, idx_th)->mu3, 0.319720);
    // EXPECT_EQ(pdf_p->pdfParams(idx_p, idx_th)->sigma3, 0.029991);
    // EXPECT_EQ(pdf_p->pdfParams(idx_p, idx_th)->fraction, 0.509146);
    // likelihoods_plus[Const::proton.getIndex()] = pdf_p->pdf(eop, idx_p, idx_th);

    // // p-
    // ECL::ECLProtonPdf* pdf_antip = dynamic_cast<ECL::ECLProtonPdf*>(eclPdfs[1][Const::proton.getIndex()]);
    // EXPECT_EQ(pdf_antip->pdfParams(idx_p, idx_th)->fitrange_dn, 0.0);
    // EXPECT_EQ(pdf_antip->pdfParams(idx_p, idx_th)->fitrange_up, 2.4);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->mu1, 0.050056);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->sigma1l, 0.001000);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->sigma1r, 0.157782);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->fraction, 0.344824);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->mu2, 0.400000);
    // EXPECT_EQ(pdf_antip->pdfParamsMu(idx_p, idx_th)->sigma2, 0.120000);
    // EXPECT_EQ(pdf_antip->pdfParams(idx_p, idx_th)->mu3, 1.470712);
    // EXPECT_EQ(pdf_antip->pdfParams(idx_p, idx_th)->sigma3, 0.581322);
    // EXPECT_EQ(pdf_antip->pdfParams(idx_p, idx_th)->fraction, 0.049152);
    // likelihoods_minus[Const::proton.getIndex()] = pdf_antip->pdf(eop, idx_p, idx_th);

    // // Set the ECL likelihood dataobjects

    // StoreArray<ECLPidLikelihood> ecl_likelihoods_minus;
    // const auto* lk_minus = ecl_likelihoods_minus.appendNew(likelihoods_minus);

    // EXPECT_NEAR(lk_minus->getLogLikelihood(Const::electron), 0.210833, 0.01); // e-
    // EXPECT_NEAR(lk_minus->getLogLikelihood(Const::muon), 6.329390, 0.01); // mu-
    // EXPECT_NEAR(lk_minus->getLogLikelihood(Const::pion), 0.487489, 0.01); // pi-
    // EXPECT_NEAR(lk_minus->getLogLikelihood(Const::kaon), 1.515832, 0.01); // K-
    // EXPECT_NEAR(lk_minus->getLogLikelihood(Const::proton), 0.344486, 0.01); // p-

    // StoreArray<ECLPidLikelihood> ecl_likelihoods_plus;
    // const auto* lk_plus = ecl_likelihoods_plus.appendNew(likelihoods_plus);

    // EXPECT_NEAR(lk_plus->getLogLikelihood(Const::electron), 0.202594, 0.01); // e+
    // EXPECT_NEAR(lk_plus->getLogLikelihood(Const::muon), 7.133166, 0.01); // mu+
    // EXPECT_NEAR(lk_plus->getLogLikelihood(Const::pion), 0.376061, 0.01); // pi+
    // EXPECT_NEAR(lk_plus->getLogLikelihood(Const::kaon), 1.058059, 0.01); // K+
    // EXPECT_NEAR(lk_plus->getLogLikelihood(Const::proton), 0, 0.01); // p+

  } // Testcases for ECL PDFs parameters and normalisation.

}  // namespace
