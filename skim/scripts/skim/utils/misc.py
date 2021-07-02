#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Miscellaneous utility functions for skim experts.
"""

import subprocess
import json
import re
from pathlib import Path

from skim.registry import Registry


def get_file_metadata(filename):
    """
    Retrieve the metadata for a file using ``b2file-metadata-show``.

    Parameters:
       metadata (str): File to get number of events from.

    Returns:
        dict: Metadata of file in dict format.
    """
    if not Path(filename).exists():
        raise FileNotFoundError(f"Could not find file {filename}")

    proc = subprocess.run(
        ["b2file-metadata-show", "--json", str(filename)],
        stdout=subprocess.PIPE,
        check=True,
    )
    metadata = json.loads(proc.stdout.decode("utf-8"))
    return metadata


def get_eventN(filename):
    """
    Retrieve the number of events in a file using ``b2file-metadata-show``.

    Parameters:
       filename (str): File to get number of events from.

    Returns:
        int: Number of events in the file.
    """
    return int(get_file_metadata(filename)["nEvents"])


def resolve_skim_modules(SkimsOrModules, *, LocalModule=None):
    """
    Produce an ordered list of skims, by expanding any Python skim module names into a
    list of skims in that module. Also produce a dict of skims grouped by Python module.

    Raises:
        RuntimeError: Raised if a skim is listed twice.
        ValueError: Raised if ``LocalModule`` is passed and skims are normally expected
            from more than one module.
    """
    skims = []

    for name in SkimsOrModules:
        if name in Registry.names:
            skims.append(name)
        elif name in Registry.modules:
            skims.extend(Registry.get_skims_in_module(name))

    duplicates = set([skim for skim in skims if skims.count(skim) > 1])
    if duplicates:
        raise RuntimeError(
            f"Skim{'s'*(len(duplicates)>1)} requested more than once: {', '.join(duplicates)}"
        )

    modules = sorted({Registry.get_skim_module(skim) for skim in skims})
    if LocalModule:
        if len(modules) > 1:
            raise ValueError(
                f"Local module {LocalModule} specified, but the combined skim expects "
                "skims from more than one module. No steering file written."
            )
        modules = {LocalModule.rstrip(".py"): sorted(skims)}
    else:
        modules = {
            module: sorted(
                [skim for skim in skims if Registry.get_skim_module(skim) == module]
            )
            for module in modules
        }

    return skims, modules


def _sphinxify_decay(decay_string):
    """Format the given decay string by using LaTeX commands instead of plain-text.
    Output is formatted for use with Sphinx (ReStructured Text).

    This is a utility function for autogenerating skim documentation.

    Parameters:
        decay_string (str): A decay descriptor.

    Returns:
        sphinxed_string (str): LaTeX version of the decay descriptor.
    """

    decay_string = re.sub("^(B.):generic", "\\1_{\\\\text{had}}", decay_string)
    decay_string = decay_string.replace(":generic", "")
    decay_string = decay_string.replace(":semileptonic", "_{\\text{SL}}")
    decay_string = decay_string.replace(":FSP", "_{FSP}")
    decay_string = decay_string.replace(":V0", "_{V0}")
    decay_string = re.sub("_[0-9]+", "", decay_string)
    # Note: these are applied from top to bottom, so if you have
    # both B0 and anti-B0, put anti-B0 first.
    substitutes = [
        ("==>", "\\to"),
        ("->", "\\to"),
        ("gamma", "\\gamma"),
        ("p+", "p"),
        ("anti-p-", "\\bar{p}"),
        ("pi+", "\\pi^+"),
        ("pi-", "\\pi^-"),
        ("pi0", "\\pi^0"),
        ("K_S0", "K^0_S"),
        ("K_L0", "K^0_L"),
        ("mu+", "\\mu^+"),
        ("mu-", "\\mu^-"),
        ("tau+", "\\tau^+"),
        ("tau-", "\\tau^-"),
        ("nu", "\\nu"),
        ("K+", "K^+"),
        ("K-", "K^-"),
        ("e+", "e^+"),
        ("e-", "e^-"),
        ("J/psi", "J/\\psi"),
        ("anti-Lambda_c-", "\\Lambda^{-}_{c}"),
        ("anti-Sigma+", "\\overline{\\Sigma}^{+}"),
        ("anti-Lambda0", "\\overline{\\Lambda}^{0}"),
        ("anti-D0*", "\\overline{D}^{0*}"),
        ("anti-D*0", "\\overline{D}^{0*}"),
        ("anti-D0", "\\overline{D}^0"),
        ("anti-B0", "\\overline{B}^0"),
        ("Sigma+", "\\Sigma^{+}"),
        ("Lambda_c+", "\\Lambda^{+}_{c}"),
        ("Lambda0", "\\Lambda^{0}"),
        ("D+", "D^+"),
        ("D-", "D^-"),
        ("D0", "D^0"),
        ("D*+", "D^{+*}"),
        ("D*-", "D^{-*}"),
        ("D*0", "D^{0*}"),
        ("D_s+", "D^+_s"),
        ("D_s-", "D^-_s"),
        ("D_s*+", "D^{+*}_s"),
        ("D_s*-", "D^{-*}_s"),
        ("B+", "B^+"),
        ("B-", "B^-"),
        ("B0", "B^0"),
        ("B_s0", "B^0_s"),
        ("K*0", "K^{0*}"),
    ]
    tex_string = decay_string
    for (key, value) in substitutes:
        tex_string = tex_string.replace(key, value)
    return f":math:`{tex_string}`"


def fancy_skim_header(SkimClass):
    """Decorator to generate a fancy header to skim documentation and prepend it to the
    docstring. Add this just above the definition of a skim.

    Also ensures the documentation of the template functions like `BaseSkim.build_lists`
    is not repeated in every skim documentation.

    .. code-block:: python

        @fancy_skim_header
        class MySkimName(BaseSkim):
            # docstring here describing your skim, and explaining cuts.
    """
    SkimName = SkimClass.__name__
    SkimCode = Registry.encode_skim_name(SkimName)
    authors = SkimClass.__authors__ or ["(no authors listed)"]
    description = SkimClass.__description__ or "(no description)"
    contact = SkimClass.__contact__ or "(no contact listed)"
    category = SkimClass.__category__ or "(no category listed)"

    if isinstance(authors, str):
        # If we were given a string, split it up at: commas, "and", "&", and newlines
        authors = re.split(
            r",\s+and\s+|\s+and\s+|,\s+&\s+|\s+&\s+|,\s+|\s*\n\s*", authors
        )
        # Strip any remaining whitespace either side of an author's name
        authors = [re.sub(r"^\s+|\s+$", "", author) for author in authors]

    if isinstance(category, list):
        category = ", ".join(category)

    # If the contact is of the form "NAME <EMAIL>" or "NAME (EMAIL)", then make it a link
    match = re.match("([^<>()`]+) [<(]([^<>()`]+@[^<>()`]+)[>)]", contact)
    if match:
        name, email = match[1], match[2]
        contact = f"`{name} <mailto:{email}>`_"

    header = f"""
    Note:
        * **Skim description**: {description}
        * **Skim name**: {SkimName}
        * **Skim LFN code**: {SkimCode}
        * **Category**: {category}
        * **Author{"s"*(len(authors) > 1)}**: {", ".join(authors)}
        * **Contact**: {contact}
    """

    if SkimClass.ApplyHLTHadronCut:
        HLTLine = "*This skim includes a selection on the HLT flag* ``hlt_hadron``."
        header = f"{header.rstrip()}\n\n        {HLTLine}\n"

    if SkimClass.__doc__:
        SkimClass.__doc__ = header + "\n\n" + SkimClass.__doc__.lstrip("\n")
    else:
        # Handle case where docstring is empty, or was not redefined
        SkimClass.__doc__ = header

    # If documentation of template functions not redefined, make sure BaseSkim docstring is not repeated
    SkimClass.load_standard_lists.__doc__ = SkimClass.load_standard_lists.__doc__ or ""
    SkimClass.build_lists.__doc__ = SkimClass.build_lists.__doc__ or ""
    SkimClass.validation_histograms.__doc__ = (
        SkimClass.validation_histograms.__doc__ or ""
    )
    SkimClass.additional_setup.__doc__ = SkimClass.additional_setup.__doc__ or ""

    return SkimClass
