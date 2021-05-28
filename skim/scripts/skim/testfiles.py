import re
from pathlib import Path

import yaml


class Sample:
    """Base class for skim test samples."""

    def __init__(self, **kwargs):
        """
        Initialise Sample. Passing any unrecognised keywords will raise an error.
        """
        if kwargs:
            keys = ", ".join(kwargs.keys())
            raise ValueError(f"Unrecognised arguments in test sample YAML file: {keys}")

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
        self.experiment = str(experiment)
        self.beam_energy = beam_energy
        self.general_skim = general_skim

    def __repr__(self):
        return (
            f"{self.__class__.__module__}.{self.__class__.__name__}("
            f"location={repr(self.location)},"
            f"processing={repr(self.processing)},"
            f"experiment={repr(self.experiment)},"
            f"beam_energy={repr(self.beam_energy)},"
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
        return (
            f"{self.processing} exp{self.experiment} (beam={self.beam_energy}, "
            f"GeneralSkim={self.general_skim})"
        )


class MCSample(Sample):
    def __init__(self, *, location, process, campaign, background="BGx1", **kwargs):
        # Pass unrecognised kwargs to base class
        super().__init__(**kwargs)

        self.location = self.resolve_path(location)
        self.process = process

        if isinstance(campaign, int) or not campaign.startswith("MC"):
            campaign = f"MC{campaign}"
        self.campaign = campaign

        if isinstance(background, int) or not background.startswith("BGx"):
            background = f"BGx{background}"
        self.background = background

    def __repr__(self):
        return (
            f"{self.__class__.__module__}.{self.__class__.__name__}("
            f"location={repr(self.location)}, "
            f"process={repr(self.process)}, "
            f"campaign={repr(self.campaign)}, "
            f"background={repr(self.background)})"
        )

    @property
    def as_dict(self):
        return {
            "location": str(self.location),
            "process": self.process,
            "campaign": self.campaign,
            "background": self.background,
        }

    @property
    def encodeable_name(self):
        return "-".join(("MC", self.campaign, self.process, self.background))

    @property
    def printable_name(self):
        return " ".join((self.campaign, self.process, self.background))


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
            f"{self.__class__.__module__}.{self.__class__.__name__}("
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
        return (
            f"{self.__class__.__module__}.{self.__class__.__name__}("
            f"SampleList={repr(list(self))})"
        )

    @property
    def SampleDict(self):
        return {
            "MC": [s.as_dict for s in self.mc_samples],
            "Data": [s.as_dict for s in self.data_samples],
            "Custom": [s.as_dict for s in self.custom_samples],
        }

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
