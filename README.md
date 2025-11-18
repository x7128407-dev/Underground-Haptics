# Underground-Haptics
This repository contains the hardware designs, firmware, and example data supporting the Underground-Haptics system. The project integrates TENS-based haptic actuation with signal sensing and calibration pipelines for robust underground or embedded tactile communication.

Repository Structure
/circuit
Contains PCB CAD design files for:
- TENS-Receiver Module — captures and conditions incoming TENS signals
- TENS-Sender Module — generates stimulation waveforms for haptic actuation
- Sensing Module — handles real-time signal acquisition for feedback and calibration

/code
Includes firmware and processing scripts for:
- Calibration — routines for sensor alignment, signal normalization, and system tuning
- Haptic Actuation — control logic and actuation patterns for TENS-based feedback

/data
Provides example datasets demonstrating:
- Signal recordings
- Calibration samples
- Reference actuation outputs

Getting Started
- Review the PCB files in the circuit folder to fabricate or inspect hardware modules.
- Flash or run the scripts in the code directory to operate the system.
- Use the data folder to test or validate your setup with example inputs.
