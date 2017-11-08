#pragma once
#include <framework/geometry/BFieldManager.h>

#include <TEveTrackPropagator.h>

namespace Belle2 {
  /** Provide magnetic field values for TEveTrackPropagator. */
  class EveVisBField : public TEveMagField {
  public:
    EveVisBField(): TEveMagField() { }
    virtual ~EveVisBField() { }

    /** return field strength at given coordinates, using Eve conventions. */
    virtual TEveVector GetField(Float_t x, Float_t y, Float_t z) const override
    {
      TEveVector v;

      v.Set(BFieldManager::getField(x, y, z) / Unit::T);
      v.fZ *= -1; //Eve has inverted B field convention
      v.fZ -= 1e-6; //field must not be zero!

      return v;
    }
    /** maximal field strength (is this correct?) */
    virtual Float_t GetMaxFieldMag() const override { return 1.5; }
  };
}
