#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2_mva


if __name__ == "__main__":

    general = basf2_mva.GeneralOptions()

    # path to your files here
    general.m_datafiles = basf2_mva.vector(
        "/afs/desy.de/user/j/jkrohn/nfs/belle2/my_stuff/generation/root_files/more_new_vars_id/leo_id*.root",)
    general.m_treename = "ECLdata"
    general.m_weight = ""
    general.m_weightfile = "ECLExpert.xml"
    general.m_variables = basf2_mva.vector(
        "m_KLMnLayer",
        "m_KLMnInnermostLayer",
        "m_KLMglobalZ",
        "m_KLMtime",
        "m_KLMnextCluster",
        "m_KLMenergy",
        "m_KLMtrackToECL",
        "m_KLMECLEerror",
        "m_KLMTrackSepDist",
        "m_KLMInitialTrackSepAngle",
        "m_KLMTrackRotationAngle",
        "m_KLMTrackSepAngle")

    general.m_target_variable = "KLMTruth"
    specific = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general, specific)
