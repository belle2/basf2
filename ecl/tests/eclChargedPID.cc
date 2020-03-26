#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>

#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dbobjects/ECLChargedPidPDFs.h>

#include <utility>
#include <gtest/gtest.h>

#include <TF1.h>

namespace Belle2 {

  /** Test the ECL charged PID. */
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

    /** Clear datastore */
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
    lk->setLogLikelihood(Const::muon, 0.58);
    lk->setLogLikelihood(Const::pion, 0.28);
    lk->setLogLikelihood(Const::kaon, 0.38);
    lk->setLogLikelihood(Const::proton, 0.48);

    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::electron), 0.12);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::muon), 0.58);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::pion), 0.28);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::kaon), 0.38);
    EXPECT_FLOAT_EQ(lk->getLogLikelihood(Const::proton), 0.48);

  } // Testcases for ECLPidLikelihood setters and getters.

  /** Test ECL PDFs. Check separately +/- charge hypotheses. */
  TEST_F(ECLChargedPIDTest, TestECLPdfs)
  {

    ECLChargedPidPDFs eclPdfs;

    float pmin_vals[]     = {0.2, 0.6, 1.0, 100.0};
    float thetamin_vals[] = {0.21, 0.56, 2.24, 2.70};

    TH2F histgrid("binsgrid",
                  "bins grid",
                  sizeof(thetamin_vals) / sizeof(float) - 1,
                  thetamin_vals,
                  sizeof(pmin_vals) / sizeof(float) - 1,
                  pmin_vals);

    eclPdfs.setEnergyUnit(Unit::GeV);
    eclPdfs.setAngularUnit(Unit::rad);

    std::vector<int> pdgIds = {11, -11, 13, -13, 211, -211, 321, -321, 2212, -2212, 1000010020, -1000010020};

    eclPdfs.setPdfCategories(&histgrid);

    // Create a set of fictitious (normalised) PDFs of E/p=x, for various signed particle hypotheses.
    TF1 pdf_el("pdf_el", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.2);
    TF1 pdf_elanti("pdf_elanti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.2);

    TF1 pdf_mu("pdf_mu", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.0);
    TF1 pdf_muanti("pdf_muanti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.0);

    TF1 pdf_pi("pdf_pi", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.2);
    TF1 pdf_pianti("pdf_pianti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 2.0);

    TF1 pdf_k("pdf_k", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.2);
    TF1 pdf_kanti("pdf_kanti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 2.0);

    TF1 pdf_p("pdf_p", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.6);
    TF1 pdf_panti("pdf_panti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 2.0);

    TF1 pdf_d("pdf_d", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.6);
    TF1 pdf_danti("pdf_danti", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 2.0);

    std::map<int, TF1*> pdfs = {
      {11, &pdf_el},
      { -11, &pdf_elanti},
      {13, &pdf_mu},
      { -13, &pdf_muanti},
      {211, &pdf_pi},
      { -211, &pdf_pianti},
      {321, &pdf_k},
      { -321, &pdf_kanti},
      {2212, &pdf_p},
      { -2212, &pdf_panti},
      {1000010020, &pdf_d},
      { -1000010020, &pdf_danti}
    };

    // Store the value of the PDFs in here, and pass it to the ECLPidLikelihood object later on.
    float likelihoods_plus[Const::ChargedStable::c_SetSize];
    float likelihoods_minus[Const::ChargedStable::c_SetSize];

    // Choose an arbitrary set of (clusterTheta, p), and of E/p.
    double theta = 1.047; // (X) --> theta [rad] = 60 [deg]
    double p     = 0.85;  // (Y) --> P [GeV] = 850 [MeV]
    double eop   = 0.87;

    std::vector<ECLChargedPidPDFs::InputVar> varids = {ECLChargedPidPDFs::InputVar::c_EoP};

    // Fill the DB PDF map.
    for (const auto& pdf : pdfs) {
      for (int ip(1); ip <= histgrid.GetNbinsY(); ++ip) {
        for (int jth(1); jth <= histgrid.GetNbinsX(); ++jth) {
          for (const auto& varid : varids) {
            eclPdfs.add(pdf.first, ip, jth, varid, pdf.second);
          }
        }
      }
    }

    for (const auto& pdf : pdfs) {
      float pdfval = eclPdfs.getPdf(pdf.first, p, theta, varids.at(0))->Eval(eop);
      EXPECT_NEAR(pdf.second->Eval(eop), pdfval, 0.001);
      int abspdgId = abs(pdf.first);
      if (pdf.first < 0) {
        likelihoods_minus[Const::chargedStableSet.find(abspdgId).getIndex()] = log(pdfval);
      } else {
        likelihoods_plus[Const::chargedStableSet.find(abspdgId).getIndex()] = log(pdfval);
      }
    }

    // Set the ECL likelihood dataobjects
    StoreArray<ECLPidLikelihood> ecl_likelihoods_minus;
    const auto* lk_minus = ecl_likelihoods_minus.appendNew(likelihoods_minus);
    StoreArray<ECLPidLikelihood> ecl_likelihoods_plus;
    const auto* lk_plus = ecl_likelihoods_plus.appendNew(likelihoods_plus);

    for (const auto& pdf : pdfs) {
      float logl(-700);
      float logl_expect = log(pdf.second->Eval(eop));
      if (pdf.first < 0) {
        logl = lk_minus->getLogLikelihood(Const::chargedStableSet.find(abs(pdf.first)));
      } else {
        logl = lk_plus->getLogLikelihood(Const::chargedStableSet.find(abs(pdf.first)));
      }
      EXPECT_NEAR(logl_expect, logl, 0.01);
    }

  } // Testcases for ECL PDFs.

}  // namespace
