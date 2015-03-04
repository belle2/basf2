#!/usr/bin/env python
# -*- coding: utf-8 -*-


from trackfindingcdc.run.display import CDCDisplayRun

import logging


class CDCDebugDisplayRun(CDCDisplayRun):
    finder_module = "TrackFinderCDCAutomatonDev"
    show_all_drawoptions = True  # Also show draw options that are related to the cellular automaton track finder on the command line!


def main():
    cdcDebugDisplayRun = CDCDebugDisplayRun()
    cdcDebugDisplayRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
