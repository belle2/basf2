/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <framework/core/Module.h>




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



    int m_event_nr = 0;

    struct geometry_data;
    std::shared_ptr<geometry_data>  m_geometry;
    std::vector<int> m_layerUsed;
    std::string m_dummy_used_layers;
    std::string m_geometry_fileName;

  };
} // namespace Belle2
