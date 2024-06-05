#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
``%(prog)s`` is a tool for converting a list of LPNs into YAML format expected by
:ref:`b2skim-prod<b2skim-prod>`. The expected input to ``%(prog)s`` is a text file of
LPNs, like those which can be downloaded from the dataset searcher.

The test sample labels (under the key ``sampleLabel``) are automatically generated, so
please check they all correspond to a label ``skim/scripts/TestFiles.yaml`` after
running the script.

--epilog--
.. rubric:: Example usage

* Convert list of BGx1 MCri LPNs into YAML format and print to screen::

    $ %(prog)s my_MCri_LPNs.txt --mcri --bg BGx1

* Convert list of BGx1 MCrd LPNs into YAML format and print to screen::

    $ %(prog)s my_MCrd_LPNs.txt --mcrd --bg BGx1

* Convert list of data LPNs into YAML format and save to file::

    $ %(prog)s my_data_LPNs.txt --data -o my_data_LPNs.yaml

"""

import argparse
import pandas as pd
from pathlib import Path
import re
import sys

from skim.utils.testfiles import DataSample, MCSample

import yaml


def get_argument_parser():
    description, epilog = __doc__.split("--epilog--")
    parser = argparse.ArgumentParser(
        description=description,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=epilog,
    )
    parser.add_argument(
        metavar="input_lpn_list_file",
        dest="input",
        help="Input file containing list of LPNs (such as that from the dataset searcher).",
    )
    parser.add_argument(
        "-o",
        metavar="output_filename",
        dest="output",
        help="Output YAML file name. If none given, prints output to screen.",
    )
    DataMCGroup = parser.add_mutually_exclusive_group(required=True)
    DataMCGroup.add_argument(
        "--data", action="store_true", help="Flag to indicate the LPNs are for data."
    )
    DataMCGroup.add_argument(
         "--mcri", action="store_true", help="Flag to indicate the LPNs are for run-independent MC."
    )
    DataMCGroup.add_argument(
        "--mcrd", action="store_true", help="Flag to indicate the LPNs are for run-dependent MC."
    )

    parser.add_argument(
        "--bg",
        choices=("BGx0", "BGx1"),
        required=("--mcri" in sys.argv or "--mcrd" in sys.argv),
        help="Beam background level of MC samples. Only required for MC.",
    )
    return parser


def verify_dataframe(df, mcri):
    # Check that we got the names of the columns right
    if not (
        all(df["release"].str.startswith("release-"))
        and all(df["DBGT"].str.startswith("DB"))
        and all(df["expNumber"].str.startswith("e"))
        and all(df["runNumber"].str.startswith("r"))
    ):
        raise ValueError(
            "The column values don't seem to line up with what's expected."
        )

    # Check assumptions about columns
    if not all(df["dataLevel"].str.match("mdst")):
        raise ValueError("Input LPNs must all be mdst.")
    if "generalSkimName" in df.columns and len(set(df["generalSkimName"])) > 1:
        raise ValueError("More than one GeneralSkimName in input data LPNs.")
    if mcri and len(set(df["runNumber"])) > 1:
        raise ValueError("More than one run number listed for MC LPNs.")


def to_yaml(data, output=None):
    """Print to screen or file as YAML format."""
    string = yaml.dump(data, sort_keys=False)

    # Add warning about sample labels
    warning = "# TODO: Ensure this label matches a sample label in SkimStats.json"
    string = re.sub("(sampleLabel.*)$", f"\\1  {warning}", string, flags=re.MULTILINE)

    if isinstance(output, (str, Path)):
        OutputFilename = Path(output).with_suffix(".yaml")
        with open(OutputFilename, "w") as f:
            f.write(string)
        print(f"Wrote YAML file to {OutputFilename}", file=sys.stderr)
    elif not output:
        print(string)


def main():
    parser = get_argument_parser()
    args = parser.parse_args()

    # Read in data
    with open(args.input) as f:
        LPNs = sorted(filter(None, f.read().split("\n")))

    # Trim prefix (everything up to the release number)
    prefixes = {re.match(r"^(.*)(/release)", LPN)[1] for LPN in LPNs}
    if len(prefixes) > 1:
        raise ValueError("Somehow got multiple different prefixes!")
    else:
        prefix = list(prefixes)[0]
    LPNs = [re.sub(r"^.*(release)", r"\1", LPN) for LPN in LPNs]

    # ...and put it all into a lovely dataframe, split by LPN part!
    df = pd.DataFrame([Path(LPN).parts for LPN in LPNs])
    if args.data:
        if len(df.columns) == 8:
            # If eight components to LPN, then we're dealing with the old data LPN schema
            columns = dict(
                enumerate(
                    [
                        "release",
                        "DBGT",
                        "campaign",
                        "prodNumber",
                        "expNumber",
                        "beamEnergy",
                        "runNumber",
                        "dataLevel",
                    ]
                )
            )
        elif len(df.columns) == 9:
            # If nine components to LPN, then we're dealing with the new data LPN schema,
            # which includes an additional GeneralSkimName component
            columns = dict(
                enumerate(
                    [
                        "release",
                        "DBGT",
                        "campaign",
                        "prodNumber",
                        "expNumber",
                        "beamEnergy",
                        "runNumber",
                        "generalSkimName",
                        "dataLevel",
                    ]
                )
            )
    else:
        columns = dict(
            enumerate(
                [
                    "release",
                    "DBGT",
                    "campaign",
                    "prodNumber",
                    "s00",
                    "expNumber",
                    "beamEnergy",
                    "runNumber",
                    "MCEventType",
                    "dataLevel",
                ]
            )
        )

    df.rename(columns=columns, inplace=True)
    if args.data:
        if 'generalSkimName' not in df.columns:
            df['generalSkimName'] = ""
    verify_dataframe(df, args.mcri)

    if args.mcri or args.mcrd:
        df.loc[:, "expNumber"] = df["s00"] + "/" + df["expNumber"]
        df.drop("s00", axis=1, inplace=True)

    DataBlocks = {}

    # Group into blocks, to make sure everything in block has the same prodID, release, etc.
    if args.data:
        ExpGroups = df.groupby(["campaign", "expNumber"])
        for (campaign, expNumber), ExpGroup in ExpGroups:
            groups = ExpGroup.groupby(["beamEnergy", "release", "DBGT", "prodNumber"])
            for (iGroup, ((beamEnergy, release, DBGT, prodNumber), group)) in enumerate(
                groups
            ):
                # Extract integers from columns
                prodNumber = int(re.sub(r"^prod0*", "", prodNumber))
                DBGT = int(re.sub(r"^DB0*", "", DBGT))
                expInteger = int(re.sub(r"^e0*", "", expNumber))

                # If beam energy is not 4S, then point it out in label
                onres = beamEnergy == "4S"
                scan = beamEnergy == "5S_scan"
                if onres:
                    label = f"{campaign}_exp{expInteger}r{iGroup+1}"
                elif scan:
                    # more complicated process to retrieve the exact 5S_scan energy from output filename...
                    pattern = r"_5Sscan_(\d+)"
                    match = re.search(pattern, args.output)
                    scanEnergy = match.group(1)
                    label = f"{campaign}_{beamEnergy}_{scanEnergy}_exp{expInteger}r{iGroup+1}"
                else:
                    label = f"{campaign}_{beamEnergy}_exp{expInteger}r{iGroup+1}"

                if "generalSkimName" in df.columns:
                    generalSkim = list(group["generalSkimName"])[0]
                    if generalSkim == '':
                        generalSkim = 'hadron'
                else:
                    generalSkim = "all"

                # Use sample name encoding from DataSample
                sampleLabel = DataSample(
                    location="DUMMY_PATH",
                    processing=campaign,
                    experiment=expInteger,
                    beam_energy=beamEnergy,
                    general_skim=generalSkim,
                ).encodeable_name

                # Add everything to our mega dict
                DataBlocks[label] = {
                    "sampleLabel": sampleLabel,
                    "LPNPrefix": prefix,
                    "inputReleaseNumber": release,
                    "prodNumber": prodNumber,
                    "inputDBGlobalTag": DBGT,
                    "procNumber": campaign,
                    "experimentNumber": expNumber,
                    "beamEnergy": beamEnergy,
                    "inputDataLevel": "mdst",
                    "runNumbers": list(group["runNumber"]),
                }

                if "generalSkimName" in df.columns:
                    DataBlocks[label]["generalSkimName"] = list(
                        group["generalSkimName"]
                    )[0]
    elif args.mcrd:
        ExpGroups = df.groupby(["campaign", "MCEventType", "expNumber"])
        for (campaign, MCEventType, expNumber), ExpGroup in ExpGroups:
            groups = ExpGroup.groupby(["beamEnergy", "release", "DBGT", "prodNumber"])
            for (iGroup, ((beamEnergy, release, DBGT, prodNumber), group)) in enumerate(
                groups
            ):
                # Extract integers from columns
                prodNumber = int(re.sub(r"^prod0*", "", prodNumber))
                DBGT = int(re.sub(r"^DB0*", "", DBGT))
                expInteger = int(re.sub(r"^s00/e0*", "", expNumber))

                # If beam energy is not 4S, then point it out in label
                onres = beamEnergy == "4S"
                if onres:
                    label = f"{campaign}_exp{expInteger}_{MCEventType}_{prodNumber}r{iGroup+1}"
                else:
                    label = f"{campaign}_{beamEnergy}_exp{expInteger}_{MCEventType}_{prodNumber}r{iGroup+1}"

                # Add everything to our mega dict
                DataBlocks[label] = {
                    "sampleLabel": (f"MC-{campaign}-{beamEnergy}-{MCEventType}-{args.bg}"),
                    "LPNPrefix": prefix,
                    "inputReleaseNumber": release,
                    "mcCampaign": campaign,
                    "prodNumber": prodNumber,
                    "inputDBGlobalTag": DBGT,
                    "experimentNumber": expNumber,
                    "beamEnergy": beamEnergy,
                    "mcType": MCEventType,
                    "mcBackground": args.bg,
                    "inputDataLevel": "mdst",
                    "runNumbers": list(group["runNumber"]),
                }
    else:
        # Extract integers from columns
        df.loc[:, "prodNumber"] = (
            df["prodNumber"].str.replace("^prod0*", "", regex=True).astype(int)
        )

        MCTypeGroups = df.groupby(["campaign", "MCEventType"])
        for (campaign, MCEventType), MCTypeGroup in MCTypeGroups:
            groups = MCTypeGroup.groupby(
                ["beamEnergy", "expNumber", "release", "DBGT", "runNumber"]
            )
            for (
                iGroup,
                ((beamEnergy, expNumber, release, DBGT, runNumber), group),
            ) in enumerate(groups):
                # Extract integers from columns
                DBGT = int(re.sub(r"^DB0*", "", DBGT))

                # If beam energy is not 4S, then point it out in label
                label = f"{campaign}_{MCEventType}{args.bg}"
                BlockLabel = f"{label}r{iGroup+1}"

                # Use sample name encoding from MCSample
                sampleLabel = MCSample(
                    location="DUMMY_PATH",
                    process=MCEventType,
                    campaign=campaign,
                    beam_energy=beamEnergy,
                    beam_background=args.bg,
                ).encodeable_name

                # Add everything to our mega dict
                DataBlocks[BlockLabel] = {
                    "sampleLabel": sampleLabel,
                    "LPNPrefix": prefix,
                    "inputReleaseNumber": release,
                    "mcCampaign": campaign,
                    "prodNumber": list(group["prodNumber"]),
                    "inputDBGlobalTag": DBGT,
                    "experimentNumber": expNumber,
                    "beamEnergy": beamEnergy,
                    "mcType": MCEventType,
                    "mcBackground": args.bg,
                    "inputDataLevel": "mdst",
                    "runNumbers": runNumber,
                }

    to_yaml(DataBlocks, args.output)


if __name__ == "__main__":
    main()
