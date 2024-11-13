/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <svd/persistenceManager/PersistenceManager.h>

namespace Belle2 {
  class SVDValidationModule : public Module {
  public:
    SVDValidationModule();

    virtual ~SVDValidationModule() = default;

    virtual void initialize() override;

    virtual void event() override;

    virtual void terminate() override;

  private:
    std::unique_ptr<PersistenceManager> persistenceManager;

    std::string m_storageType;
  };
}
