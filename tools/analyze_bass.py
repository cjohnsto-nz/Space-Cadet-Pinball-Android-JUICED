#!/usr/bin/env python3
"""
Bass Analysis Tool for HDR Glow Modulation
Analyzes an audio file and generates a beat map JSON file with bass intensity values.

Requirements:
    pip install librosa numpy

Usage:
    python analyze_bass.py input.mp3 output.json

The output JSON contains timestamps (in milliseconds) and bass intensity values (0.0-1.0)
"""

import sys
import json
import numpy as np

try:
    import librosa
except ImportError:
    print("Error: librosa not installed. Run: pip install librosa numpy")
    sys.exit(1)


def analyze_bass(audio_path, output_path, sample_rate=22050, hop_length=512, 
                 bass_freq_max=250, smoothing_window=5):
    """
    Analyze bass frequencies in an audio file and generate a beat map.
    
    Args:
        audio_path: Path to input audio file (MP3, WAV, FLAC, etc.)
        output_path: Path to output JSON file
        sample_rate: Audio sample rate for analysis
        hop_length: Number of samples between analysis frames
        bass_freq_max: Maximum frequency to consider as "bass" (Hz)
        smoothing_window: Window size for smoothing bass envelope
    """
    print(f"Loading audio: {audio_path}")
    y, sr = librosa.load(audio_path, sr=sample_rate, mono=True)
    
    duration = librosa.get_duration(y=y, sr=sr)
    print(f"Duration: {duration:.2f} seconds")
    
    # Compute Short-Time Fourier Transform
    print("Computing STFT...")
    stft = np.abs(librosa.stft(y, hop_length=hop_length))
    
    # Get frequency bins
    freqs = librosa.fft_frequencies(sr=sr)
    
    # Find bass frequency bin indices (0 to bass_freq_max Hz)
    bass_bins = np.where(freqs <= bass_freq_max)[0]
    
    # Extract bass energy for each frame
    print(f"Extracting bass energy (0-{bass_freq_max} Hz)...")
    bass_energy = np.sum(stft[bass_bins, :], axis=0)
    
    # Apply onset detection to find bass hits
    print("Detecting bass onsets...")
    onset_env = librosa.onset.onset_strength(y=y, sr=sr, hop_length=hop_length,
                                              aggregate=np.median,
                                              fmax=bass_freq_max)
    
    # Combine bass energy with onset strength for better transient detection
    combined = bass_energy * onset_env[:len(bass_energy)]
    
    # Normalize to 0-1 range
    if np.max(combined) > 0:
        combined = combined / np.max(combined)
    
    # Apply smoothing to reduce noise
    if smoothing_window > 1:
        kernel = np.ones(smoothing_window) / smoothing_window
        combined = np.convolve(combined, kernel, mode='same')
    
    # Apply a power curve to make peaks more pronounced
    combined = np.power(combined, 0.7)
    
    # Re-normalize after power curve
    if np.max(combined) > 0:
        combined = combined / np.max(combined)
    
    # Convert frame indices to timestamps (milliseconds)
    frame_times = librosa.frames_to_time(np.arange(len(combined)), 
                                          sr=sr, hop_length=hop_length)
    
    # Downsample to reduce file size (every ~23ms = ~43 samples per second)
    # This is enough resolution for smooth glow modulation
    downsample_factor = max(1, len(combined) // int(duration * 43))
    
    beat_map = []
    for i in range(0, len(combined), downsample_factor):
        timestamp_ms = int(frame_times[i] * 1000)
        intensity = float(combined[i])
        # Only include points with meaningful intensity or at regular intervals
        if intensity > 0.05 or i % (downsample_factor * 10) == 0:
            beat_map.append({
                "t": timestamp_ms,
                "v": round(intensity, 3)
            })
    
    # Create output structure
    output = {
        "version": 1,
        "duration_ms": int(duration * 1000),
        "sample_count": len(beat_map),
        "bass_freq_max": bass_freq_max,
        "beats": beat_map
    }
    
    print(f"Generated {len(beat_map)} data points")
    
    # Write JSON
    with open(output_path, 'w') as f:
        json.dump(output, f, separators=(',', ':'))
    
    print(f"Beat map saved to: {output_path}")
    
    # Print some stats
    intensities = [b["v"] for b in beat_map]
    print(f"\nStats:")
    print(f"  Min intensity: {min(intensities):.3f}")
    print(f"  Max intensity: {max(intensities):.3f}")
    print(f"  Avg intensity: {np.mean(intensities):.3f}")
    print(f"  File size: {len(json.dumps(output, separators=(',', ':'))) / 1024:.1f} KB")


def main():
    if len(sys.argv) < 3:
        print("Usage: python analyze_bass.py <input_audio> <output_json>")
        print("Example: python analyze_bass.py 808generative.mp3 808generative_beats.json")
        sys.exit(1)
    
    audio_path = sys.argv[1]
    output_path = sys.argv[2]
    
    analyze_bass(audio_path, output_path)


if __name__ == "__main__":
    main()
