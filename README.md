## Investigating the latency of QAudioInput

### Goal

Reducing the time between a sound and a DMX value being sent while using the [Sound2Light Tool](https://github.com/ETCLabs/Sound2Light) together with an Eos Console.
To be able to better optimize the latency of the the involved QAudioInput, a minimal working example was created that allows to measure the time between a sound and receiving a value from QAudioInput.

### Measurement Procedure

A video is captured with a microphone in the foreground and the application window in the background. Then the frames are counted between tapping on the mic and seeing a change in the UI. The 	measuring tolerance is 16ms with this technique (when capturing with 60 fps).
