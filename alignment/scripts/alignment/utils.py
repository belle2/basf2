#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


class AlignmentGeneratorConfig:
    """ Class to generate data for AlignmentGenerator module
    Can be used to train VXD misalignment by storing wrong alignment
    and trying to reach nominal values (zeros)
    """

    def __init__(self):
        """ Constructor """
        #: alignment data = list of VxdID mask, param id, generation distribution and value
        self.alignment = []

    def get_name(self):
        """ get object name """
        return self.name

    def get_data(self):
        """ get alignment data """
        return self.alignment

    def align_sensors(self, vxdid, param_id, distribution, value):
        """ align sensors by VxdID pattern """
        self.alignment.append(str(vxdid + ',' + str(param_id) + ',' + distribution + ',' + str(value)))

    def gen_u_gaus(self, vxdid, value):
        """ generate u coordinate shift for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 1, 'gaus', value)

    def gen_v_gaus(self, vxdid, value):
        """ generate v coordinate shift for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 2, 'gaus', value)

    def gen_w_gaus(self, vxdid, value):
        """ generate w coordinate shift for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 3, 'gaus', value)

    def gen_alpha_gaus(self, vxdid, value):
        """ generate alpha rotation for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 4, 'gaus', value)

    def gen_beta_gaus(self, vxdid, value):
        """ generate beta rotation for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 5, 'gaus', value)

    def gen_gamma_gaus(self, vxdid, value):
        """ generate gamma rotation coordinate shift for VxdID mask with gausian distribution of given width """
        self.align_sensors(vxdid, 6, 'gaus', value)

    def gen_u_uniform(self, vxdid, value):
        """ generate u coordinate shift for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 1, 'gaus', value)

    def gen_v_uniform(self, vxdid, value):
        """ generate v coordinate shift for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 2, 'gaus', value)

    def gen_w_uniform(self, vxdid, value):
        """ generate w coordinate shift for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 3, 'gaus', value)

    def gen_alpha_uniform(self, vxdid, value):
        """ generate alpha rotation for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 4, 'gaus', value)

    def gen_beta_uniform(self, vxdid, value):
        """ generate beta rotation for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 5, 'gaus', value)

    def gen_gamma_uniform(self, vxdid, value):
        """ generate gamma rotation coordinate shift for VxdID mask with uniform distribution of given width """
        self.align_sensors(vxdid, 6, 'gaus', value)

    def set_u(self, vxdid, value):
        """ set u coordinate shift for VxdID """
        self.align_sensors(vxdid, 1, 'fix', value)

    def set_v(self, vxdid, value):
        """ set v coordinate shift for VxdID """
        self.align_sensors(vxdid, 2, 'fix', value)

    def set_w(self, vxdid, value):
        """ set w coordinate shift for VxdID """
        self.align_sensors(vxdid, 3, 'fix', value)

    def set_alpha(self, vxdid, value):
        """ set alpha angle for VxdID mask """
        self.align_sensors(vxdid, 4, 'fix', value)

    def set_beta(self, vxdid, value):
        """ set beta angle for VxdID mask  """
        self.align_sensors(vxdid, 5, 'fix', value)

    def set_gamma(self, vxdid, value):
        """ set gamma angle for VxdID mask """
        self.align_sensors(vxdid, 6, 'fix', value)

    def set_all(self, vxdid, value):
        """ Set all shifts/rotation to given value (usually 0.) for VxdID mask """
        self.set_u(vxdid, value)
        self.set_v(vxdid, value)
        self.set_w(vxdid, value)
        self.set_alpha(vxdid, value)
        self.set_beta(vxdid, value)
        self.set_gamma(vxdid, value)
