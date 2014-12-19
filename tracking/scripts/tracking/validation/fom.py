#!/usr/bin/env python
# -*- coding: utf-8 -*-

import collections
import ROOT


class ValidationFiguresOfMerit(collections.MutableMapping):

    def __init__(
        self,
        name,
        description='',
        check='',
        contact='',
        ):

        self.name = name
        self.description = description
        self.check = check
        self.contact = contact

        self.figures_by_name = {}

    def __str__(self):
        """Informal sting output listing the assigned figures of merit."""

        return '\n'.join('%s : %s' % (key, value) for (key, value) in
                         self.figures_by_name.items())

    def write(self):
        """Writes the figures of merit as a TNtuple to the currently open TFile in the format complient with the validation frame work."""

        name = self.name
        figure_names = list(self.figures_by_name.keys())
        values = list(self.figures_by_name.values())

        leaf_specification = ':'.join(figure_names)
        title = ''
        ntuple = ROOT.TNtuple(name, title, leaf_specification)
        ntuple.Fill(*values)

        ntuple.SetAlias('Description', self.description)
        ntuple.SetAlias('Check', self.check)
        ntuple.SetAlias('Contact', self.contact)

        ntuple.Write()

    def __setitem__(self, figure_name, value):
        """Braketed item assignement for figures of merit"""

        self.figures_by_name[figure_name] = value

    def __getitem__(self, figure_name):
        """Braketed item lookup for figures of merit"""

        return self.figures_by_name[figure_name]

    def __delitem__(self, figure_name):
        """Braketed item deletion for figures of merit"""

        del self.figures_by_name[figure_name]

    def __iter__(self):
        """Implements the iter() hook as if it was a dictionary."""

        return iter(self.figures_by_name)

    def __len__(self):
        """Returns the number of figures of merit assigned. Implements the len() hook."""

        return len(self.figures_by_name)


