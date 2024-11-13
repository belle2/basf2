#pragma once

#include <svd/persistenceManager/PersistenceManager.h>

class ConcreteVariablesToNtuplePersistenceManager : public PersistenceManager {
public:
  void initialize() override;
  void addEntry() override;
  void store() override;
};
