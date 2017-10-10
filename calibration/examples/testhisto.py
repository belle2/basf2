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
logging.package("calibration").debug_level = 90


def _create_file(run):
    main = create_path()
    main.add_module("EventInfoSetter", expList=[1, 1], runList=[run, run + 1], evtNumList=[100, 100])
    main.add_module("RootOutput", outputFileName="test_data/MyInputFile_" + str(run) + ".root")
    process(main)
    print(statistics)


def _run_child(run):
    set_random_seed(run * 5)
    main = create_path()
    main.add_module("RootInput", inputFileNames=["test_data/MyInputFile_*.root"], entrySequences=[])
    main.add_module("HistoManager", histoFileName="MyOutputFile_" + str(run) + ".root", workDirName=working_dir)
    testmod = main.add_module("TestHisto", entriesPerEvent=1000)
    testmod.param("granularity", "all")
#    testmod.param("preScale", 0.5)
#    testmod.param("maxEventsPerRun", 100)
    process(main)
    # print(statistics(statistics.TOTAL))
    print(statistics)


def create_test_data():
    for run in range(1, 11):
        child = ctx.Process(target=_create_file, args=(run,))
        child.start()
        # wait for it to finish
        child.join()


def run_collector():
    for run in range(1, 2):
        child = ctx.Process(target=_run_child, args=(run,))
        child.start()
        # wait for it to finish
        child.join()


if __name__ == "__main__":
    # create_test_data()
    run_collector()
