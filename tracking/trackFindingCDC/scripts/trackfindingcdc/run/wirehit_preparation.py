from tracking.run.mixins import RunMixin

import argparse


class WireHitPreparerRunMixin(RunMixin):
    """Mixin part the """

    #: Used option for the initial flight time estimation
    flight_time_estimation = ""

    #: Loop limit above which on wire hits are prepared for the track finder - Uses MC information
    n_loops = float("nan")

    def create_argument_parser(self, **kwds):
        """Configure the basf2 job script using the translated command-line arguments"""

        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            "-ft",
            "--flight-time-estimation",
            choices=["none", "outwards", "downwards"],
            default=argparse.SUPPRESS,
            dest="flight_time_estimation",
            help=("Choose which estimation method for the time of flight should be use. \n"
                  "* 'none' no time of flight corrections\n"
                  "* 'outwards' means the minimal time needed to travel to the wire from the interaction point \n"
                  "* 'downwards' means the minimal time needed to travel to the wire from the y = 0 plane downwards \n")
        )

        argument_parser.add_argument(
            "-l",
            "--mc-loop-limit",
            type=float,
            dest="n_loops",
            help=("Choose to block all wire hits but the ones located on the {mc_loops} first loops")
        )

        return argument_parser

    def create_path(self):
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """
        path = super().create_path()

        wire_hit_preparer = path.add_module("TFCDC_WireHitPreparer",
                                            UseNLoops=self.n_loops)
        if self.flight_time_estimation:
            wire_hit_preparer.param(dict(flightTimeEstimation=self.flight_time_estimation))

        return path
