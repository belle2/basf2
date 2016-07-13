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
    general.m_variables = basf2_mva.vector("ECLenergy", "ECLE9oE25", "ECLtiming", "ECLEerror", "ECLdistToTrack", "ECLdeltaL")

    general.m_target_variable = "ECLTruth"
    specific = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general, specific)
