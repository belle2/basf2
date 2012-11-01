#ifndef PARTICLE_CLASS_MOMENTUM_H
#define PARTICLE_CLASS_MOMENTUM_H

#include <string>

#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Matrix.h"

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <tracking/dataobjects/Track.h>
#include <generators/dataobjects/MCParticle.h>
#include <cmath>

//#include "helix/Helix.h"
#include "analysis/particle/constant.h"
#include "analysis/particle/Ptype.h"

using namespace CLHEP;

/// Mometum class supplies you interfaces for various values of particle, such as its momentum, position, and vertex info. etc..
namespace Belle2 {
  class Momentum {

  public:// Constructor
    /// Default constructor
    Momentum();
    /// Copy constructor
    Momentum(const Momentum&);
    /// Constructor with momentum
    Momentum(const HepLorentzVector&,
             const HepSymMatrix& error = HepSymMatrix(4, 0));
    /// Constructor with Mdst\_charged
    Momentum(const Track&, const double mass,
             const Hep3Vector & = Hep3Vector(0., 0., 0.));
    /// Constructor with Mdst\_charged
    Momentum(const Track&, const Ptype&,
             const Hep3Vector & = Hep3Vector(0., 0., 0.));
    /// Constructor with Mdst\_ecl
    Momentum(const ECLShower&);
    /// Constructor with Mdst\_gamma
    Momentum(const ECLGamma&);
    /// Constructor with Mdst\_pi0
    Momentum(const ECLPi0&);

    /**
     * Construct Momentum from a MCParticle. The 4-momentum vector,
     * position and production vertex are filled with generated values.
     */
    Momentum(const MCParticle*);

    /// Destructor
    virtual ~Momentum() {};

  public:// General interfaces
    /// returns class name.
    virtual std::string className(void) { return std::string("Momentum"); }
    /// dumps debug information. (not implement.)
    virtual void dump(const std::string& keyword = std::string("mass momentum return"),
                      const std::string& prefix  = std::string("")) const;

  public:// Selectors
    /// returns momentum vector.
    virtual const HepLorentzVector& p(void) const { return m_momentum; }

    /// returns error matrix(4x4) of momentum vector. (not reference)
    virtual const HepSymMatrix dp(void) const { return m_error.sub(1, 4); }

    /// returns position vector.
    virtual const Hep3Vector& x(void) const { return m_position; }

    /// returns error matrix(3x3) of position vector. (not reference)
    virtual const HepSymMatrix dx(void) const { return m_error.sub(5, 7); }

    /// returns error matrix of momentum and position vector.
    virtual const HepSymMatrix& dpx(void) const { return m_error; }

    /// returns mass.
    virtual double mass(void) const { return m_momentum.mag(); }

    /// returns error of mass.
    virtual double dMass(void) const;

    /// retruns production vertex.
    virtual const Hep3Vector& vertex(void) const { return m_vertex; }

    /// retruns error matrix(3x3) of production vertex.
    virtual const HepSymMatrix& dVertex(void) const { return m_vertexError; }

    /// retruns decay vertex.
    virtual const Hep3Vector& decayVertex(void) const { return m_decayVertex; }

    /// retruns error matrix(3x3) of decay vertex.
    virtual const HepSymMatrix& dDecayVertex(void) const { return m_decayVertexError; }

  public:// Modifiers
    /// sets momentum vector and its error matrix(4x4).
    virtual void momentum(const HepLorentzVector&,
                          const HepSymMatrix&     error = HepSymMatrix(4, 0));

    /// sets position vector and its error matrix(3x3).
    virtual void position(const Hep3Vector&,
                          const HepSymMatrix& error = HepSymMatrix(3, 0));

    /// sets momentum and position vector and its error matrix(7x7).
    virtual void momentumPosition(const HepLorentzVector&, const Hep3Vector&,
                                  const HepSymMatrix& error = HepSymMatrix(7, 0));

    /// sets production vertex and its error matrix(3x3).
    virtual Hep3Vector& vertex(const Hep3Vector&,
                               const HepSymMatrix& error = HepSymMatrix(3, 0));

    /// sets decay vertex and its error matrix(3x3).
    virtual Hep3Vector& decayVertex(const Hep3Vector&,
                                    const HepSymMatrix& error = HepSymMatrix(3, 0));

  public:// Operators
    /// copy operator.
    Momentum& operator = (const Momentum&);

    //protected:// Protected members
  private:// Private members
    HepLorentzVector m_momentum;       // 4
    Hep3Vector       m_position;       // 3
    HepSymMatrix     m_error;          // 7 x 7

    Hep3Vector       m_vertex;         // 3      ... production vertex
    HepSymMatrix     m_vertexError;    // 3 x 3  ... production vertex
    Hep3Vector       m_decayVertex;         // 3
    HepSymMatrix     m_decayVertexError;    // 3 x 3
  };
}
#endif /* PARTICLE_CLASS_MOMENTUM_H */
