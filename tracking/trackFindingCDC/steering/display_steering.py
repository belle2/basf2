#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

import basf2
basf2.set_log_level(basf2.LogLevel.INFO)
# basf2.set_random_seed(12345)

import cdclocaltracking.cdcdisplay as cdcdisplay
import cdclocaltracking.event_generation as event_generation


def main():
    """
    Sets up standard command line arguments for event generation or event reading from files and displayes the events with the svg display.
    """

    argument_parser = event_generation.create_argument_parser()
    arguments = argument_parser.parse_args()

    main_path = event_generation.create_path_from_parsed_arguments(arguments)

    # Add the display module as the last module in the chain
    svgdisplay = cdcdisplay.CDCSVGDisplayModule('/tmp')
    svgdisplay.draw_wires = True
    svgdisplay.draw_hits = False

    svgdisplay.draw_superlayer_boundaries = True
    svgdisplay.draw_interactionpoint = True

    svgdisplay.draw_mcparticle_id = False
    svgdisplay.draw_mcparticle_pdgcodes = False
    svgdisplay.draw_mcparticle_primary = False

    svgdisplay.draw_mcsegments = False

    svgdisplay.draw_simhits = False
    svgdisplay.draw_simhit_tof = False
    svgdisplay.draw_simhit_posflag = False
    svgdisplay.draw_simhit_pdgcode = False
    svgdisplay.draw_simhit_bkgtag = True

    svgdisplay.draw_connect_tof = False

    svgdisplay.draw_rlinfo = False
    svgdisplay.draw_reassigned = False

    main_path.add_module(svgdisplay)

    basf2.process(main_path)
    print basf2.statistics


if __name__ == '__main__':
    main()
    sys.exit(0)
