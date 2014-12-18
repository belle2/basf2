/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::MCParticle+;
#pragma link C++ class vector<Belle2::MCParticle*>+;
#pragma link C++ class Belle2::PIDLikelihood+;
#pragma link C++ class Belle2::ECLCluster+;

#pragma link C++ class Belle2::KLMCluster+;

#pragma link C++ class Belle2::Track+;
#pragma link C++ class Belle2::HitPatternCDC+;
#pragma link C++ class Belle2::HitPatternVXD+;
#pragma link C++ class Belle2::TrackFitResult+;
#pragma link C++ class Belle2::V0+;
#pragma link C++ class pair<Belle2::TrackFitResult*, Belle2::TrackFitResult*>+;
#pragma link C++ class pair<Belle2::Track*, Belle2::Track*>+;
#pragma link C++ class pair<short, short>+;

#pragma link C++ class Belle2::TRGSummary+;

#pragma link C++ class Belle2::HLTTag;

// Allow reading PIDLikelihood version <=2 (less particle types, different order)
//
// schema evolution rule as described in "Support For Significant Evolutions of the User Data Model In ROOT Files"
// Ph Canal et al 2010 J. Phys.: Conf. Ser. 219
//
// Note: target must be empty, otherwise the rule isn't applied in this case. Probably some bug there?
//       One can still access the fresh object via 'newObj', though.
#pragma read sourceClass="Belle2::PIDLikelihood" version="[-2]" \
  source="float m_logl[6][5]" \
  targetClass="Belle2::PIDLikelihood" target="" \
  code="{ \
    const static int oldToNewIndex[] = {2, 3, 4, 0, 1}; \
    for (int iDet = 0; iDet < 6; iDet++) { \
      for (int iPDG = 0; iPDG < 5; iPDG++) { \
        int newIndex = oldToNewIndex[iPDG]; \
        newObj->m_logl[iDet][newIndex] = onfile.m_logl[iDet][iPDG]; \
      } \
      /* use proton likelihood for deuterons. */ \
      newObj->m_logl[iDet][5] = onfile.m_logl[iDet][2]; \
    }\
  }"

#endif
