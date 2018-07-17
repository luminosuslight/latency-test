#include "AudioInput.h"

#include <algorithm>
#include <qendian.h>
#include <QDebug>
#include <qmath.h>


AudioInput::AudioInput(QObject *parent)
    : QObject(parent)
    , m_bufferSize(64)
    , m_rtAudio(new RtAudio())
    , m_maxLevelOfLastChunk(0)
    , m_gloablMaxLevel(0)
    , m_history(qMin(6, 40000 / m_bufferSize), 0)
    , m_historyIndex(0)
    , m_lastChunkSize(0)
{

}

int recordCallback(void* /*out*/, void* inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus status, void* userData) {
    if (status) {
        qWarning() << "Stream overflow detected!";
    }
    QByteArray buffer(reinterpret_cast<const char*>(inputBuffer), nBufferFrames*2*2);
    AudioInput* audioInput = reinterpret_cast<AudioInput*>(userData);
    audioInput->updateFromRawData(buffer);
    return 0;
}

void AudioInput::startCapture(QString /*inputName*/) {
    if (m_rtAudio->getDeviceCount() < 1) {
        qDebug() << "No audio devices found!";
        return;
    }
    for (unsigned int i = 0; i < m_rtAudio->getDeviceCount(); ++i) {
        RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(i);
        qDebug() << QString::fromStdString(info.name);
    }
    RtAudio::StreamParameters parameters;
    parameters.deviceId = m_rtAudio->getDefaultInputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = m_bufferSize;
    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_MINIMIZE_LATENCY;
    options.numberOfBuffers = 2;
    try {
        m_rtAudio->openStream(NULL, &parameters, RTAUDIO_SINT16,
                       sampleRate, &bufferFrames, &recordCallback, this, &options);
        m_rtAudio->startStream();
    } catch (RtAudioError& e) {
        e.printMessage();
    }
}

qreal AudioInput::lastPeakValue() const {
    return *std::max_element(m_history.begin(), m_history.end());
}

void AudioInput::updateFromRawData(QByteArray audioData) {
    const int channelCount = 2;
    const int channelBytes = 2;
    const int sampleBytes = channelCount * channelBytes;
    const int numSamples = audioData.size() / sampleBytes;
    if (numSamples <= 0) return;
    m_lastChunkSize = numSamples;
    emit lastChunkSizeChanged();

    quint32 maxValue = 0;
    const unsigned char* ptr = reinterpret_cast<const unsigned char *>(audioData.data());

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < channelCount; ++j) {
            quint32 value = 0;

            value = qAbs(qFromLittleEndian<qint16>(ptr));

            maxValue = qMax(value, maxValue);
            ptr += channelBytes;
        }
    }

    m_gloablMaxLevel = qMax(maxValue, m_gloablMaxLevel);
    qreal level = m_gloablMaxLevel > 0 ? qreal(maxValue) / m_gloablMaxLevel : 0.0;
    m_history[m_historyIndex] = qPow(level, 0.5);
    m_historyIndex = (m_historyIndex + 1) % m_history.size();
    emit lastPeakValueChanged();
}
