#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from string import Formatter
from pathlib import Path
import re
import json


def string(some_string):
    """
    Used to escape user strings for LaTex.
        @param some_string which is escaped
        @return string escaped version of some_string
    """
    return some_string.replace('\\', r'\textbackslash').replace('_', r'\_').replace('^', r'\^{}')


def variable(variable_string):
    """
    Adds hyphenations after brackets, and for common variables.
        @param variable_string variable name
        @return string with hyphenation hints for latex
    """
    substitutes = {
        '=': r'=\allowbreak ',
        '_': r'\_\allowbreak ',
        ':': r':\allowbreak ',
        '(': r'(\allowbreak ',
        'extraInfo': r'ex\-tra\-In\-fo',
        'SignalProbability': r'Sig\-nal\-Prob\-a\-bil\-i\-ty',
        'cosAngleBetweenMomentumAndVertexVector': r'cosAngle\-Between\-Momentum\-And\-Vertex\-Vector'}
    for key, value in substitutes.items():
        variable_string = variable_string.replace(key, value)
    return variable_string


def decayDescriptor(decay_string):
    """
    Prettifies the given decay string by using latex-symbols instead of plain-text
        @param decay_string string containing a decay descriptor
        @return string latex version of the decay descriptor
    """
    decay_string = decay_string.replace(':generic', '')
    decay_string = decay_string.replace(':semileptonic', '$_{SL}$')
    decay_string = decay_string.replace(':FSP', '$_{FSP}$')
    decay_string = decay_string.replace(':V0', '$_{V0}$')
    # Note: these are applied from top to bottom, so if you have
    # both B0 and anti-B0, put anti-B0 first.
    substitutes = [
        ('==>', '$\\to$'),
        ('gamma', r'$\gamma$'),
        ('p+', r'$p$'),
        ('anti-p-', r'$\bar{p}$'),
        ('pi+', r'$\pi^+$'),
        ('pi-', r'$\pi^-$'),
        ('pi0', r'$\pi^0$'),
        ('K_S0', r'$K^0_S$'),
        ('K_L0', r'$K^0_L$'),
        ('mu+', r'$\mu^+$'),
        ('mu-', r'$\mu^-$'),
        ('tau+', r'$\tau^+$'),
        ('tau-', r'$\tau^-$'),
        ('nu', r'$\nu$'),
        ('K+', r'$K^+$'),
        ('K-', r'$K^-$'),
        ('e+', r'$e^+$'),
        ('e-', r'$e^-$'),
        ('J/psi', r'$J/\psi$'),
        ('anti-Lambda_c-', r'$\Lambda^{-}_{c}$'),
        ('anti-Sigma+', r'$\overline{\Sigma}^{+}$'),
        ('anti-Lambda0', r'$\overline{\Lambda}^{0}$'),
        ('anti-D0*', r'$\overline{D^{0*}}$'),
        ('anti-D*0', r'$\overline{D^{0*}}$'),
        ('anti-D0', r'$\overline{D^0}$'),
        ('anti-B0', r'$\overline{B^0}$'),
        ('Sigma+', r'$\Sigma^{+}$'),
        ('Lambda_c+', r'$\Lambda^{+}_{c}$'),
        ('Lambda0', r'$\Lambda^{0}$'),
        ('D+', r'$D^+$'),
        ('D-', r'$D^-$'),
        ('D0', r'$D^0$'),
        ('D*+', r'$D^{+*}$'),
        ('D*-', r'$D^{-*}$'),
        ('D*0', r'$D^{0*}$'),
        ('D_s+', r'$D^+_s$'),
        ('D_s-', r'$D^-_s$'),
        ('D_s*+', r'$D^{+*}_s$'),
        ('D_s*-', r'$D^{-*}_s$'),
        ('B+', r'$B^+$'),
        ('B-', r'$B^-$'),
        ('B0', r'$B^0$'),
        ('B_s0', r'$B^0_s$'),
        ('K*0', r'$K^{0*}$')]
    tex_string = decay_string
    for (key, value) in substitutes:
        tex_string = tex_string.replace(key, value)
    return '\\texorpdfstring{%s}{%s}' % (tex_string, string(decay_string))


def duration(seconds):
    """
    Converts a duration given in seconds into a nice latex-style duration string
        @param seconds duration in seconds
        @return string describing a duration in a natural format
    """
    minutes = int(seconds / 60)
    hours = int(minutes / 60)
    minutes %= 60
    ms = int(seconds * 1000) % 1000
    us = int(seconds * 1000 * 1000) % 1000
    seconds = int(seconds % 60)
    string = ''
    if hours != 0:
        string += "%dh" % (hours)
    if minutes != 0:
        string += "%dm" % (minutes)
    if seconds != 0 and hours == 0:
        string += "%ds" % (seconds)
    if ms != 0 and hours == 0 and minutes == 0 and seconds == 0:
        string += "%dms" % (ms)
    if us != 0 and hours == 0 and minutes == 0 and seconds == 0 and ms == 0:
        string += r"%d$\mu$s" % (us)

    if hours == 0 and minutes == 0 and seconds == 0 and ms == 0 and us == 0:
        string += r'$<1\mu$s'
    return string


class AttrDict:
    """Simple wrapper class to allow accessing dictionary elements via
    attribute access"""

    def __init__(self, content):
        """Remember the dictionary"""
        #: Dictionary we want to access via attributes
        self.__content = content

    def __getattr__(self, key):
        """Return any dictionay element as attribute"""
        return self.__content[key]


class DefaultValueFormatter(Formatter):
    """Custom formatter class which allows to substitute a missing value with a
    default value"""

    def format_field(self, value, spec):
        """
        Format a single field:

        * if the field is None and we have a ``:=`` specification we replace
          the field with the default value and change the spec to 's'
        * if the value is None we replace it with an empty string

        then we just run the normal formatter ...
        """
        if spec.startswith("="):
            if value is None:
                value = spec[1:]
            spec = "s"
        if value is None:
            value = ""
            spec = ""
        return super().format_field(value, spec)

    def get_field(self, field_name, args, kwargs):
        """
        Try to get the field as usual but in case we cannot find it because it
        either doesn't exist or it doesn't have the correct attribute/item we
        just return None
        """
        try:
            return super().get_field(field_name, args, kwargs)
        except (KeyError, TypeError):
            return None, None


def format_filename(template, filename, metadata_json):
    """
    Format a file name as described in BELLE2-NOTE-TE-2017-012

    Parameters:
      template (str): the format string
      filename (str): the name of the file
      metadata_json (str): a string representation of the file metadata json

    Returns:
      a string with the formatted file name
    """
    filepath = Path(filename)
    metadata = json.loads(metadata_json)
    for key, value in metadata.items():
        if isinstance(value, dict):
            metadata[key] = AttrDict(value)
    formatter = DefaultValueFormatter()
    result = formatter.format(template, file=filepath, **metadata)
    result = re.sub(r'//+', "/", result)
    result = re.sub(r'\s+', '_', result)
    return result
