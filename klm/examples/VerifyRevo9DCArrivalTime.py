#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to verify the getRevo9DCArrivalTime() function implementation.
This script reads KLMDigitRaw data, extracts Revo9DCArrivalTime values, and creates
distributions.
"""

import basf2 as b2
from ROOT import Belle2, TH1F, TCanvas, gStyle
import sys


class Revo9DCArrivalTimeVerificationModule(b2.Module):
    """
    Module to verify getRevo9DCArrivalTime() implementation by creating Revo9DCArrivalTime distributions
    from KLMDigitRaws and comparing with expected results.
    """

    def __init__(self):
        super().__init__()
        self.hist_Revo9DCArrivalTime_raw = None
        self.hist_Revo9DCArrivalTime_final = None
        self.hist_Revo9DCArrivalTime_final_ns = None
        self.hist_Revo9DCArrivalTime_diff = None

    def initialize(self):
        """Initialize histograms."""
        # Step 1: Raw Revo9Trig - word2 (before correction)
        self.hist_Revo9DCArrivalTime_raw = TH1F(
            "h_Revo9DCArrivalTime_raw",
            "Step 1: Revo9TriggerWord - word2 (Raw);Revo9 Trig - DatCon Arrival (TTD Clocks);Counts",
            650, -1000, 65000
        )

        # Step 4: Final Revo9DCArrivalTime distribution (negative times)
        self.hist_Revo9DCArrivalTime_final = TH1F(
            "h_Revo9DCArrivalTime_final",
            "Final Revo9DCArrivalTime Distribution;Hit Arrival Time in DataCon w.r.t. L1 Trig (TTD Clocks);Counts",
            140, -1400, 400
        )
        # Final Revo9DCArrivalTime in nanoseconds
        self.hist_Revo9DCArrivalTime_final_ns = TH1F(
            "h_Revo9DCArrivalTime_final_ns",
            "Final Revo9DCArrivalTime Distribution;Hit Arrival Time in DataCon w.r.t. L1 Trig (ns);Counts",
            140, -1400 * 7.861, 400 * 7.861
        )

        # Comparison histograms
        self.hist_Revo9DCArrivalTime_diff = TH1F(
            'h_Revo9DCArrivalTime_diff',
            'Revo9DCArrivalTime Difference (function_logic_implementation - KLMDigit_implementation);Difference (clocks);Counts',
            100, -10, 10
        )

        # Constants for Revo9DCArrivalTime calculation
        self.FRAME9_MAX = 11520
        self.TTD_PERIOD_NS = 7.861

    def event(self):
        """Process each event."""
        # Get KLMDigits (not KLMDigitRaws - we need geometry info)
        digits = Belle2.PyStoreArray('KLMDigits')
        digit_event_info_array = Belle2.PyStoreArray('KLMDigitEventInfos')

        if digit_event_info_array.getEntries() == 0:
            return

        # Get first event info entry
        digit_event_info = digit_event_info_array[0]

        # Get trigger Revo9DCArrivalTime
        triggerTime = digit_event_info.getRevo9TriggerWord()

        if triggerTime == 0:
            return

        # Process each Digit
        for digit in digits:
            # Check if it's an RPC hit
            if not digit.inRPC():
                continue

            # Get related KLMDigitRaw for raw data
            digit_raw_rels = digit.getRelationsTo('KLMDigitRaws')
            if digit_raw_rels.size() == 0:
                continue

            digit_raw = digit_raw_rels[0]
            ctime = digit_raw.getCtime()

            # Raw difference (before correction)
            FRAME = self.FRAME9_MAX  # e.g. 11520
            raw_diff = (int(triggerTime) - int(ctime)) % FRAME
            self.hist_Revo9DCArrivalTime_raw.Fill(raw_diff)

            # Correct for overflows to get final Revo9DCArrivalTime
            halfFRAME = FRAME // 2   # = 5760 threshold
            Revo9DCArrivalTime_clock = int(ctime) - int(triggerTime)

            if (Revo9DCArrivalTime_clock >= 0) and (Revo9DCArrivalTime_clock > halfFRAME):
                Revo9DCArrivalTime_clock = Revo9DCArrivalTime_clock - FRAME

            elif (Revo9DCArrivalTime_clock < 0) and (Revo9DCArrivalTime_clock < -halfFRAME):
                # cases: where hit is very early compared to trigger
                print(f"WARNING: Revo9DCArrivalTime_clock={Revo9DCArrivalTime_clock} for ctime={ctime}, triggerTime={triggerTime}")

            # from KLMUnpacker module
            self.hist_Revo9DCArrivalTime_diff.Fill(digit.getRevo9DCArrivalTime() - Revo9DCArrivalTime_clock)
            # Fill final histograms
            self.hist_Revo9DCArrivalTime_final.Fill(Revo9DCArrivalTime_clock)
            self.hist_Revo9DCArrivalTime_final_ns.Fill(Revo9DCArrivalTime_clock * self.TTD_PERIOD_NS)

    def terminate(self):
        """Save histograms and create comparison plots."""
        from ROOT import TFile

        output_file = TFile("Revo9DCArrivalTime_verification_log.root", "RECREATE")

        # Save all histograms
        self.hist_Revo9DCArrivalTime_raw.Write()
        self.hist_Revo9DCArrivalTime_final.Write()
        self.hist_Revo9DCArrivalTime_final_ns.Write()
        bin1 = self.hist_Revo9DCArrivalTime_final.FindBin(-1400)
        bin2 = self.hist_Revo9DCArrivalTime_final.FindBin(400)
        print("INFO: Count valid entries between -1400 and 400 in final Revo9DCArrivalTime histogram:",
              self.hist_Revo9DCArrivalTime_final.Integral(bin1, bin2))
        print("INFO: Count underflow entries < -1400 in final Revo9DCArrivalTime histogram:",
              self.hist_Revo9DCArrivalTime_final.Integral(0, 1))
        # Create canvas with comparison
        gStyle.SetOptStat(1111)
        canvas = TCanvas("c_Revo9DCArrivalTime_steps", "Revo9DCArrivalTime Verification Steps", 1800, 600)
        canvas.Divide(2, 2)

        canvas.cd(1)
        canvas.GetPad(1).SetLogy()
        self.hist_Revo9DCArrivalTime_raw.Draw()

        canvas.cd(2)
        canvas.GetPad(2).SetLogy()
        self.hist_Revo9DCArrivalTime_final.Draw()

        canvas.cd(3)
        canvas.GetPad(3).SetLogy()
        self.hist_Revo9DCArrivalTime_final_ns.Draw()

        canvas.cd(4)
        self.hist_Revo9DCArrivalTime_diff.Draw()

        canvas.Write()
        canvas.SaveAs("Revo9DCArrivalTime_verification_log.pdf")

        output_file.Close()

        # Print statistics
        b2.B2INFO("=" * 60)
        b2.B2INFO("Revo9DCArrivalTime Verification Results:")
        b2.B2INFO("=" * 60)
        b2.B2INFO(f"Total RPC hits processed: {int(self.hist_Revo9DCArrivalTime_final.GetEntries())}")
        b2.B2INFO("Final Revo9DCArrivalTime distribution:")
        b2.B2INFO(f"  Mean: {self.hist_Revo9DCArrivalTime_final.GetMean():.2f} TTD clocks")
        b2.B2INFO(f"  RMS:  {self.hist_Revo9DCArrivalTime_final.GetRMS():.2f} TTD clocks")
        b2.B2INFO("=" * 60)
        b2.B2INFO("Output files created:")
        b2.B2INFO("  - Revo9DCArrivalTime_verification_log.root")
        b2.B2INFO("  - Revo9DCArrivalTime_verification_log.pdf")
        b2.B2INFO("=" * 60)


def create_path(input_file):
    """
    Create basf2 path for Revo9DCArrivalTime verification.

    Args:
        input_file: Path to input ROOT file with raw data
    """
    path = b2.Path()

    # Input module
    root_input = b2.register_module('RootInput')
    root_input.param('inputFileName', input_file)
    path.add_module(root_input)

    # Unpack KLM data
    unpacker = b2.register_module('KLMUnpacker')
    unpacker.param('WriteDigitRaws', True)  # Important: need raw data
    path.add_module(unpacker)

    # Add our verification module
    path.add_module(Revo9DCArrivalTimeVerificationModule())

    # Progress
    progress = b2.register_module('Progress')
    path.add_module(progress)

    return path


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: basf2 VerifyRevo9DCArrivalTime.py <input_raw_data_file.root>")
        sys.exit(1)

    input_file = sys.argv[1]

    # Create and run path
    path = create_path(input_file)
    b2.process(path)
