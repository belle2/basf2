/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef ECLExpertModule_H
#define ECLExpertModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KlId.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <string>


namespace Belle2 {


  /** Module to perform the ECL KlId classification. This module only classifies ECL clusters.
   * The output is a KlId object on the datastore. It contains KlId, bkgProb and wheter its an ECL or ECL cluster */
  class ECLExpertModule : public Module {

  public:

    /** Constructor */
    ECLExpertModule();

    /** Destructor */
    virtual ~ECLExpertModule();

    /** init */
    virtual void initialize() override;

    /** beginn run */
    virtual void beginRun() override;

    /** process event */
    virtual void event() override;

    /** terminate and free */
    virtual void terminate() override
    {
      m_expert.reset();
      m_dataset.reset();
    }

  private:

    /**
     * Initialize mva expert, dataset and features
     * Called everytime the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);


    /** Distance between cluster center and track extrapolation to ECL. */
    Double32_t  m_ECLminTrkDistance;  //[0.0, 250., 10]

    /** Zernike 40. */
    Double32_t  m_ECLZ40;  //[0.0, 1.7, 10]

    /** Zernike 51. */
    Double32_t  m_ECLZ51;  //[0.0, 1.2, 10]

    /** E1oE9. */
    Double32_t  m_ECLE1oE9;  //[0.0, 1., 10]

    /** E9oE21. */
    Double32_t  m_ECLE9oE21;  //[0.0, 1., 10]

    /** Second Moment. */
    Double32_t  m_ECLsecondMoment;  //[0.0, 40.0, 10]

    /** LAT. */
    Double32_t  m_ECLLAT;  //[0.0, 1., 10]

    /** Number of Crystals in a shower (sum of weights). */
    Double32_t  m_ECLnumberOfCrystals;  //[0.0, 200.0, 10]

    /** Time. */
    Double32_t  m_ECLtime;  //[-1000.0, 1000.0, 12]

    /** Delta Time 99. */
    Double32_t  m_ECLdeltaTime99;  //[0.0, 1000.0, 12]

    /** Theta [rad]. */
    Double32_t  m_ECLtheta;  //[0.0, pi, 16]

    /** Phi [rad]. */
    Double32_t  m_ECLphi;  //[-pi, pi, 16]

    /** Radius [cm]. */
    Double32_t  m_ECLr;  //[75.0, 300.0, 16]

    /** DeltaL  */
    Double32_t  m_ECLDeltaL;  //[75.0, 300.0, 16]

    /** MVA classifier that uses pulse shape discrimination to identify electromagnetic vs hadronic showers. Classifier value is 1.0 EM showers and 0.0 for hadronic showers. */
    Double32_t  m_ECLPulseShapeDiscriminationMVA;  //[0.0, 1.0, 18]

    /** Cluster Hadron Component Intensity (pulse shape discrimination variable). Sum of the CsI(Tl) hadron scintillation component emission normalized to the sum of CsI(Tl) total scintillation emission.  Computed only using cluster digits with energy greater than 50 MeV and good offline waveform fit chi2. Will be removed in release-04*/
    //    Double32_t  m_ECLClusterHadronIntensity;  //[-0.1, 0.8, 18]

    /** Number of hadron digits in cluster (pulse shape discrimination variable).  Weighted sum of digits in cluster with significant scintillation emission (> 3 MeV) in the hadronic scintillation component.*/
    Double32_t m_ECLNumberOfHadronDigits;  //[0, 255, 18]

    /** output of a BDT fitted on various Z-moments for the closest ECL cluster */
    float m_ECLZMVA;

    /**< Energy [GeV]. */
    Double32_t  m_ECLEnergy;  //[-5, 3., 18]

    /**< Log. Energy [GeV]. */
    Double32_t  m_ECLlogEnergy;  //[-5, 3., 18]

    /** Log. Highest Crystal Energy [GeV]. */
    Double32_t  m_ECLlogEnergyHighestCrystal;  //[-5, 3., 18]

    /** storearray */
    StoreArray<ECLCluster> m_eclClusters;
    /** storearray */
    StoreArray<KlId> m_klids;

    /** vars to be classified */
    std::vector<float> m_feature_variables;

    /** mva identifier. no ending means its loaded from the database  */
    std::string m_identifier =
      "ECL_fBDT_1xBkgPhiGamma.xml";
    //      "ECL_fBDT_1xBkgpGunMulti.xml";
    //      "ECL_fBDT_test_mixedBkg50kv1.xml";
    //      "ECL_fBDT_mixedBkg10k"; // assuming 1 times beambkg and generic BBbar events

    /** Database pointer to the Database representation of the weightfile */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation;
    /** Pointer to the current MVA Expert */
    std::unique_ptr<MVA::Expert> m_expert;
    /** Pointer to the current dataset */
    std::unique_ptr<MVA::SingleDataset> m_dataset;


  }; // end class
} // end namespace Belle2


#endif
