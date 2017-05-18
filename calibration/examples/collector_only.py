from basf2 import *
set_log_level(LogLevel.INFO)

for run in range(1, 5):
    main = create_path()
    main.add_module("RootInput", inputFileName="test_data/DST_exp1_run" + str(run) + ".root")
    main.add_module("CaTest")
    main.add_module('RootOutput', branchNames=['EventMetaData'], outputFileName="CollectorOutput_" + str(run) + ".root")
    process(main)
