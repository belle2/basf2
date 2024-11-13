#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

class ConcreteVariablesToHistogramPersistenceManager : public PersistenceManager {
  void initialize() override;
  void addEntry() override;
  void store() override;
};
