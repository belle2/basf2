#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Thomas Keck 2015
#
# This sonification module used pyo
# http://ajaxsoundstudio.com/software/pyo/
# You have to install pyo to use it ;-)

# @cond

# Add system python libraries because pyo isn't available via pip
import sys
sys.path.append("/usr/lib/python2.7/dist-packages/")

from pyo import *
from basf2 import *
from ROOT import Belle2
import variables
import time


class Sonic(Module):

    def __init__(self):
        super(Sonic, self).__init__()
        # Defines underlying beat, with probabilities for w1 (down beat), w2 (up-beat), w3 (off-beat)
        self.melody_beat = Beat(time=.125, taps=16, w1=0, w2=0, w3=0, poly=1).play()
        self.melody2_beat = Beat(time=.125, taps=16, w1=0, w2=0, w3=0, poly=1).play()
        self.melody3_beat = Beat(time=.125, taps=16, w1=0, w2=0, w3=0, poly=1).play()
        self.noise_beat = Beat(time=.125, taps=16, w1=0, w2=0, w3=0, poly=1).play()

        # Generate Midi tones beween 60 (~260 Hz) und 96 (~2000Hz), which is 3 octaves
        self.trmid_melody = TrigXnoiseMidi(self.melody_beat, dist=12, mrange=(60, 96))
        # Now "round" the generated tones to the nearest tones in the choice list (octave indepentend)
        self.melody_dur_choice = [0, 2, 4, 5, 7, 9, 11]
        self.melody_moll_choice = [0, 2, 3, 5, 7, 8, 10]
        self.trhz_melody = Snap(self.trmid_melody, choice=self.melody_dur_choice, scale=1)

        # Generate envelope using cosine table for each received beat
        self.trenv_melody = TrigEnv(self.melody_beat, table=CosTable(
            [(0, 0), (8191, 0)]), dur=self.melody_beat['dur'], mul=self.melody_beat['amp'])
        self.trenv_melody2 = TrigEnv(self.melody2_beat, table=CosTable(
            [(0, 0), (8191, 0)]), dur=self.melody2_beat['dur'], mul=self.melody2_beat['amp'])
        self.trenv_melody3 = TrigEnv(self.melody3_beat, table=CosTable(
            [(0, 0), (8191, 0)]), dur=self.melody3_beat['dur'], mul=self.melody3_beat['amp'])
        # Play sine modulated by frequency and amplitude
        self.melody = Sine(
            freq=[i * self.trhz_melody for i in range(1, 5)],
            mul=[i * self.trenv_melody for i in [0.3, 0.02, 0.18, 0.01, 0.15]])
        # Add Terz and Quinte
        self.melody += SDelay(Sine(freq=5.0 / 4.0 * self.trhz_melody, mul=0.3 * self.trenv_melody2), delay=0.005)
        self.melody += SDelay(Sine(freq=3.0 / 2.0 * self.trhz_melody, mul=0.3 * self.trenv_melody3), delay=0.01)
        # Add reverb
        self.melody_verb = Freeverb(self.melody, size=[.1, .1], damp=.5, bal=.3).out()

        # Generate envelope using cosine table for each received beat
        self.trenv_noise = TrigEnv(self.noise_beat, table=CosTable(
            [(0, 0), (8191, 0)]), dur=self.noise_beat['dur'], mul=self.noise_beat['amp'])
        # Play pink noice for the beat
        self.noise = PinkNoise(mul=self.trenv_noise * 0.5).mix(2).out()

    def event(self):

        # Get amount of leptons/mesons/baryons/gammas
        # On real data: could be replaced by FSP lists with some PID cuts
        amount_of_leptons = variables.variables.evaluate('NumberOfMCParticlesInEvent(11, 13)', 0)
        amount_of_mesons = variables.variables.evaluate('NumberOfMCParticlesInEvent(111, 211, 312, 311)', 0)
        amount_of_baryons = variables.variables.evaluate('NumberOfMCParticlesInEvent(2212, 2112)', 0)
        amount_of_gammas = variables.variables.evaluate('NumberOfMCParticlesInEvent(22)', 0)

        print(amount_of_leptons, amount_of_mesons, amount_of_baryons, amount_of_gammas)

        # Check if it's a Y4S Event
        # On real data: could be replaced by FEI or continuum suppresion
        isY4SEvent = not bool(variables.variables.evaluate('isContinuumEvent', 0))

        print(isY4SEvent)

        # Amount of tracks, ecl and klm clusters
        amount_of_tracks = variables.variables.evaluate('nTracks', 0)
        amount_of_ecl_clusters = variables.variables.evaluate('nECLClusters', 0)
        amount_of_klm_clusters = variables.variables.evaluate('nKLMClusters', 0)

        energy_ecl_clusters = variables.variables.evaluate('ECLEnergy', 0)
        energy_klm_clusters = variables.variables.evaluate('KLMEnergy', 0)

        # Amount of mesons, leptons and baryons in final state
        melody_up_beat = min(int(amount_of_leptons * 20.0), 100)  # Leptons
        melody_down_beat = min(int(amount_of_mesons * 10.0), 100)  # Mesons
        melody_off_beat = min(int(amount_of_baryons * 20.0), 100)  # Baryons
        self.melody_beat.setWeights(melody_up_beat, melody_down_beat, melody_off_beat)

        melody_terz_beat = min(int(amount_of_gammas * 5), 100)  # Gammas
        melody_quint_beat = min(int(amount_of_gammas * 5), 100)  # Gammas
        self.melody2_beat.setWeights(melody_terz_beat, 0, 0)
        self.melody3_beat.setWeights(melody_quint_beat, 0, 0)

        # Energy in ecl and klm clusters define tone range
        maximum_step = min(0.3 + energy_klm_clusters / 4.0, 1.0)
        melody_shift = int(energy_ecl_clusters) * 2
        self.trmid_melody.setRange(60 + melody_shift, 84 + melody_shift)
        self.trmid_melody.setX2(maximum_step)

        # KSW moments
        noise_up_beat = 50
        noise_down_beat = [80, 90]
        noise_off_beat = 10
        self.noise_beat.setWeights(noise_up_beat, noise_down_beat, noise_off_beat)
        self.noise_beat.new()

        # Use Dur for Y4S Event and Moll for continuum
        melody_choice = self.melody_dur_choice if isY4SEvent else self.melody_moll_choice
        self.trhz_melody.setChoice(melody_choice)

        # Amount of gammas define hold of melody tones
        melody_hold = max(min(int(amount_of_gammas * 100), 7000), 200)
        melody_table = CosTable([(0, 0), (100, 1), (melody_hold, .2), (8000, 0)])
        self.trenv_melody.setTable(melody_table)
        self.trenv_melody2.setTable(melody_table)
        self.trenv_melody3.setTable(melody_table)

        # Ratio of tracks and tracks+clusters defines roomsize
        verb_size = 1 - amount_of_tracks / (amount_of_tracks + amount_of_ecl_clusters + amount_of_klm_clusters)
        self.melody_verb.setSize(verb_size)

        # Noise table not modified yet
        noise_table = CosTable([(0, 0), (100, 1), (500, .3), (1000, 0)])
        self.trenv_noise.setTable(noise_table)

        # Let it play for at least 5 seconds
        time.sleep(5)


# Create pyo server, tries to connect to pulseaudio, jack or alsa
# s = Server(audio='offline_nb')
s = Server()
# s.recordOptions(dur=60, filename='test.ogg', fileformat=7)
s.boot()
# Start server
s.start()
# s.recstart()

path = create_path()
path.add_module('RootInput')
path.add_module('Gearbox')
path.add_module('ProgressBar')
path.add_module(Sonic())
process(path)

# Shutdown server
# s.recstop()
s.stop()

# @endcond
