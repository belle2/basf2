"""
This file contains variables globally used in the analysis.
Variales are grouped in logical classes (python dictionaries), and typicaly that's classes who are called.
Each group is organised as follows:
Group_Of_Variables = {
    'Variable1': {
        'tuple_name':'<Name of the variable used in nTuple>',
        'lLim': <lower limit of the variable. Will be used for drawing>,
        'uLim': <upper limit of the variable. Will be used for drawing>,
        'xaxis':"<root-TeX-style name of the variable used to name axes on root plots>",
        'MClLim': <lower limit of the variable at gen-level. Will be used for drawing. This field is optional>,
        'MCuLim': <upper limit of the variable at gen-level. Will be used for drawing. This field is optional>,
        'units':"<TeX-style units of the variabel. This field is optional>",
        'logY': <False|True. If variable should be drawn with LogY. This field is optional>,
        'tex_name':"<TeX-style name of the variable used in formatting the tables. This field is optional>"
        },
    'Variable2':{...}
    ...
}
If you modify original ntuples, pleae make changes of tuple names here accordingly.
"""


#: Continuum background
CB_suppression = {
    'R2':  #: Ratio of 2nd to 0th order Fox-Wolfarm moments
    {'tuple_name': 'B_R2',
     'lLim': 0.,
     'uLim': 1.,
        'xaxis': "R2"},
    'CosTBz':  #: Cosine of the angle between the B thrust axis & the z axis in the CM
    {'tuple_name': 'B_CosTBz',
     'lLim': 0,
     'uLim': 1,
     'xaxis': "cos(#theta_{Bz})"},
    'cosTBTO':
        {'tuple_name': 'B_CosTBTO',
         'lLim': 0,
         'uLim': 1,
         'xaxis': "cos(#theta_{Bz})"},
    'ThrustB':
        {'tuple_name': 'B_ThrustB',
         'lLim': 0.9,
         'uLim': 1,
         'xaxis': "B_ThrustB"},
    'ThrustO':
        {'tuple_name': 'B_ThrustO',
         'lLim': 0,
         'uLim': 1,
         'xaxis': "B_ThrustO"},
    'B_cc1':
        {'tuple_name': 'B_cc1',
         'lLim': 0,
         'uLim': 10,
         'xaxis': 'B_cc1'}
}


#: KS0 variables
k0s_variables = {
    'KsOpAng':  #: Opening angle
    {'tuple_name': 'B_K0s_hel',
     'lLim': -1.001,
     'uLim': 1.001,
     'xaxis': "cos(#theta_{K^{0}_{s}})"},
    'Ksdr':  #: transverse distance in respect to IP
    {'tuple_name': 'B_K0s_dr',
             'lLim': 0.,
             'uLim': 120,
             'logY': False,
             'xaxis': "K^{0}_{s} flight distance, cm"},
    'goodKs':  #: GoodKs
    {'tuple_name': 'B_K0s_goodKs',
     'lLim': 0.,
     'uLim': 1.001,
     'xaxis': "goodKs"},
    'KsVLike':  #: K0s V-Like
    {'tuple_name': 'B_K0s_ksnbVLike',
     'lLim': 0.,
     'uLim': 1.001,
     'xaxis': "K0s v-like"},
    'KsNoLam':  #: KsNoLam
    {'tuple_name': 'B_K0s_ksnbNoLam',
     'lLim': -1.001,
     'uLim': 1.001,
     'xaxis': "KsNoLam"},
    'KsSt':  #: Standard
    {'tuple_name': 'B_K0s_ksnbStandard',
             'lLim': 0.,
             'uLim': 1.001,
             'xaxis': "Standard K0s"}
}

