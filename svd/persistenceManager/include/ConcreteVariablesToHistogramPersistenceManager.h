#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

namespace Belle2::SVD {
  class ConcreteVariablesToHistogramPersistenceManager : public PersistenceManager {
  public:
    ConcreteVariablesToHistogramPersistenceManager();

    void initialize(const std::string&, const std::string&, const Variables::ComputableVariables&) override;
    void addEntry(const Variables::EvaluatedVariables&) override;
    void store() override;
  };
}