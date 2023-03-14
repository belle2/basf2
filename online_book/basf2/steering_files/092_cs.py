#!/usr/bin/env python3

import basf2_mva

general_options = basf2_mva.GeneralOptions()
general_options.m_datafiles = basf2_mva.vector("ContinuumSuppression.root")
general_options.m_treename = "tree"
general_options.m_identifier = "MVAFastBDT.root"  # outputted weightfile
general_options.m_variables = basf2_mva.vector(
    "R2",
    "thrustBm",
    "thrustOm",
    "cosTBTO",
    "cosTBz",
    "KSFWVariables(et)",
    "KSFWVariables(mm2)",
    "KSFWVariables(hso00)",
    "KSFWVariables(hso02)",
    "KSFWVariables(hso04)",
    "KSFWVariables(hso10)",
    "KSFWVariables(hso12)",
    "KSFWVariables(hso14)",
    "KSFWVariables(hso20)",
    "KSFWVariables(hso22)",
    "KSFWVariables(hso24)",
    "KSFWVariables(hoo0)",
    "KSFWVariables(hoo1)",
    "KSFWVariables(hoo2)",
    "KSFWVariables(hoo3)",
    "KSFWVariables(hoo4)",
    "CleoConeCS(1)",
    "CleoConeCS(2)",
    "CleoConeCS(3)",
    "CleoConeCS(4)",
    "CleoConeCS(5)",
    "CleoConeCS(6)",
    "CleoConeCS(7)",
    "CleoConeCS(8)",
    "CleoConeCS(9)",
)
general_options.m_target_variable = "isContinuumEvent"
fastbdt_options = basf2_mva.FastBDTOptions()

basf2_mva.teacher(general_options, fastbdt_options)
