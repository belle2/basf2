#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from tracking.run.tracked_event_generation import ReadOrGenerateTrackedEventsRun
import trackfindingcdc.cdcdisplay as cdcdisplay


class CDCDisplayRun(ReadOrGenerateTrackedEventsRun):
    """Prepare and execute a basf2 job to read generated events or generate new events then display the CDC tracks"""

    #: Destination folder for displays
    output_folder = '/tmp'

    #: Switch to show the event display after each event. Run in batch mode for false
    iteractive = True

    #: Switch to also show draw command line options only related to the cellular automaton track finder
    show_all_drawoptions = False

    #: Prefix of the output files
    filename_prefix = ""

    def __init__(self):
        """Constructor"""
        super().__init__()
        #: Use the CDCSVGDisplay module to draw the CDC and tracks/hits
        self._cdc_display_module = cdcdisplay.CDCSVGDisplayModule(self.output_folder)

    @property
    def cdc_display_module(self):
        """Get the display module"""
        cdc_display_module = self._cdc_display_module
        return cdc_display_module

    def create_argument_parser(self, **kwds):
        """Translate the command-lne arguments to basf2-job parameter"""
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-o',
            '--output-folder',
            dest='output_folder',
            default=self.output_folder,
            help='Folder where the output files are written to. If the folder does not exist create it. '
        )

        argument_parser.add_argument(
            '--non-interactive',
            dest='interactive',
            action='store_false',
            help='Run in batch mode and do not show each event.'
        )

        argument_parser.add_argument(
            '--use-python',
            dest='use_python',
            action='store_true',
            help='Swtich to activate the legacy implementation written in python'
        )

        argument_parser.add_argument(
            "--use_time_in_filename",
            action='store_true',
            help='Use the current time in the names of the generated files'
        )

        argument_parser.add_argument(
            "-pf",
            '--filename_prefix',
            default=self.filename_prefix,
            help='Prefix to the names of the generated files'
        )

        argument_parser.add_argument(
            '-m',
            '--mc-tracks',
            action='store_const',
            dest='finder_module',
            const='TrackFinderMCTruthRecoTracks',
            default=self.finder_module,
            help='Generate the mc tracks using the TrackFinderMCTruthRecoTracks. Short hand for -f TrackFinderMCTruthRecoTracks'
        )

        subparser_description = \
            """
Various options to configure what shall be drawn in the display.
Note that some options are only relevant, if the cellular automaton finder in the CDC has been run before.
"""
        draw_argument_group = argument_parser.add_argument_group(
            title='Draw options',
            description=subparser_description
        )

        cdc_display_module = self.cdc_display_module

        if self.show_all_drawoptions:
            drawoptions = cdc_display_module.all_drawoptions
        else:
            drawoptions = cdc_display_module.drawoptions

        for option in sorted(drawoptions):
            options_flag = '--%s ' % option.replace('_', '-')

            draw_argument_group.add_argument(
                options_flag,
                dest=option,
                action='store_true',
                default=getattr(cdc_display_module, option)
            )

        return argument_parser

    def configure(self, arguments):
        """Configure the basf2 job script using the translated command-line arguments"""
        super().configure(arguments)

        cdc_display_module = self.cdc_display_module

        cdc_display_module.output_folder = arguments.output_folder
        cdc_display_module.interactive = arguments.interactive

        cdc_display_module.use_python = arguments.use_python
        cdc_display_module.use_cpp = not arguments.use_python
        cdc_display_module.use_time_in_filename = arguments.use_time_in_filename
        cdc_display_module.filename_prefix = arguments.filename_prefix

        if self.show_all_drawoptions:
            drawoptions = cdc_display_module.all_drawoptions
        else:
            drawoptions = cdc_display_module.drawoptions

        for option in drawoptions:
            try:
                is_active_option = getattr(arguments, option)
            except AttributeError:
                continue
            else:
                print('Setting', option, 'to', is_active_option)
                setattr(cdc_display_module, option, is_active_option)

    def create_path(self):
        """Create the basf2 path"""
        main_path = super().create_path()
        main_path.add_module(self.cdc_display_module)
        return main_path
