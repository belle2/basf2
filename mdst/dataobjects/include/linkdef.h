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

#pragma link C++ class Belle2::Cluster+;

#pragma link C++ class Belle2::MCParticle+;
#pragma link C++ class vector<Belle2::MCParticle*>+;
#pragma link C++ class Belle2::PIDLikelihood+;
#pragma link C++ class Belle2::ECLCluster+;

#pragma link C++ class Belle2::KLMCluster+;
#pragma link C++ class Belle2::KlId+;

#pragma link C++ class Belle2::Track+;
#pragma link C++ class Belle2::HitPatternCDC+;
#pragma link C++ class Belle2::HitPatternVXD+;
#pragma link C++ class Belle2::TrackFitResult+;
#pragma link C++ class Belle2::V0+;
#pragma link C++ class pair<Belle2::TrackFitResult*, Belle2::TrackFitResult*>+;
#pragma link C++ class pair<Belle2::Track*, Belle2::Track*>+;
#pragma link C++ class pair<short, short>+;
#pragma link C++ class Belle2::EventLevelTrackingInfo+;

#pragma link C++ class Belle2::TRGSummary+;
#pragma link C++ class Belle2::SoftwareTriggerResult+;

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

#pragma read sourceClass="Belle2::TrackFitResult" version="[6]"   \
  source="Double32_t m_cov5[15]; Double32_t m_tau[5];"      \
  targetClass="Belle2::TrackFitResult"          \
  target="m_cov5"                 \
  code="{							      \
  /* Translate the false covariance matrix back to the 6x6 matrix */  \
  TMatrixDSym oldCov5(5);					      \
  {								      \
    unsigned int counter = 0;					      \
    for (unsigned int i = 0; i < 5; ++i) {			      \
      for (unsigned int j = i; j < 5; ++j) {			      \
        oldCov5(i, j) = oldCov5(j, i) = onfile.m_cov5[counter];	      \
        ++counter;							      \
        }                 \
      }                 \
    }                 \
                  \
  const double bZ = 1.5;            \
  const int iX = 0;             \
  const int iY = 1;             \
  const int iZ = 2;             \
  const int iPx = 3;              \
  const int iPy = 4;              \
  const int iPz = 5;              \
				  \
  const int iD0 = 0;             \
  const int iPhi0 = 1;             \
  const int iOmega = 2;             \
  const int iZ0 = 3;              \
  const int iTanLambda = 4;              \
                  \
  /* Transform covariance matrix */         \
  TMatrixD jacobianInflate(6, 5);         \
  jacobianInflate.Zero();           \
                  \
  const double d0 = onfile.m_tau[0];            \
  const double omega = onfile.m_tau[2];         \
  const double tanLambda = onfile.m_tau[4];         \
  const Belle2::Helix helix(d0,0, omega, 0, tanLambda);	    \
  const TVector3 position = helix.getPerigee();	\
  /* Attention! Using a hardcoded 1.5 here. */ \
  const TVector3 momentum = helix.getMomentum(bZ);          \
  const int charge = helix.getChargeSign();		      \
  							\
  const double alpha =  1.0 / (bZ * TMath::C()) * 1E11;     \
  const double absAlphaOmega = alpha * std::fabs(omega);    \
  const double signedAlphaOmega2 =  absAlphaOmega * omega;    \
                  \
  const double invAbsAlphaOmega = 1.0 / absAlphaOmega;      \
  const double invSignedAlphaOmega2 = 1.0 / signedAlphaOmega2;    \
								  \
  /* Undo the false translation back to the position and momentum uncertainties*/ \
  jacobianInflate(iX, iPhi0) = d0;       \
  jacobianInflate(iY, iD0) = charge;      \
  jacobianInflate(iZ, iZ0) = 1.0;         \
  jacobianInflate(iPx, iOmega) = invSignedAlphaOmega2;      \
  jacobianInflate(iPy, iPhi0) = invAbsAlphaOmega;     \
  jacobianInflate(iPz, iOmega) = tanLambda * invSignedAlphaOmega2;	\
  jacobianInflate(iPz, iTanLambda) = invAbsAlphaOmega;			\
									\
  TMatrixDSym cov6 = oldCov5; /* copy */				\
  cov6.Similarity(jacobianInflate);					\
									\
  const Belle2::UncertainHelix uncertainHelix(position, momentum, charge, bZ, cov6, 0.0); \
									\
  /* Upper half of the covariance matrix goes into m_cov5. */		\
  const TMatrixDSym& newCov5 = uncertainHelix.getCovariance();     \
  memset(m_cov5, 0, 15*sizeof(Double32_t));       \
  unsigned int counter = 0;           \
  for (unsigned int i = 0; i < 5; ++i) {        \
    for (unsigned int j = i; j < 5; ++j) {      \
      m_cov5[counter++] = newCov5(i, j);        \
    }                 \
  }                 \
  }"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Energy" \
  targetClass="Belle2::ECLCluster" target="m_logEnergy" \
  code="{if(onfile.m_Energy>0.0) m_logEnergy = log(onfile.m_Energy); \
         else m_logEnergy = -5.;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_HighestE" \
  targetClass="Belle2::ECLCluster" target="m_logEnergyHighestCrystal" \
  code="{if(onfile.m_HighestE>0.0) m_logEnergyHighestCrystal = log(onfile.m_HighestE); \
         else m_logEnergyHighestCrystal = -5.;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_EnedepSum" \
  targetClass="Belle2::ECLCluster" target="m_logEnergyRaw" \
  code="{if(onfile.m_EnedepSum>0.0) m_logEnergyRaw = log(onfile.m_EnedepSum); \
	else m_logEnergyRaw = -5.;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Theta" \
  targetClass="Belle2::ECLCluster" target="m_theta" \
  code="{m_theta = onfile.m_Theta;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Phi" \
  targetClass="Belle2::ECLCluster" target="m_phi" \
  code="{m_phi = onfile.m_Phi;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_R" \
  targetClass="Belle2::ECLCluster" target="m_r" \
  code="{m_r = onfile.m_R;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Timing" \
  targetClass="Belle2::ECLCluster" target="m_time" \
  code="{m_time = onfile.m_Timing;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_ErrorTiming" \
  targetClass="Belle2::ECLCluster" target="m_deltaTime99" \
  code="{m_deltaTime99 = onfile.m_ErrorTiming;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_E9oE25" \
  targetClass="Belle2::ECLCluster" target="m_E9oE21" \
  code="{m_E9oE21 = onfile.m_E9oE25;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="int m_NofCrystals" \
  targetClass="Belle2::ECLCluster" target="m_numberOfCrystals" \
  code="{m_numberOfCrystals = (double) onfile.m_NofCrystals;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="int m_CrystHealth" \
  targetClass="Belle2::ECLCluster" target="m_status" \
  code="{m_status = onfile.m_CrystHealth;}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Error[6]" \
  targetClass="Belle2::ECLCluster" target="m_sqrtcovmat_00" \
  code="{m_sqrtcovmat_00 = onfile.m_Error[0];}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Error[6]" \
  targetClass="Belle2::ECLCluster" target="m_sqrtcovmat_11" \
  code="{m_sqrtcovmat_11 = onfile.m_Error[2];}"

#pragma read sourceClass="Belle2::ECLCluster" version="[-3]" \
  source="float m_Error[6]" \
  targetClass="Belle2::ECLCluster" target="m_sqrtcovmat_22" \
  code="{m_sqrtcovmat_22 = onfile.m_Error[5];}"

#endif
