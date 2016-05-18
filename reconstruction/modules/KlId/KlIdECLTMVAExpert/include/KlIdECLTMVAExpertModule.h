/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef KlIdECLTMVAExpertModule_H
#define KlIdECLTMVAExpertModule_H

#include <framework/utilities/FileSystem.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>


#include <string>

#include <TMVA/Reader.h>

namespace Belle2 {

  class KlIdECLTMVAExpertModule : public Module {

  public:

    KlIdECLTMVAExpertModule();

    virtual ~KlIdECLTMVAExpertModule();

    /** init */
    virtual void initialize();

    /** beginn run */
    virtual void beginRun();

    /** process event */
    virtual void event();

    /** end run */
    virtual void endRun();

    /** terminate */
    virtual void terminate();

  protected:

  private:


    /** varibales to classify */

    /** measured energy */
    Float_t m_ECLE;
    /** energy of 9/25 chrystall rings (E dispersion shape) */
    Float_t m_ECLE9oE25;
    /** timing of ECL */
    Float_t m_ECLTiming;
    /** uncertainty on E measurement in ECL */
    Float_t m_ECLEerror;
    /** more sophisticated distaqnce to track in ECL, might be removed */
    Float_t m_ECLminTrkDistance; // new
    /** disatance between track entrace into cluster and cluster center */
    Float_t m_ECLdeltaL; // new
    /** distance cluster to next track in ECL */
    Float_t m_ECLtrackDist;
    /** classifier output from bkg classification of ECL cluster */
    Float_t m_ECLBKGProb;


    /** default classifier name ECL ID */
    std::string m_classifierName = "ECLBDT";
    /** default classifier name BKG classifier*/
    std::string m_BKGClassifierName = "ECLBKGClassifierBDT";

    /** TMVA classifier object. */
    TMVA::Reader* m_reader = new TMVA::Reader("Verbose");
    /** TMVA classifier object. */
    TMVA::Reader* m_readerBKG = new TMVA::Reader("Verbose");

    /** path to training .xml file. */
    std::string m_classifierPath = FileSystem::findFile(
                                     "reconstruction/data/weights/TMVAFactory_ECLBDT.weights.xml");

    /** path to training .xml file. */
    std::string m_BKGClassifierPath = FileSystem::findFile(
                                        "reconstruction/data/weights/TMVAFactory_ECLBKGClassifierBDT.weights.xml");


  }; // end class
} // end namespace Belle2

#endif

