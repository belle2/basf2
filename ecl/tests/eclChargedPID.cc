/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

    eclPdfs.setPdfCategories(&histgrid);

    // Create a set of fictitious (normalised) PDFs of E/p, for various signed particle hypotheses.
    // The first element in the pair is the true charge.

    typedef std::pair<int, TF1> pdfSet;

    pdfSet pdf_el = std::make_pair(-1, TF1("pdf_el", "TMath::Gaus(x, 1.0, 0.2, true)", 0.0, 1.3));
    pdfSet pdf_elanti = std::make_pair(1, TF1("pdf_elanti", "TMath::Gaus(x, 0.9, 0.2, true)", 0.0, 1.2));

    pdfSet pdf_mu = std::make_pair(-1, TF1("pdf_mu", "TMath::Gaus(x, 0.3, 0.2, true)", 0.0, 1.0));
    pdfSet pdf_muanti = std::make_pair(1, TF1("pdf_muanti", "TMath::Gaus(x, 0.35, 0.22, true)", 0.0, 1.0));

    pdfSet pdf_pi = std::make_pair(1, TF1("pdf_pi", "TMath::Gaus(x, 0.4, 0.1, true)", 0.0, 1.0));
    pdfSet pdf_pianti = std::make_pair(-1, TF1("pdf_pianti", "TMath::Gaus(x, 0.38, 0.15, true)", 0.0, 1.0));

    pdfSet pdf_k = std::make_pair(1, TF1("pdf_k", "TMath::Gaus(x, 0.38, 0.2, true)", 0.0, 1.0));
    pdfSet pdf_kanti = std::make_pair(-1, TF1("pdf_kanti", "TMath::Gaus(x, 0.5, 0.22, true)", 0.0, 1.0));

    pdfSet pdf_p = std::make_pair(1, TF1("pdf_p", "TMath::Gaus(x, 1.0, 0.4, true)", 0.0, 1.6));
    pdfSet pdf_panti = std::make_pair(-1, TF1("pdf_panti", "TMath::Gaus(x, 1.3, 0.5, true)", 0.0, 2.0));

    pdfSet pdf_d = std::make_pair(1, TF1("pdf_d", "TMath::Gaus(x, 1.1, 0.45, true)", 0.0, 1.6));
    pdfSet pdf_danti = std::make_pair(-1, TF1("pdf_danti", "TMath::Gaus(x, 1.2, 0.6, true)", 0.0, 2.0));

    std::map<unsigned int, std::vector<pdfSet>> pdfs = {
      {Const::electron.getPDGCode(), std::vector<pdfSet>{pdf_el, pdf_elanti}},
      {Const::muon.getPDGCode(), std::vector<pdfSet>{pdf_mu, pdf_muanti}},
      {Const::pion.getPDGCode(), std::vector<pdfSet>{pdf_pi, pdf_pianti}},
      {Const::kaon.getPDGCode(), std::vector<pdfSet>{pdf_k, pdf_kanti}},
      {Const::proton.getPDGCode(), std::vector<pdfSet>{pdf_p, pdf_panti}},
      {Const::deuteron.getPDGCode(), std::vector<pdfSet>{pdf_d, pdf_danti}},
    };

    // E/p is the only variable considered in the test.
    std::vector<ECLChargedPidPDFs::InputVar> varids = {ECLChargedPidPDFs::InputVar::c_EoP};

    // Fill the DB PDF map.
    // (Use the same PDF for all (clusterTheta, p) bins for simplicity).
    for (auto& [pdg, pdf_setlist] : pdfs) {
      for (auto& pdf_set : pdf_setlist) {
        for (int ip(1); ip <= histgrid.GetNbinsY(); ++ip) {
          for (int jth(1); jth <= histgrid.GetNbinsX(); ++jth) {
            for (const auto& varid : varids) {
              eclPdfs.add(pdg, pdf_set.first, ip, jth, varid, &pdf_set.second);
            }
          }
        }
      }
    }

    // Store the value of the PDFs in here, and pass it to the ECLPidLikelihood object later on.
    float likelihoods_plus[Const::ChargedStable::c_SetSize];
    float likelihoods_minus[Const::ChargedStable::c_SetSize];

    // Choose an arbitrary set of (clusterTheta, p), and of E/p.
    double clusterTheta = 1.047; // (X) --> clusterTheta [rad] = 60 [deg]
    double p     = 0.85;  // (Y) --> P [GeV] = 850 [MeV]
    double eop   = 0.87;

    // Now read the PDFs.
    for (const auto& [pdg, pdf_setlist] : pdfs) {
      for (const auto& pdf_set : pdf_setlist) {
        float pdfval = eclPdfs.getPdf(pdg, pdf_set.first, p, clusterTheta, varids.at(0))->Eval(eop);
        EXPECT_NEAR(pdf_set.second.Eval(eop), pdfval, 0.001);
        if (pdf_set.first < 0) {
          likelihoods_minus[Const::chargedStableSet.find(pdg).getIndex()] = log(pdfval);
        } else {
          likelihoods_plus[Const::chargedStableSet.find(pdg).getIndex()] = log(pdfval);
        }
      }
    }

    // Set the ECL likelihood dataobjects
    StoreArray<ECLPidLikelihood> ecl_likelihoods_minus;
    const auto* lk_minus = ecl_likelihoods_minus.appendNew(likelihoods_minus);
    StoreArray<ECLPidLikelihood> ecl_likelihoods_plus;
    const auto* lk_plus = ecl_likelihoods_plus.appendNew(likelihoods_plus);

    for (const auto& [pdg, pdf_setlist] : pdfs) {
      for (const auto& pdf_set : pdf_setlist) {
        float logl;
        float logl_expect = log(pdf_set.second.Eval(eop));
        if (pdf_set.first < 0) {
          logl = lk_minus->getLogLikelihood(Const::chargedStableSet.find(pdg));
        } else {
          logl = lk_plus->getLogLikelihood(Const::chargedStableSet.find(pdg));
        }
        EXPECT_NEAR(logl_expect, logl, 0.01);
      }
    }

  } // Testcases for ECL PDFs.

}  // namespace
