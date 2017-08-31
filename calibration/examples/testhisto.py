from basf2 import *
set_log_level(LogLevel.INFO)

import multiprocessing

# prepare a multiprocessing context which uses fork
ctx = multiprocessing.get_context("fork")

if not os.path.exists("collector_output"):
    os.mkdir("collector_output")

logging.package("calibration").log_level = LogLevel.DEBUG


def _create_file(num):
    main = create_path()
    main.add_module("EventInfoSetter", expList=[1, 1], runList=[run, run + 1], evtNumList=[100, 100])
    main.add_module("RootOutput", outputFileName="MyInputFile_" + str(run) + ".root")
    process(main)
    print(statistics)


def _run_child(run):
    main = create_path()
    main.add_module("RootInput", inputFileNames=["MyInputFile_" + str(run) + ".root", "MyInputFile_" + str(run + 1) + ".root"])
    main.add_module("HistoManager", histoFileName="MyOutputFile_" + str(run) + ".root", workDirName="collector_output")
    testmod = main.add_module("TestHisto", entriesPerEvent=10)
    process(main)
    print(statistics)


for run in range(1, 4):
    child = ctx.Process(target=_create_file, args=(run,))
    child.start()
    # wait for it to finish
    child.join()

for run in range(1, 3):
    child = ctx.Process(target=_run_child, args=(run,))
    child.start()
    # wait for it to finish
    child.join()
