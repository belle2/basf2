/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef ECLBKGExpertModule_H
#define ECLBKGExpertModule_H

#include <framework/utilities/FileSystem.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <string>

namespace Belle2 {

  /** Module to perform the ECL background KlId classification. This module only classifies ECL clusters.
   * */
  class ECLBKGExpertModule : public Module {

  public:

    /** Constructor */
    ECLBKGExpertModule();

    /** destructor */
    virtual ~ECLBKGExpertModule();

    /** init */
    virtual void initialize();

    /** beginn run */
    virtual void beginRun();

    /** process event */
    virtual void event();

  protected:

  private:

    /**
     * Initialize mva expert, dataset and features
     * Called everytime the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);


    /** measured energy */
    float m_ECLE;
    /** energy of 9/25 chrystall rings (E dispersion shape) */
    float m_ECLE9oE25;
    /** timing of ECL */
    float m_ECLTiming;
    /** uncertainty on E measurement in ECL */
    float m_ECLEerror;
    /** more sophisticated distaqnce to track in ECL, might be removed */
    float m_ECLminTrkDistance; // new
    /** disatance between track entrace into cluster and cluster center */
    float m_ECLdeltaL; // new
    /** distance cluster to next track in ECL */
    float m_ECLtrackDist;

    /** number of variables. used to resize the variable vector.  */
    float m_nVars = 7;
    /** vars to be classified */
    std::vector<float> m_feature_variables;
    /** true means use signal fraction from training  */
    float m_signal_fraction_override;

    /** mva identifier. no ending means its loaded from the database  */
    std::string m_identifier = FileSystem::findFile(
                                 "reconstruction/data/weights/ECLBKGExpert.xml");
    /**< Database pointer to the Database representation of the weightfile */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation;
    /**< Pointer to the current MVA Expert */
    std::unique_ptr<MVA::Expert> m_expert;
    /**< Pointer to the current dataset */
    std::unique_ptr<MVA::SingleDataset> m_dataset;


  }; // end class
} // end namespace Belle2

#endif

