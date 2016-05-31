
/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef IDValidationModule_H
#define IDValidationModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TTree.h>
#include <TFile.h>
#include <string>

namespace Belle2 {

  /** Module to write data to analyse KlId performance. */
  class IDValidationModule : public Module {

  public:

    IDValidationModule();

    virtual ~IDValidationModule();

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

    /** is KLM*/
    Float_t m_isKLM;
    /** is ECL*/
    Float_t m_isECL;
    /** is KLM*/
    Float_t m_ECLMatchedCorrectly;

    /** is signal*/
    Float_t m_isKlong;
    /** is beamBKG*/
    Float_t m_isBeamBKG;
    /** klong id*/
    Float_t m_KlId;
    /** klong id*/
    Float_t m_BKGId;
    /** pdg code */
    Float_t m_PDG;

    /** Out put path, has to be specified by the user.*/
    std::string m_outPath = "ROCData.root";
    /** root file */
    TFile* m_f = nullptr; //
    /** tree for klm data */
    TTree* m_treeKLM = nullptr;
    /** tree for ecl data */
    TTree* m_treeECL = nullptr;


  }; // end class
} // end namespaceBelle2

#endif
