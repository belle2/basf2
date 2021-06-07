#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import re
import sys
from pathlib import Path

import json
import jsonschema
import yaml

from basf2 import find_file


class Sample:
    """Base class for skim test samples."""

    def __init__(self, **kwargs):
        """
        Initialise Sample. Passing any unrecognised keywords will raise an error.
        """
        if kwargs:
            keys = ", ".join(kwargs.keys())
            raise ValueError(
                f"Unrecognised arguments in test sample initialisation: {keys}"
            )

    location = NotImplemented
    """Path of the test file."""

    @property
    def encodeable_name(self):
        """
        Identifying string which is safe to be included as a filename component or as a
        key in the skim stats JSON file.

        As a rough naming convention, data samples should start with 'Data-', MC sample
        with 'MC-', and custom samples with 'Custom-'.
        """
        return NotImplemented

    @property
    def printable_name(self):
        """
        Human-readable name for displaying in printed tables.
        """
        return NotImplemented

    @staticmethod
    def resolve_path(location):
        """
        Replace ``'${SampleDirectory}'`` with ``Sample.SampleDirectory``, and resolve
        the path.

        Parameters:
            location (str, pathlib.Path): Filename to be resolved.

        Returns:
            pathlib.Path: Resolved path.
        """
        SampleDirectory = "/group/belle2/dataprod/MC/SkimTraining"
        location = str(location).replace("${SampleDirectory}", SampleDirectory)
        return Path(location).expanduser().resolve()

    @property
    def as_dict(self):
        """
        Sample serialised as a dictionary.
        """
        return NotImplemented

    def __str__(self):
        return self.encodeable_name


class DataSample(Sample):
    def __init__(
        self,
        *,
        location,
        processing,
        experiment,
        beam_energy="4S",
        general_skim="all",
        **kwargs,
    ):
        # Pass unrecognised kwargs to base class
        super().__init__(**kwargs)

        self.location = self.resolve_path(location)
        self.processing = processing
        if isinstance(experiment, int) or not experiment.startswith("exp"):
            experiment = f"exp{experiment}"
        self.experiment = experiment
        self.beam_energy = beam_energy
        self.general_skim = general_skim

    def __repr__(self):
        return (
            f"{self.__class__.__name__}("
            f"location={repr(self.location)}, "
            f"processing={repr(self.processing)}, "
            f"experiment={repr(self.experiment)}, "
            f"beam_energy={repr(self.beam_energy)}, "
            f"general_skim={repr(self.general_skim)})"
        )

    @property
    def as_dict(self):
        return {
            "location": str(self.location),
            "processing": self.processing,
            "experiment": self.experiment,
            "beam_energy": self.beam_energy,
            "general_skim": self.general_skim,
        }

    @property
    def encodeable_name(self):
        return "-".join(
            (
                "Data",
                self.processing,
                self.experiment,
                self.beam_energy,
                self.general_skim,
            )
        )

    @property
    def printable_name(self):
        name = f"{self.processing} {self.experiment}"
        # Only print additional info in non-default situations
        if self.beam_energy != "4S":
            name += f", {self.beam_energy}"
        if self.general_skim != "all":
            name += f", ({self.general_skim})"
        return name


