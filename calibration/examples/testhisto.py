from basf2 import *
set_log_level(LogLevel.INFO)

import multiprocessing

# prepare a multiprocessing context which uses fork
ctx = multiprocessing.get_context("fork")

working_dir = "collector_output"

if not os.path.exists(working_dir):
    os.mkdir(working_dir)

# Comment out to turn off debugging
logging.package("calibration").log_level = LogLevel.DEBUG


def _create_file(num):
    main = create_path()
    main.add_module("EventInfoSetter", expList=[1], runList=[run], evtNumList=[10000])
    main.add_module("RootOutput", outputFileName="MyInputFile_" + str(run) + ".root")
    process(main)
    print(statistics)


def _run_child(run):
    set_random_seed(run*5)
    main = create_path()
    main.add_module("RootInput", inputFileNames=["MyInputFile_" + str(run) + ".root"])
    main.add_module("HistoManager", histoFileName=working_dir+"/MyOutputFile_" + str(run) + ".root", workDirName=working_dir)
    testmod = main.add_module("TestHisto", entriesPerEvent=100)
    process(main)
    print(statistics)


for run in range(1, 11):
    child = ctx.Process(target=_create_file, args=(run,))
    child.start()
    # wait for it to finish
    child.join()

for run in range(1, 11):
    child = ctx.Process(target=_run_child, args=(run,))
    child.start()
    # wait for it to finish
    child.join()
