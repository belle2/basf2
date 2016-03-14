#!/usr/bin/env python
# -*- coding: utf-8 -*-

import logging

import basf2

from tracking.run.event_generation import ReadOrGenerateEventsRun

import trackfindingcdc.cdcdisplay as cdcdisplay


class CDCDebugDisplayRun(ReadOrGenerateEventsRun):
    finder_module = "TrackFinderCDCAutomatonDev"
    output_folder = '/tmp'
    iteractive = True

    def __init__(self):
        super(CDCDebugDisplayRun, self).__init__()
        self._cdc_display_module = \
            cdcdisplay.CDCSVGDisplayModule(self.output_folder)

    @property
    def cdc_display_module(self):
        cdc_display_module = self._cdc_display_module

        # Reinforce the output folder in case it has changed
        return cdc_display_module

    def create_argument_parser(self, **kwds):
        argument_parser = super(CDCDebugDisplayRun,
                                self).create_argument_parser(**kwds)

        argument_parser.add_argument('-o', '--output-folder',
                                     dest='output_folder',
                                     default=self.output_folder,
                                     help='Folder where the output files are written to. If the folder does not exist create it. '
                                     )

        argument_parser.add_argument('--non-interactive', dest='interactive',
                                     action='store_false',
                                     help='Run in batch mode and do not show the each event.'
                                     )

        subparser_description = \
            """
Various options to configure what shall be drawn in the display.
Note that some options are only relevant, if the cellular automaton finder in the CDC has been run before.
"""
        draw_argument_group = \
            argument_parser.add_argument_group(title='Draw options',
                                               description=subparser_description)

        cdc_display_module = self.cdc_display_module
        for option in sorted(cdc_display_module.all_drawoptions):
            options_flag = '--%s ' % option.replace('_', '-')

            draw_argument_group.add_argument(options_flag, dest=option,
                                             action='store_true', default=getattr(cdc_display_module,
                                                                                  option))

        return argument_parser

    def configure(self, arguments):
        super(CDCDebugDisplayRun, self).configure(arguments)

        cdc_display_module = self.cdc_display_module

        cdc_display_module.output_folder = arguments.output_folder
        cdc_display_module.interactive = arguments.interactive

        for option in cdc_display_module.all_drawoptions:
            try:
                is_active_option = getattr(arguments, option)
            except AttributeError:
                continue
            else:
                print 'Setting', option, 'to', is_active_option
                setattr(cdc_display_module, option, is_active_option)

    def create_path(self):
        path = super(CDCDebugDisplayRun, self).create_path()

        finder_module = self.finder_module
        if isinstance(finder_module, str):
            finder_module = basf2.register_module(finder_module)

        finder_module.logging.log_level = basf2.LogLevel.DEBUG
        finder_module.logging.debug_level = 200

        path.add_module(finder_module)

        path.add_module(self.cdc_display_module)
        return path


def main():
    cdcDebugDisplayRun = CDCDebugDisplayRun()
    cdcDebugDisplayRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig()
    main()
