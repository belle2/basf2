#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import unittest
import numpy as np
import pandas as pd
from pidvar import Reweighter
import itertools


class TestPIDVar(unittest.TestCase):
    """Test case of PIDVar module"""

    def __init__(self, *args, **kwargs):
        """Constructor"""
        super().__init__(*args, **kwargs)
        cosTheta_min = [-0.7, -0.5,   0, 0.2, 0.5, 0.8, 0.9]
        cosTheta_max = [-0.5,    0, 0.2, 0.5, 0.8, 0.9, 0.95]
        cosTheta_var = [list(c) for c in zip(cosTheta_min, cosTheta_max)]
        p_min = [0.5, 1, 2, 3, 4, 5]
        p_max = [1, 2, 3, 4, 5, 6]
        charge_var = [[-2, 0], [0, 2]]
        p_var = [list(p) for p in zip(p_min, p_max)]
        eff_binning = np.array([list(c) for c in itertools.product(p_var, cosTheta_var, charge_var)]).reshape(-1, 6)
        p_min = [0.5, 1, 2, 3, 4]
        p_max = [1, 2, 3, 4, 5]
        p_var = [list(p) for p in zip(p_min, p_max)]
        fake_binning = np.array([list(c) for c in itertools.product(p_var, cosTheta_var, charge_var)]).reshape(-1, 6)
        np.random.seed(0)
        eff_df = pd.DataFrame(eff_binning, columns=['p_min', 'p_max', 'cosTheta_min', 'cosTheta_max', 'charge_min', 'charge_max'])
        eff_df['data_MC_ratio'] = np.random.normal(1, 0.1, size=len(eff_df))
        eff_df['data_MC_uncertainty_stat_dn'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_stat_up'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_sys_up'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['data_MC_uncertainty_sys_dn'] = np.random.uniform(0.001, 0.01, size=len(eff_df))
        eff_df['variable'] = 'electronID'
        eff_df['threshold'] = 0.8

        fake_df = pd.DataFrame(fake_binning, columns=['p_min', 'p_max', 'cosTheta_min', 'cosTheta_max', 'charge_min', 'charge_max'])
        fake_df['data_MC_ratio'] = np.random.normal(2, 0.2, size=len(fake_df))
        fake_df['data_MC_uncertainty_stat_dn'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_stat_up'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_sys_up'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['data_MC_uncertainty_sys_dn'] = np.random.uniform(0.01, 0.02, size=len(fake_df))
        fake_df['variable'] = 'electronID'
        fake_df['threshold'] = 0.8
        #: PID Tables for testing
        self.tables = {(11, 11): eff_df, (11, 211): fake_df}

        user_data_dict = {
            'cosTheta': np.array([0.25, 0.55, 0.85, 0.925]),
            'p': np.array([1.5, 2.5, 3.5, 4.5]),
            'charge': np.array([-1, 1, 1, 1]),
            'electronID': np.array([0.85, 0.9, 0.95, 0.975]),
            'PDG': np.array([11, -11, -11, -11]),
            'mcPDG': np.array([11, 211, -11, 211])
        }
        #: User data for testing
        self.user_data = pd.DataFrame(user_data_dict)
        print(self.tables)
        print(self.user_data)

    def test_merge_tables(self):
        """Tests merging of PID tables"""
        reweighter = Reweighter()
        thresholds = {11: ('electronID', 0.8)}
        merged_table = reweighter.merge_pid_weight_tables(self.tables, thresholds)
        self.assertEqual(len(merged_table), 154)
        self.assertIn('PDG', merged_table.columns)
        self.assertIn('mcPDG', merged_table.columns)
        with self.assertRaises(ValueError):
            wrong_thresholds = {11: ('electronID', 0.9)}
            reweighter.merge_pid_weight_tables(self.tables, wrong_thresholds)

    def test_binning(self):
        """Tests binning of PID tables"""
        reweighter = Reweighter()
        binning = reweighter.get_binning(self.tables[(11, 11)])
        print(binning)
        self.assertEqual(len(binning), 3)
        self.assertCountEqual(binning.keys(), ['p', 'cosTheta', 'charge'])
        self.assertEqual(len(binning['p']), 7)
        self.assertEqual(len(binning['cosTheta']), 8)
        self.assertEqual(len(binning['charge']), 3)
        binning = reweighter.get_binning(self.tables[(11, 211)])
        self.assertEqual(len(binning), 3)
        self.assertEqual(len(binning['p']), 6)

    def test_add_pid_particle(self):
        """Tests adding PID particle"""
        reweighter = Reweighter()
        thresholds = {11: ('electronID', 0.8)}
        reweighter.add_pid_particle('', self.tables, thresholds)
        particle = reweighter.get_particle('')
        self.assertIsNotNone(particle)
        self.assertEqual(len(particle.merged_table), 154)
        self.assertCountEqual(particle.get_binning_variables(), ['p', 'cosTheta', 'charge'])
        print(particle)
        with self.assertRaises(ValueError):
            reweighter.add_pid_particle('', self.tables, thresholds)

    def test_reweight(self):
        """Tests reweighting of PID particle"""
        n_variations = 100
        reweighter = Reweighter(n_variations=n_variations)
        thresholds = {11: ('electronID', 0.8)}
        reweighter.add_pid_particle('', self.tables, thresholds)
        local_data = self.user_data.copy(deep=True)
        reweighter.reweight(local_data)
        self.assertEqual(len(local_data), 4)
        self.assertIn('Weight', local_data.columns)
        cols = [f'Weight_{i}' for i in range(0, n_variations)]
        self.assertCountEqual(cols, [col for col in local_data.columns if col.startswith('Weight_')])
        self.assertTrue((local_data.query('abs(mcPDG) == 11')[['Weight']+cols] < 1.5).all().all())
        self.assertTrue((local_data.query('abs(mcPDG) != 11')[['Weight']+cols] > 1.5).all().all())
        print(local_data.query('abs(mcPDG) != 11')[['Weight']+cols].std(axis=1))
        self.assertTrue((local_data.query('abs(mcPDG) == 11')[['Weight']+cols].std(axis=1) < 0.015).all())
        self.assertTrue((local_data.query('abs(mcPDG) != 11')[['Weight']+cols].std(axis=1) > 0.015).all())


if __name__ == '__main__':
    unittest.main()