class MCSample(Sample):
    def __init__(
        self,
        *,
        location,
        process,
        campaign,
        beam_energy="4S",
        beam_background="BGx1",
        **kwargs,
    ):
        # Pass unrecognised kwargs to base class
        super().__init__(**kwargs)

        self.location = self.resolve_path(location)
        self.process = process
        self.beam_energy = beam_energy

        if isinstance(campaign, int) or not campaign.startswith("MC"):
            campaign = f"MC{campaign}"
        self.campaign = campaign

        if isinstance(beam_background, int) or not beam_background.startswith("BGx"):
            beam_background = f"BGx{beam_background}"
        self.beam_background = beam_background

    def __repr__(self):
        return (
            f"{self.__class__.__name__}("
            f"location={repr(self.location)}, "
            f"process={repr(self.process)}, "
            f"campaign={repr(self.campaign)}, "
            f"beam_energy={repr(self.beam_energy)}, "
            f"beam_background={repr(self.beam_background)})"
        )

    @property
    def as_dict(self):
        return {
            "location": str(self.location),
            "process": self.process,
            "campaign": self.campaign,
            "beam_energy": self.beam_energy,
            "beam_background": self.beam_background,
        }

    @property
    def encodeable_name(self):
        return "-".join(
            ("MC", self.campaign, self.beam_energy, self.process, self.beam_background)
        )

    @property
    def printable_name(self):
        name = f"{self.campaign} {self.process}"
        # Only print additional info in non-default situations
        if self.beam_background != "BGx1":
            name += f" {self.beam_background}"
        if self.beam_energy != "4S":
            name += f", {self.beam_energy}"
        return name


class CustomSample(Sample):
    def __init__(self, *, location, label=None, **kwargs):
        # Pass unrecognised kwargs to base class
        super().__init__(**kwargs)

        self.location = self.resolve_path(location)
        if label is None:
            self.label = str(location)
        else:
            self.label = label

        self.sanitised_label = re.sub(r"[^A-Za-z0-9]", "", self.label)

    def __repr__(self):
        return (
            f"{self.__class__.__name__}("
            f"location={repr(self.location)}, "
            f"label={repr(self.label)})"
        )

    @property
    def as_dict(self):
        return {"location": str(self.location), "label": self.label}

    @property
    def encodeable_name(self):
        return f"Custom-{self.sanitised_label}"

    @property
    def printable_name(self):
        return self.label


