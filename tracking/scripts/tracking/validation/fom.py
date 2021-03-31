#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import collections
import array
import ROOT

from tracking.root_utils import root_cd, root_save_name
from tracking.validation.matplotting import get_logger


class ValidationFiguresOfMerit(collections.MutableMapping):
    """Create and write an TNtuple of the validation figures of merit"""

    def __init__(
        self,
        name,
        description='',
        check='',
        contact='',
        title='',
    ):
        """Constructor"""

        #: cached name for this figure of merit
        self.name = root_save_name(name)
        #: cached description for this figure of merit
        self.description = description
        #: cached user-check action for this figure of merit
        self.check = check
        #: cached contact person for this figure of merit
        self.contact = contact
        #: cached title for this figure of merit
        self.title = title

        #: cached dictionary of figures for this figure of merit
        self.figures_by_name = collections.OrderedDict()

    def __str__(self):
        """Informal string output listing the assigned figures of merit."""

        figures_by_name = self.figures_by_name
        return '\n'.join('%s : %s' % (key, figures_by_name[key])
                         for key in
                         figures_by_name.keys())

    def write(self, tdirectory=None):
        """Writes the figures of merit as a TNtuple.

        Parameters
        ----------
        tdirectory : ROOT.TDirectory, optional
            The directory to which the TNtuple shall be written.
            Defaults to the current directory.
        """
        name = self.name

        if not self.figures_by_name:
            get_logger().warning('Do not create Ntuple for empty ValidationFiguresOfMerit %s' % name)
            return

        title = self.title or name
        contact = self.contact

        description = self.description
        check = self.check

        figure_names = [root_save_name(key) for key in list(self.figures_by_name.keys())]
        values = list(self.figures_by_name.values())

        with root_cd(tdirectory) as tdirectory:
            # Try to find the object first
            tntuple = tdirectory.Get(name)
            if tntuple:
                former_description = tntuple.GetAlias('Description')
                former_check = tntuple.GetAlias('Check')
                former_figure_names = []
                former_values = []
                tntuple.GetEntry(0)
                for tleaf in tntuple.GetListOfLeaves():
                    former_figure_names.append(tleaf.GetName())
                    former_values.append(tleaf.GetValue())

                # Append the description and check of this figure of merit to whatever is there
                description = former_description + ' <br/>\n' + description
                check = former_check + ' <br/>\n' + check

                figure_names = former_figure_names + figure_names
                values = former_values + values

                # Need both delete and overwrite to get rid of the former object.
                tdirectory.Delete(name)
                write_option = ROOT.TObject.kOverwrite

            else:
                write_option = 0

            leaf_specification = ':'.join(figure_names)
            tntuple = ROOT.TNtuple(name, title, leaf_specification)

            array_of_values = array.array('f', values)
            tntuple.Fill(array_of_values)

            tntuple.SetAlias('Description', description)
            tntuple.SetAlias('Check', check)
            tntuple.SetAlias('Contact', contact)

            # Overwrite the former TNtuple if one was there
            tntuple.Write("", write_option)

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


class ValidationManyFiguresOfMerit(ValidationFiguresOfMerit):
    """Create and write an TNtuple with several validation figures of merit"""

    def __str__(self):
        """Describe myself"""
        return 'Not supported.'

    def write(self, tdirectory=None):
        """Writes the figures of merit as a TNtuple.

        Parameters
        ----------
        tdirectory : ROOT.TDirectory, optional
            The directory to which the TNtuple shall be written.
            Defaults to the current directory.
        """
        name = self.name
        figure_names = [root_save_name(key) for key in list(self.figures_by_name.keys())]
        values = list(self.figures_by_name.values())

        leaf_specification = ':'.join(figure_names)
        title = self.title or ""
        ntuple = ROOT.TNtuple(name, title, leaf_specification)

        for value in zip(*values):
            ntuple.Fill(*value)

        ntuple.SetAlias('Description', self.description)
        ntuple.SetAlias('Check', self.check)
        ntuple.SetAlias('Contact', self.contact)

        with root_cd(tdirectory):
            ntuple.Write()
