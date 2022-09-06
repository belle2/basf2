/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <mdst/dataobjects/MCParticle.h>

#include <string>

namespace Belle2 {

  /** Creates a new flavorTaggerInfoMap DataObject for the specific methods. Saves there all the relevant information of the
    flavorTagger:
    -The pointer to the track with highest target probability in Track Level for each category
    -The highest target track probability in Track Level for each category
    -The pointer to the track with highest category probability in Event Level
    -The highest category probability in Event Level for each category
    -The qr Output of each category, i.e. the Combiner input values. They could be used for independent tags.
    -qr Output of the Combiner.
    -Direct Output of the FBDT Combiner: Combined probability of being a B0.
    -Direct Output of the FANN Combiner: Combined probability of being a B0. */

  class FlavorTaggerInfoFillerModule : public Module {
  private:

    StoreObjPtr<EventExtraInfo> m_eventExtraInfo; /**< event extra info object pointer */

    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticles */

    StoreObjPtr<RestOfEvent> m_roe; /**< ROE object pointer */

    /** Used Flavor Tagger trackLevel Categories of the lists */
    std::vector<std::tuple<std::string, std::string>> m_trackLevelParticleLists;

    /** Used Flavor Tagger eventLevel Categories of the lists */
    std::vector<std::tuple<std::string, std::string, std::string>> m_eventLevelParticleLists;

    bool m_FANNmlp; /**< Sets if FANN Combiner output will be saved or not **/

    bool m_TMVAfbdt; /**< Sets if FastBDT Combiner output will be saved or not **/

    bool m_DNNmlp; /**< Sets if DNN tagger output will be saved or not **/

    bool m_qpCategories; /**< Sets if individual Categories output will be saved or not **/

    bool m_targetProb; /**< Sets if individual Categories output will be saved or not **/

    bool m_trackPointers; /**< Sets if track pointers to target tracks are saved or not **/

    bool m_istrueCategories;  /**< Sets if individual MC thruth for each Category is saved or not **/

  public:
    /** Constructor. */
    FlavorTaggerInfoFillerModule();

    /** Initialises the module. */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2


