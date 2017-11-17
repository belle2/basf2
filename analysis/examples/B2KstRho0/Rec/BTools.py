RecBTools = ['EventMetaData', '^B+']
RecBTools += ['InvMass', '^B+']
RecBTools += ['DeltaEMbc', '^B+']
RecBTools += ['Vertex', '^B+']
RecBTools += ['CustomFloats[E:p:px:py:pz:pt]', '^B+']
RecBTools += ['CustomFloats[Mbc_corr:helR:helK:MK:MR:deltaE_corr:isContinuumEvent:helK_ref:helR_ref]', '^B+']
RecBTools += ['CustomFloats[K_px:K_py:K_pz:K_pt:K_p:K_E:K_M:K_ErrM:K_SigM:K_charge:K_isSignal:K_PID_K_vs_pi:K_PID_p_vs_pi:\
K_PID_p_vs_K:K_PID_muID:K_PID_eID:K_dr:K_dx:K_dy:K_dz:K_SOD:\
\
Kh0_px:Kh0_py:Kh0_pz:Kh0_pt:Kh0_p:Kh0_E:Kh0_M:Kh0_ErrM:Kh0_SigM:Kh0_charge:Kh0_isSignal:Kh0_PID_K_vs_pi:\
Kh0_PID_p_vs_pi:Kh0_PID_p_vs_K:Kh0_PID_muID:Kh0_PID_eID:Kh0_dr:Kh0_dx:Kh0_dy:Kh0_dz:Kh0_SOD:hel_Kh0_px:hel_Kh0_py:hel_Kh0_pz:hel_Kh0_p:\
\
Kh1_px:Kh1_py:Kh1_pz:Kh1_pt:Kh1_p:Kh1_E:Kh1_M:Kh1_ErrM:Kh1_SigM:Kh1_charge:Kh1_isSignal:Kh1_PID_K_vs_pi:\
Kh1_PID_p_vs_pi:Kh1_PID_p_vs_K:Kh1_PID_muID:Kh1_PID_eID:Kh1_dr:Kh1_dx:Kh1_dy:Kh1_dz:Kh1_SOD:hel_Kh1_px:hel_Kh1_py:hel_Kh1_pz:hel_Kh1_p:\
\
R_px:R_py:R_pz:R_pt:R_p:R_E:R_M:R_ErrM:R_SigM:R_charge:R_isSignal:R_PID_K_vs_pi:R_PID_p_vs_pi:\
R_PID_p_vs_K:R_PID_muID:R_PID_eID:R_dr:R_dx:R_dy:R_dz:R_SOD:\
\
Rh0_px:Rh0_py:Rh0_pz:Rh0_pt:Rh0_p:Rh0_E:Rh0_M:Rh0_ErrM:Rh0_SigM:Rh0_charge:Rh0_isSignal:Rh0_PID_K_vs_pi:\
Rh0_PID_p_vs_pi:Rh0_PID_p_vs_K:Rh0_PID_muID:Rh0_PID_eID:Rh0_dr:Rh0_dx:Rh0_dy:Rh0_dz:Rh0_SOD:hel_Rh0_px:hel_Rh0_py:hel_Rh0_pz:hel_Rh0_p:\
\
Rh1_px:Rh1_py:Rh1_pz:Rh1_pt:Rh1_p:Rh1_E:Rh1_M:Rh1_ErrM:Rh1_SigM:Rh1_charge:Rh1_isSignal:Rh1_PID_K_vs_pi:\
Rh1_PID_p_vs_pi:Rh1_PID_p_vs_K:Rh1_PID_muID:Rh1_PID_eID:Rh1_dr:Rh1_dx:Rh1_dy:Rh1_dz:Rh1_SOD:hel_Rh1_px:\
hel_Rh1_py:hel_Rh1_pz:hel_Rh1_p]', '^B+']
RecBTools += ['CustomFloats[vtx_rank:rnd_rank:rnd]', '^B+']
RecBTools += ['CustomFloats[Kpi0fakeK_M:K0spifakeK_M:M_3pi:Kpi0fakeD_M:K0spifakeD_M]', '^B+']


RoETools = ['ContinuumSuppression', '^B+']
RoETools += ['CustomFloats[out_FastBDT]', '^B+']
RoETools += ['CustomFloats[csv_FastBDT]', '^B+']


