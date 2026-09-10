# System Audio Routing Setup

Status: Phase04 foundation. These steps are manual and must be performed by the user on macOS.

## Goal

Route system audio into Local Audio Mixer as one stereo system source:

`source app -> BlackHole 2ch -> native engine -> physical output`

Do not use a Multi-Output Device that sends the original app audio directly to speakers/headphones, because that creates a dry path alongside the processed mixer path.

## Manual Setup

1. Install BlackHole 2ch from the official BlackHole project.
2. Open `Audio MIDI Setup`.
3. Create one Aggregate Device for mixer work.
4. Add BlackHole 2ch, the physical output device, and any mic/interface needed for monitoring.
5. Set all included devices to the project sample rate, initially `48 kHz`.
6. Choose a stable clock source and enable drift correction for non-master devices when needed.
7. Keep the engine output mapped only to the physical output channel pair.
8. Keep outputs that feed BlackHole silent to avoid feedback.
9. After the engine reports route diagnostics as valid, choose BlackHole 2ch as the macOS system output for apps you want to hear through the mixer.
10. To recover manually, set macOS `System Settings -> Sound -> Output` back to the physical speaker/headphones.

## Current Diagnostics

The native engine exposes `routing-system-diagnostics`.

It reports:
- BlackHole availability.
- selected BlackHole input channel range.
- selected physical output channel range.
- sample-rate mismatch.
- rejected loopback output routes.

Current foundation is read-only. It does not install BlackHole, create aggregate devices, or change macOS default output.
