from basf2 import *
set_log_level(LogLevel.INFO)

import multiprocessing

# prepare a multiprocessing context which uses fork
ctx = multiprocessing.get_context("fork")

if not os.path.exists("collector_output"):
    os.mkdir("collector_output")


def _run_child(run):
    main = create_path()
    main.add_module("EventInfoSetter", expList=[1], runList=[run], evtNumList=[100])
    main.add_module("HistoManager", histoFileName="MyOutputFile_" + str(run) + ".root", workDirName="collector_output")
    main.add_module("TestHisto", entriesPerEvent=10)
    process(main)


for run in range(1, 3):
    child = ctx.Process(target=_run_child, args=(run,))
    child.start()
    # wait for it to finish
    child.join()
