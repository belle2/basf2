#pragma once

#include <TEveElement.h>

class TEveCaloData;

namespace Belle2 {
  /** handles selection of individual ECLClusters (getting them out of TEve is hard). */
  class EveTower : public TEveElement, public TObject {
  public:
    /** Construct from given ECLCluster representation. (id = AddTower() return value) */
    EveTower(TEveCaloData* calodata, int towerID): TEveElement(), TObject(), m_caloData(calodata), m_id(towerID) { }

    /** returns tower ID. */
    int getID() const { return m_id; }

    /** Overrides TEveElement::SelectElement(), performs magic to select a single cluster. */
    virtual void  SelectElement(Bool_t state);

  private:
    TEveCaloData* m_caloData; /**< Calo data object containing the tower. */
    int m_id; /**< id of tower. */

    ClassDef(EveTower, 0); /**< handles selection of individual ECLClusters (getting them out of TEve is hard). */

  };
}
