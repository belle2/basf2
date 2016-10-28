#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2_mva


if __name__ == "__main__":

    general = basf2_mva.GeneralOptions()

    # path to your files here
    general.m_datafiles = basf2_mva.vector(
        "/nfs/dust/belle2/user/jkrohn/belle2/my_stuff/generation/root_files/FINAL_training/*.root",)

    general.m_treename = "KLMdata"
    general.m_weight = ""
    general.m_identifier = "KLMKLExpert.xml"
    general.m_variables = basf2_mva.vector(
        "KLMnLayer",
        "KLMnInnermostlayer",
        "KLMglobalZ",
        "KLMtime",
        "KLMdistToNextCl",
        "KLMenergy",
        "KLMtrackToECL",
        "KLMECLEerror",
        "KLMTrackSepDist",
        "KLMInitialtrackSepAngle",
        "KLMTrackRotationAngle",
        "KLMTrackSepAngle")

    general.m_target_variable = "KLMTruth"
    specific = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general, specific)

    # Upload the weightfile on disk to the database
    # basf2_mva.upload('KLMKLExpert.xml', 'KLMKLExpert')
