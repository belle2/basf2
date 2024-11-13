#pragma once

class PersistenceManager {
public:
  virtual void initialize() = 0;
  virtual void addEntry() = 0;
  virtual void store() = 0;
  virtual ~PersistenceManager() = default;
};