MCBTools = ['MCVertex', '^B+']
MCBTools += ['MCTruth', '^B+ ']
MCBTools += ['CustomFloats[MCTrue:isSignal]', '^B+']
MCBTools += ['CustomFloats[MCT_Mbc_corr:MCT_helR:MCT_helK:MCT_MK:MCT_MR:MCT_deltaE_corr]', '^B+']
MCBTools += ['CustomFloats[MCT_K_px:MCT_K_py:MCT_K_pz:MCT_K_pt:MCT_K_p:MCT_K_E:MCT_K_M:MCT_K_ErrM:MCT_K_SigM:MCT_K_charge:\
MCT_K_isSignal:MCT_K_mcPDG:MCT_K_PID_K_vs_pi:MCT_K_PID_p_vs_pi:MCT_K_PID_p_vs_K:MCT_K_PID_muID:MCT_K_PID_eID:MCT_K_dr:MCT_K_dx:MCT_K_dy:MCT_K_dz:MCT_K_SOD:\
MCT_Kh0_px:MCT_Kh0_py:MCT_Kh0_pz:MCT_Kh0_pt:MCT_Kh0_p:MCT_Kh0_E:MCT_Kh0_M:MCT_Kh0_ErrM:MCT_Kh0_SigM:MCT_Kh0_charge:\
MCT_Kh0_isSignal:MCT_Kh0_mcPDG:MCT_Kh0_PID_K_vs_pi:MCT_Kh0_PID_p_vs_pi:MCT_Kh0_PID_p_vs_K:MCT_Kh0_PID_muID:MCT_Kh0_PID_eID:\
MCT_Kh0_dr:MCT_Kh0_dx:MCT_Kh0_dy:MCT_Kh0_dz:MCT_Kh0_SOD:hel_MCT_Kh0_px:hel_MCT_Kh0_py:hel_MCT_Kh0_pz:hel_MCT_Kh0_p:\
MCT_Kh1_px:MCT_Kh1_py:MCT_Kh1_pz:MCT_Kh1_pt:MCT_Kh1_p:MCT_Kh1_E:MCT_Kh1_M:MCT_Kh1_ErrM:MCT_Kh1_SigM:MCT_Kh1_charge:\
MCT_Kh1_isSignal:MCT_Kh1_mcPDG:MCT_Kh1_PID_K_vs_pi:MCT_Kh1_PID_p_vs_pi:MCT_Kh1_PID_p_vs_K:MCT_Kh1_PID_muID:MCT_Kh1_PID_eID:\
MCT_Kh1_dr:MCT_Kh1_dx:MCT_Kh1_dy:MCT_Kh1_dz:MCT_Kh1_SOD:hel_MCT_Kh1_px:hel_MCT_Kh1_py:hel_MCT_Kh1_pz:hel_MCT_Kh1_p:\
MCT_R_px:MCT_R_py:MCT_R_pz:MCT_R_pt:MCT_R_p:MCT_R_E:MCT_R_M:MCT_R_ErrM:MCT_R_SigM:MCT_R_charge:MCT_R_isSignal:MCT_R_mcPDG:\
MCT_R_PID_K_vs_pi:MCT_R_PID_p_vs_pi:MCT_R_PID_p_vs_K:MCT_R_PID_muID:MCT_R_PID_eID:MCT_R_dr:MCT_R_dx:MCT_R_dy:MCT_R_dz:MCT_R_SOD:\
MCT_Rh0_px:MCT_Rh0_py:MCT_Rh0_pz:MCT_Rh0_pt:MCT_Rh0_p:MCT_Rh0_E:MCT_Rh0_M:MCT_Rh0_ErrM:MCT_Rh0_SigM:MCT_Rh0_charge:MCT_Rh0_isSignal:\
MCT_Rh0_mcPDG:MCT_Rh0_PID_K_vs_pi:MCT_Rh0_PID_p_vs_pi:MCT_Rh0_PID_p_vs_K:MCT_Rh0_PID_muID:MCT_Rh0_PID_eID:MCT_Rh0_dr:\
MCT_Rh0_dx:MCT_Rh0_dy:MCT_Rh0_dz:MCT_Rh0_SOD:hel_MCT_Rh0_px:hel_MCT_Rh0_py:hel_MCT_Rh0_pz:hel_MCT_Rh0_p:\
MCT_Rh1_px:MCT_Rh1_py:MCT_Rh1_pz:MCT_Rh1_pt:MCT_Rh1_p:MCT_Rh1_E:MCT_Rh1_M:MCT_Rh1_ErrM:MCT_Rh1_SigM:MCT_Rh1_charge:\
MCT_Rh1_isSignal:MCT_Rh1_mcPDG:MCT_Rh1_PID_K_vs_pi:MCT_Rh1_PID_p_vs_pi:MCT_Rh1_PID_p_vs_K:MCT_Rh1_PID_muID:MCT_Rh1_PID_eID:\
MCT_Rh1_dr:MCT_Rh1_dx:MCT_Rh1_dy:MCT_Rh1_dz:MCT_Rh1_SOD:hel_MCT_Rh1_px:hel_MCT_Rh1_py:hel_MCT_Rh1_pz:hel_MCT_Rh1_p]', '^B+']
MCBTools += ['CustomFloats[Kh0_mcPDG:Kh0_mcmotherPDG:Kh1_mcPDG:Kh1_mcmotherPDG:Rh0_mcPDG:Rh0_mcmotherPDG:\
Rh1_mcPDG:Rh1_mcmotherPDG]', '^B+']

K0sTools = ['CustomFloats[K0s_dr:K0s_hel:K0s_SoD:K0s_goodKs:K0s_ksnbVLike:K0s_ksnbNoLam:K0s_ksnbStandard]', '^B+']
pi0Tools = ['CustomFloats[g0_E:g1_E:g0_p:g0_px:g0_py:g0_pz:g0_pt:g1_p:g1_px:g1_py:g1_pz:g1_pt:g0_cosTheta:g1_cosTheta]', '^B+']
MCK0sTools = [
    'CustomFloats[gd0_isSignal:gd1_isSignal:MCT_K0s_dr:MCT_K0s_hel:MCT_K0s_SoD:MCT_K0s_goodKs:MCT_K0s_ksnbVLike:\
    MCT_K0s_ksnbNoLam:MCT_K0s_ksnbStandard]',
    '^B+']
MCpi0Tools = ['CustomFloats[gd0_isSignal:gd1_isSignal:MCT_g0_E:MCT_g1_E:MCT_g0_p:MCT_g0_px:MCT_g0_py:MCT_g0_pz:MCT_g0_pt:MCT_g1_p:MCT_g1_px:\
MCT_g1_py:MCT_g1_pz:MCT_g1_pt:MCT_g0_cosTheta:MCT_g1_cosTheta]', '^B+']
