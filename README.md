## Investigating the latency of QAudioInput

### Goal

Reducing the time between a sound and a DMX value being sent while using the [Sound2Light Tool](https://github.com/ETCLabs/Sound2Light) together with an Eos Console.
To be able to better optimize the latency of the the involved QAudioInput, a minimal working example was created that allows to measure the time between a sound and receiving a value from QAudioInput.

### Measurement Procedure

A video is captured with a microphone in the foreground and the application window in the background. Then the frames are counted between tapping on the mic and seeing a change in the UI. The 	measuring tolerance is 16ms with this technique (when capturing with 60 fps).

## Insights

### Theoretic Worst Case Latency

(trying to detect low frequency using FFT, using Qt default values, 44.1 kHz sampling rate)

| Source | Latency |
| --- | --- |
| *Initial Sound* | begin |
| A/D Conversion and Hardware Buffer | ? ms |
| Software Buffer in USB and Device Driver | ? ms |
| QAudioInput Buffer (default: ~1760 Samples) | 40 ms |
| Fill whole FFT Window (2048 Samples) | 46 ms |
| Refresh Timer (60 Hz) | 16 ms |
| Computation Time | <5 ms |
| Sending OSC over Network | <5 ms |
| DMX or GUI Update (~40 Hz) | 25 ms |
| *Change in brightness of light* | end |

-> Theoretic Worst Case Latency: ? + 137 ms

### Actual Measurement

Time between sound and UI change in EOS software (Sound2Light / Luminosus and Eos running on different computers, connected using ethernet)

| Setup | Latency |
| --- | --- |
| S2L, Win 10, cheap USB Soundcard | 216 ms |
| Luminosus, Win 10, cheap USB Soundcard | 160 ms |
| Luminosus, Linux, ALSA, cheap USB Soundcard | 160 ms |

(There was no delay between the UI change in Sound2Light and Eos visible -> the network connection seems to be negligible.)