class TestSampleList:
    """Container class for lists of MC, data, and custom samples."""

    DefaultSampleYAML = (
        "/group/belle2/dataprod/MC/SkimTraining/SampleLists/TestFiles.yaml"
    )

    def __init__(self, *, SampleYAML=None, SampleDict=None, SampleList=None):
        """
        Initialise a list of test samples. Three methods are of initialisation are
        allowed. If no arguments are given this function will default to a standard list
        of samples defined in
        ``/group/belle2/dataprod/MC/SkimTraining/SampleLists/TestFiles.yaml``.

        Parameters:
            SampleYAML (str, pathlib.path): Path to a YAML file containing sample
                specifications.
            SampleDict (dict): Dict containing sample specifications.
            SampleList (list(Sample)): List of Sample objects.
        """
        if sum(p is not None for p in (SampleYAML, SampleDict, SampleList)) > 1:
            raise ValueError(
                "Only one out of SampleYAML, SampleDict, or SampleList can be passed."
            )

        if SampleList is not None:
            # Initialise from list of Sample objects
            self.mc_samples = [s for s in SampleList if isinstance(s, MCSample)]
            self.data_samples = [s for s in SampleList if isinstance(s, DataSample)]
            self.custom_samples = [s for s in SampleList if isinstance(s, CustomSample)]
            return

        if SampleDict is None:
            if SampleYAML is None:
                SampleYAML = self.DefaultSampleYAML

            with open(SampleYAML) as f:
                SampleDict = yaml.safe_load(f)

        self.validate_schema(SampleDict, SampleYAML)

        self._parse_all_samples(SampleDict)

    @property
    def _all_samples(self):
        return [*self.mc_samples, *self.data_samples, *self.custom_samples]

    def __iter__(self):
        yield from self._all_samples

    def __getitem__(self, i):
        return self._all_samples[i]

    def __len__(self):
        return len(self._all_samples)

    def __repr__(self):
        return f"{self.__class__.__name__}(" f"SampleList={repr(list(self))})"

    @property
    def SampleDict(self):
        return {
            "MC": [s.as_dict for s in self.mc_samples],
            "Data": [s.as_dict for s in self.data_samples],
            "Custom": [s.as_dict for s in self.custom_samples],
        }

    def validate_schema(self, SampleDict, InputYAML=None):
        """
        Validate YAML input against JSON schema defined in
        ``skim/tools/resources/test_samples_schema.json``.
        """
        schema_file = find_file("skim/tools/resources/test_samples_schema.json")
        with open(schema_file) as f:
            schema = json.load(f)

        try:
            jsonschema.validate(SampleDict, schema)
        except jsonschema.exceptions.ValidationError as e:
            if InputYAML:
                raise ValueError(
                    f"Error in sample list configuration file {InputYAML}"
                ) from e
            raise e

    @staticmethod
    def _parse_samples(SampleDict, BlockName, SampleClass):
        if SampleDict is None:
            return []

        try:
            InputSampleList = SampleDict[BlockName]
        except KeyError:
            return []

        if InputSampleList is None:
            return []

        samples = []
        for sample in InputSampleList:
            samples.append(SampleClass(**sample))
        return samples

    def _parse_all_samples(self, SampleDict):
        """Read in each block of the YAML and create lists of sample objects."""
        MissingParams = (
            "Error in '{block}' block of test sample yaml file.\n"
            "The following must all have defined values: {params}"
        )

        try:
            self.data_samples = self._parse_samples(SampleDict, "Data", DataSample)
        except TypeError as e:
            required = ", ".join(
                f"'{p}'"
                for p in ("location", "processing", "beam_energy", "experiment")
            )
            raise ValueError(MissingParams.format(block="Data", params=required)) from e

        try:
            self.mc_samples = self._parse_samples(SampleDict, "MC", MCSample)
        except TypeError as e:
            required = ", ".join(f"'{p}'" for p in ("location", "process", "campaign"))
            raise ValueError(MissingParams.format(block="MC", params=required)) from e

        try:
            self.custom_samples = self._parse_samples(
                SampleDict, "Custom", CustomSample
            )
        except TypeError as e:
            required = ", ".join(f"'{p}'" for p in ("location",))
            raise ValueError(
                MissingParams.format(block="Custom", params=required)
            ) from e

    def query_mc_samples(
        self,
        *,
        process=None,
        campaign=None,
        beam_energy=None,
        beam_background=None,
        exact_match=False,
    ):
        """
        Find all MC samples matching query.

        If ``exact_match`` is passed, an error is raised if there is not exactly one
        matching sample. If there is exactly one matching sample, then the single sample
        is returned, rather than a list.
        """
        samples = [
            s
            for s in self.mc_samples
            if (process is None or s.process == process)
            and (campaign is None or s.campaign == campaign)
            and (beam_energy is None or s.beam_energy == beam_energy)
            and (beam_background is None or s.beam_background == beam_background)
        ]
        if exact_match:
            if len(samples) == 1:
                return samples[0]
            else:
                raise ValueError(
                    "`exact_match=True` was specified, but did not find exactly one match."
                )
        else:
            return samples

    def query_data_samples(
        self,
        *,
        processing=None,
        experiment=None,
        beam_energy=None,
        general_skim=None,
        exact_match=False,
    ):
        """
        Find all data samples matching query.

        If ``exact_match`` is passed, an error is raised if there is not exactly one
        matching sample. If there is exactly one matching sample, then the single sample
        is returned, rather than a list.
        """
        samples = [
            s
            for s in self.data_samples
            if (processing is None or s.processing == processing)
            and (experiment is None or s.experiment == experiment)
            and (beam_energy is None or s.beam_energy == beam_energy)
            and (general_skim is None or s.general_skim == general_skim)
        ]
        if exact_match:
            if len(samples) == 1:
                return samples[0]
            else:
                raise ValueError(
                    "`exact_match=True` was specified, but did not find exactly one match."
                )
        else:
            return samples


if __name__ == "__main__":
    # Print the parsed contents of the YAML file
    try:
        samples = TestSampleList(SampleYAML=sys.argv[1])
    except IndexError:
        samples = TestSampleList()

    print("Samples defined in YAML file:")
    for sample in samples:
        print(f"  * {repr(sample)}")