#: pi0 variables
pi0_variables = {
    'EgMin':  #: Min gamma energy
    {'tuple_name': 'min(B_g0_E,B_g1_E)',
     'lLim': 0,
     'uLim': 5.,
     'xaxis': "min(E_{#gamma}), GeV"},
    'EgMax':  #: Max gamma energy
    {'tuple_name': 'max(B_g0_E,B_g1_E)',
     'lLim': 0,
     'uLim': 5.,
     'xaxis': "max(E_{#gamma}), GeV"},
    'Eg1':  #: gamma 1 energy
    {'tuple_name': 'B_g0_E',
     'lLim': 0,
     'uLim': 5.,
     'xaxis': "E_{#gamma 1}, GeV"},
    'Eg2':  #: gamma 2 energy
    {'tuple_name': 'B_g1_E',
     'lLim': 0,
     'uLim': 5.,
     'xaxis': "E_{#gamma 2}, GeV"},
    'Thetag1':  #: gamma 1 cosTheta
    {'tuple_name': 'B_g0_cosTheta',
     'lLim': -1.,
     'uLim': 1.,
     'xaxis': "cos(#theta^{#gamma 1})"},
    'Thetag2':  #: gamma 2 coTheta
    {'tuple_name': 'B_g1_cosTheta',
     'lLim': -1.,
     'uLim': 1.,
     'xaxis': "cos(#theta^{#gamma 2})"},
    'Mpi0Rec':  #: Mass
    {'tuple_name': 'pow( pow(B_g0_E+B_g1_E,2) - pow(B_g0_px+B_g1_px,2) \
        - pow(B_g0_py+B_g1_py,2) - pow(B_g0_pz+B_g1_pz,2),0.5)',
     'lLim': 0.12,
     'uLim': 0.15,
     'xaxis': "M_{#pi^{0}}, GeV/c^{2}"}
}

#: PID variables
pid_variables = {
    'RhoPi_SepMin':  #: atc_pid(3,1,5,3,2) of pi- from rho0
    {'tuple_name': 'min(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
     'lLim': 0.,
     'uLim': 1.,
     'logY': False,
     'xaxis': "min(#pi_{#rho}) K/#pi separation"},
    'RhoPi_SepMax':  #: atc_pid(3,1,5,3,2) of pi- from rho0
    {'tuple_name': 'max(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
     'lLim': 0.,
     'uLim': 1.,
     'logY': False,
     'xaxis': "max(#pi_{#rho}) K/#pi separation"},
    'KstPi_Sep':  #: atc_pid(3,1,5,3,2) of pi+ from K*+
    {'tuple_name': 'B_Kh1_PID_K_vs_pi',
     'lLim': 0.,
     'uLim': 1.,
     'logY': False,
     'xaxis': "#pi_{K^*} K/#pi separation"},
    'KstK_Sep':  #: atc_pid(3,1,5,3,2) of pi+ from K*+
    {'tuple_name': 'B_Kh0_PID_K_vs_pi',
     'lLim': 0.,
     'uLim': 1.,
     'logY': False,
     'xaxis': "K_{K^*} K/#pi separation"}
}

#: Kinematic variables
kinematics = {
    'RhoPi_pT_min':
        {'tuple_name': 'min(B_Rh0_pt,B_Rh1_pt)',
         'lLim': 0.,
         'uLim': 3.,
         'logY': False,
         'xaxis': "min(P^{T}_{#pi from #rho}), GeV/c"},
    'RhoPi_pT_max':
        {'tuple_name': 'max(B_Rh0_pt,B_Rh1_pt)',
         'lLim': 0.,
         'uLim': 3.,
         'logY': False,
         'xaxis': "max(P^{T}_{#pi from #rho}), GeV/c"},
    'RhoPi_p_min':
        {'tuple_name': 'min(B_Rh0_p,B_Rh1_p)',
         'lLim': 0.,
         'uLim': 3.,
         'logY': False,
         'xaxis': "min(P_{#pi from #rho}), GeV/c"},
    'RhoPi_p_max':
        {'tuple_name': 'max(B_Rh0_p,B_Rh1_p)',
         'lLim': 0.,
         'uLim': 3.,
         'logY': False,
         'xaxis': "max(P_{#pi from #rho}), GeV/c"},
    #: K from K*
    'KstK_M':  #: Mass
        {'tuple_name': 'B_Kh0_M',
         'lLim': 0.46,
         'uLim': 0.53,
         'xaxis': "M_{K}, GeV/c^{2}"},
    'KstK_E':  #: Mass
        {'tuple_name': 'B_Kh0_E',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "E_{K}, GeV"},
    'KstK_p':  #: Mass
        {'tuple_name': 'B_Kh0_p',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "P_{K}, GeV/c"},
    'KstK_pt':  #: Mass
        {'tuple_name': 'B_Kh0_pt',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "P^{T}_{K}, GeV/c"},
    #: pi from K*
    'KstPi_M':  #: Mass
        {'tuple_name': 'B_Kh1_M',
         'lLim': 0.13,
         'uLim': 0.14,
         'xaxis': "M_{#pi from K^{*}}, GeV/c^{2}"},
    'KstPi_E':  #: Mass
        {'tuple_name': 'B_Kh1_E',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "E_{#pi from K^{*}}, GeV"},
    'KstPi_p':  #: Mass
        {'tuple_name': 'B_Kh1_p',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "P_{#pi from K^{*}}, GeV/c"},
    'KstPi_pt':  #: Mass
        {'tuple_name': 'B_Kh1_pt',
         'lLim': 0.,
         'uLim': 4.,
         'xaxis': "P^{T}_{#pi from K^{*}}, GeV/c"},
}

