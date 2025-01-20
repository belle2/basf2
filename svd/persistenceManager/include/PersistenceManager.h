#pragma once

#include <string>
#include <vector>

#include <svd/persistenceManager/Types.h>

namespace Belle2::SVD {
  class PersistenceManager {
  public:
    virtual void initialize(const std::string&, const std::string&, Variables::Variables&) = 0;
    virtual void addEntry(const Variables::EvaluatedVariables&) = 0;
    virtual void store() = 0;
    virtual ~PersistenceManager() = default;
  };
}
