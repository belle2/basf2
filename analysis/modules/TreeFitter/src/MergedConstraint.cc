#include <framework/logging/Logger.h>
//
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/MergedConstraint.h>

namespace TreeFitter {

  ErrCode MergedConstraint::project(const FitParams& fitpar, Projection& p) const
  {
    B2DEBUG(80, "MergedConstraint:project()");
    ErrCode status ;
    for (constraintlist::const_iterator it = m_list.begin() ;
         it != m_list.end() ; ++it) {
      status |= (*it)->project(fitpar, p) ;
      p.incrementOffset((*it)->dim()) ;
    }
    return status ;
  }

  void MergedConstraint::print(std::ostream& os) const
  {
    os << "Merged constraint: " << std::endl ;
    for (constraintlist::const_iterator it = m_list.begin() ;
         it != m_list.end() ; ++it) {
      os << "          " ;
      (*it)->print(os) ;
    }
  }

}