#: Vertex quality variables
vertex_variables = {
    'Pvalue':
        {'tuple_name': 'B_VtxPvalue',
         'lLim': 0.,
         'uLim': 0.1,
         'logY': False,
         'xaxis': "B-vertex p-value"},
    'RhoPi_dr_max':
        {'tuple_name': 'max(B_Rh0_dr,B_Rh1_dr)',
         'lLim': 0.,
         'uLim': 4,
         'logY': False,
         'xaxis': "max(dr_{#pi from #rho}), cm"},
    'RhoPi_dr_min':
        {'tuple_name': 'min(B_Rh0_dr,B_Rh1_dr)',
         'lLim': 0.,
         'uLim': 4,
         'logY': False,
         'xaxis': "min(dr_{#pi from #rho}), cm"},
    'KstK_dr':
        {'tuple_name': 'B_Kh0_dr',
         'lLim': 0.,
         'uLim': 100,
         'logY': False,
         'xaxis': "dr_{K}, cm"},
    'KstPi_dr':
        {'tuple_name': 'B_Kh1_dr',
         'lLim': 0.,
         'uLim': 4,
         'logY': False,
         'xaxis': "dr_{#pi from K^*}, cm"},
    'B_Rho':
        {'tuple_name': 'B_Rho',
         'lLim': 0.,
         'uLim': 4,
         'logY': False,
         'xaxis': "dr_{B}, cm"}
}


#: Kaon and pion identification for final state with K0
K0spi_pid_variables = {
    'RhoPi_SepMin':
        {'tuple_name': 'min(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "min(#pi_{#rho}) K/#pi separation"},
    'RhoPi_SepMax':
        {'tuple_name': 'max(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "max(#pi_{#rho}) K/#pi separation"},
    'KstPip_Sep':
        {'tuple_name': 'B_Kh1_PID_K_vs_pi',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "K_{K^*} K/#pi separation"}
}

#: Kaon and pion identification for final state with pi0
Kpi0_pid_variables = {
    'RhoPi_Kpi_SepMin':
        {'tuple_name': 'min(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "min(#pi_{#rho}) K/#pi separation"},
    'RhoPi_Kpi_SepMax':
        {'tuple_name': 'max(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "max(#pi_{#rho}) K/#pi separation"},
    'Kst_Kpi_Sep':
        {'tuple_name': 'B_Kh0_PID_K_vs_pi',
         'lLim': 0.,
         'uLim': 1.,
         'logY': False,
         'xaxis': "#K_{K^*} K/#pi separation"}
}

#: All-event variables
event_variables = {
    'VTX_Rank':
        {'tuple_name': 'B_vtx_rank',
         'lLim': 0.,
         'uLim': 10.,
         'logY': False,
         'xaxis': "Rank"},
    'nCands':
        {'tuple_name': 'nCands',
         'lLim': 0.,
         'uLim': 15,
         'logY': False,
         'xaxis': "Number of candidates"},
    'iCand':
        {'tuple_name': 'iCand',
         'lLim': 0.,
         'uLim': 100.,
         'logY': False,
         'xaxis': "Candidate ID"}
}
