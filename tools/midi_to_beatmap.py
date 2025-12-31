#!/usr/bin/env python3
"""
MIDI to Beat Map Converter
Converts a MIDI file (e.g., bassline or kick track) to a beat map JSON for HDR glow modulation.

Requirements:
    pip install mido

Usage:
    python midi_to_beatmap.py bass.mid output.json

The output JSON contains timestamps (in milliseconds) and intensity values (0.0-1.0)
based on MIDI note velocity.
"""

import sys
import json

try:
    import mido
except ImportError:
    print("Error: mido not installed. Run: pip install mido")
    sys.exit(1)


def midi_to_beatmap(midi_path, output_path, decay_ms=100, offset_ms=0):
    """
    Convert a MIDI file to a beat map JSON.
    
    Args:
        midi_path: Path to input MIDI file
        output_path: Path to output JSON file
        decay_ms: How long (ms) for the intensity to decay after a note-on
        offset_ms: Time offset to apply to all timestamps (negative = earlier)
    """
    print(f"Loading MIDI: {midi_path}")
    mid = mido.MidiFile(midi_path)
    
    # Get tempo (default 120 BPM if not specified)
    tempo = 500000  # microseconds per beat (default 120 BPM)
    for track in mid.tracks:
        for msg in track:
            if msg.type == 'set_tempo':
                tempo = msg.tempo
                break
    
    bpm = mido.tempo2bpm(tempo)
    print(f"Tempo: {bpm:.1f} BPM")
    
    # Collect all note-on events with their absolute times
    note_events = []
    
    for track in mid.tracks:
        abs_time = 0  # in ticks
        for msg in track:
            abs_time += msg.time
            if msg.type == 'note_on' and msg.velocity > 0:
                # Convert ticks to milliseconds
                time_ms = int(mido.tick2second(abs_time, mid.ticks_per_beat, tempo) * 1000)
                # Apply offset
                time_ms = max(0, time_ms + offset_ms)
                # Normalize velocity (0-127) to intensity (0.0-1.0)
                intensity = msg.velocity / 127.0
                note_events.append((time_ms, intensity))
    
    # Sort by time
    note_events.sort(key=lambda x: x[0])
    
    if not note_events:
        print("Warning: No note-on events found in MIDI file!")
        return False
    
    print(f"Found {len(note_events)} note events")
    
    # Get duration
    duration_ms = max(e[0] for e in note_events) + 1000  # Add 1 second buffer
    
    # Generate beat map with INSTANT attack and decay
    # For each note: instant jump to full intensity, then linear decay
    beat_map = []
    
    for i, (note_time, intensity) in enumerate(note_events):
        # Add point just before note (at 0 intensity) for instant attack
        if i == 0 or note_time - note_events[i-1][0] > decay_ms:
            # Only add zero point if there's a gap (not back-to-back notes)
            if note_time > 0:
                beat_map.append({"t": note_time - 1, "v": 0.0})
        
        # Instant attack - full intensity at note time
        beat_map.append({"t": note_time, "v": round(intensity, 3)})
        
        # Calculate when decay ends (or next note starts)
        decay_end = note_time + decay_ms
        if i + 1 < len(note_events):
            # Don't decay past the next note
            decay_end = min(decay_end, note_events[i + 1][0] - 1)
        
        # Add decay end point (back to 0)
        if decay_end > note_time:
            beat_map.append({"t": decay_end, "v": 0.0})
    
    # Create output structure
    output = {
        "version": 1,
        "duration_ms": duration_ms,
        "sample_count": len(beat_map),
        "source": "midi",
        "tempo_bpm": round(bpm, 1),
        "beats": beat_map
    }
    
    print(f"Generated {len(beat_map)} data points")
    
    # Write JSON
    with open(output_path, 'w') as f:
        json.dump(output, f, separators=(',', ':'))
    
    print(f"Beat map saved to: {output_path}")
    
    # Print stats
    intensities = [b["v"] for b in beat_map if b["v"] > 0]
    if intensities:
        print(f"\nStats:")
        print(f"  Note events: {len(note_events)}")
        print(f"  Duration: {duration_ms / 1000:.1f} seconds")
        print(f"  Max intensity: {max(intensities):.3f}")
        print(f"  Avg intensity: {sum(intensities) / len(intensities):.3f}")
        print(f"  File size: {len(json.dumps(output, separators=(',', ':'))) / 1024:.1f} KB")
    
    return True


def main():
    if len(sys.argv) < 3:
        print("Usage: python midi_to_beatmap.py <input.mid> <output.json>")
        print("Example: python midi_to_beatmap.py bass.mid 808generative_beats.json")
        print("\nOptions:")
        print("  --decay <ms>   Decay time in milliseconds (default: 150)")
        print("  --offset <ms>  Time offset in milliseconds (default: 0, negative = earlier)")
        sys.exit(1)
    
    midi_path = sys.argv[1]
    output_path = sys.argv[2]
    
    # Parse optional decay argument
    decay_ms = 200
    if '--decay' in sys.argv:
        idx = sys.argv.index('--decay')
        if idx + 1 < len(sys.argv):
            decay_ms = int(sys.argv[idx + 1])
    
    # Parse optional offset argument
    offset_ms = 0
    if '--offset' in sys.argv:
        idx = sys.argv.index('--offset')
        if idx + 1 < len(sys.argv):
            offset_ms = int(sys.argv[idx + 1])
    
    midi_to_beatmap(midi_path, output_path, decay_ms, offset_ms)


if __name__ == "__main__":
    main()
