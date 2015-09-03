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
#pragma read sourceClass="Belle2::PIDLikelihood" version="[-2]" \
  source="float m_logl[6][5]" \
  targetClass="Belle2::PIDLikelihood" target="m_logl" \
  code="{ \
    const static int oldToNewIndex[] = {2, 3, 4, 0, 1}; \
    for (int iDet = 0; iDet < 6; iDet++) { \
      for (int iPDG = 0; iPDG < 5; iPDG++) { \
        int newIndex = oldToNewIndex[iPDG]; \
        m_logl[iDet][newIndex] = onfile.m_logl[iDet][iPDG]; \
      } \
      /* use proton likelihood for deuterons. */ \
      m_logl[iDet][5] = onfile.m_logl[iDet][2]; \
    }\
  }"

// ----------------------------------------------------------------------------
// TrackFitResult evolution
// As of version 5 the CDC hit pattern was merged into a single variable.
#pragma read sourceClass="Belle2::TrackFitResult" version="[4]" \
  source="const uint32_t m_hitPatternCDCInitializer; const uint32_t m_hitPatternCDCInitializer_part2" \
  targetClass="Belle2::TrackFitResult" target="m_hitPatternCDCInitializer" \
  code="{ \
		m_hitPatternCDCInitializer = ((long long int) onfile.m_hitPatternCDCInitializer_part2 << 32) | onfile.m_hitPatternCDCInitializer; \
	}"

// As of version 6 the data is no longer kept in vectors, but in fixed-size arrays.
#pragma read sourceClass="Belle2::TrackFitResult" version="[-5]" \
  source="std::vector<float> m_tau" \
  targetClass="Belle2::TrackFitResult" target="m_tau" \
  code = "{ \
            memset(m_tau, 0, 5*sizeof(Double32_t)); \
            for (size_t i = 0; i < std::min(onfile.m_tau.size(), (size_t)5); ++i) \
               m_tau[i] = onfile.m_tau[i]; \
          }"
#pragma read sourceClass="Belle2::TrackFitResult" version="[-5]" \
  source="std::vector<float> m_cov5" \
  targetClass="Belle2::TrackFitResult" target="m_cov5" \
  code = "{ \
            memset(m_cov5, 0, 15*sizeof(Double32_t)); \
            for (size_t i = 0; i < std::min(onfile.m_cov5.size(), (size_t)15); ++i) \
               m_cov5[i] = onfile.m_cov5[i]; \
          }"


#endif
