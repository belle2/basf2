/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dmitri Liventsev                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMTRIGGERMODULE_H
#define KLMTRIGGERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/klm/dataobjects/KLMTrgSummary.h>
namespace Belle2 {

  class KLMTriggerModule : public Module {
  public:

    // Constructor
    KLMTriggerModule();

    // Destructor
    virtual ~KLMTriggerModule() { };

    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override { };

  private: // Parameters



    int m_nLayerTrigger = 0;

    StoreObjPtr<KLMTrgSummary> m_KLMTrgSummary;

    std::vector<int> m_layerUsed;
    std::string m_dummy_used_layers;
  };
} // namespace Belle2

#endif // KLMTRIGGERMODULE_H
