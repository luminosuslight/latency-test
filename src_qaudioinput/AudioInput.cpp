#include "AudioInput.h"

#include <algorithm>
#include <qendian.h>
#include <QDebug>
#include <qmath.h>


AudioInput::AudioInput(QObject *parent)
    : QObject(parent)
    , m_bufferSize(20000)
    , m_audioInput(nullptr)
    , m_maxLevelOfLastChunk(0)
    , m_gloablMaxLevel(0)
    , m_history(qMin(6, 40000 / m_bufferSize), 0)
    , m_historyIndex(0)
    , m_lastChunkSize(0)
{

}

void AudioInput::startCapture(QString inputName) {
    // Close and delete previous input device:
    if (m_audioInput) {
        m_audioInput->stop();
    }
    delete m_audioInput;
    m_audioInput = nullptr;

    // Get device info of new input:
    m_deviceInfo = QAudioDeviceInfo::defaultInputDevice();
    if (!inputName.isEmpty()) {
        QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        for (QAudioDeviceInfo device: devices) {
            if (device.deviceName() == inputName) {
                m_deviceInfo = device;
            }
        }
    }

    // check if desired format is supported:
    m_format.setSampleRate(44100);
    m_format.setChannelCount(2);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    if (!m_deviceInfo.isFormatSupported(m_format)) {
        qWarning() << "Default audio format not supported, trying to use the nearest.";
        m_format = m_deviceInfo.nearestFormat(m_format);
    }

    // create new input:
    m_audioInput = new QAudioInput(m_deviceInfo, m_format, this);
    m_audioInput->setVolume(1.0);
    m_audioInput->setBufferSize(m_bufferSize * m_format.bytesPerFrame());
    QIODevice* ioDevice = m_audioInput->start();
    connect(ioDevice, &QIODevice::readyRead, [this, ioDevice](){
        qint64 len = this->m_audioInput->bytesReady();
        if (len <= 0) return;
        QByteArray buffer = ioDevice->read(len);
        updateFromRawData(buffer);
    });
}

qreal AudioInput::lastPeakValue() const {
    return *std::max_element(m_history.begin(), m_history.end());
}

void AudioInput::updateFromRawData(QByteArray audioData) {
    const int channelBytes = m_format.sampleSize() / 8;
    const int sampleBytes = m_format.channelCount() * channelBytes;
    const int numSamples = audioData.size() / sampleBytes;
    if (numSamples <= 0) return;
    m_lastChunkSize = numSamples;
    emit lastChunkSizeChanged();

    quint32 maxValue = 0;
    const unsigned char* ptr = reinterpret_cast<const unsigned char *>(audioData.data());

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < m_format.channelCount(); ++j) {
            quint32 value = 0;

            if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                value = *reinterpret_cast<const quint8*>(ptr);
            } else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt) {
                value = qAbs(*reinterpret_cast<const qint8*>(ptr));
            } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                    value = qFromLittleEndian<quint16>(ptr);
                else
                    value = qFromBigEndian<quint16>(ptr);
            } else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt) {
                if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                    value = qAbs(qFromLittleEndian<qint16>(ptr));
                else
                    value = qAbs(qFromBigEndian<qint16>(ptr));
            } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::UnSignedInt) {
                if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                    value = qFromLittleEndian<quint32>(ptr);
                else
                    value = qFromBigEndian<quint32>(ptr);
            } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::SignedInt) {
                if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                    value = qAbs(qFromLittleEndian<qint32>(ptr));
                else
                    value = qAbs(qFromBigEndian<qint32>(ptr));
            } else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::Float) {
                value = qAbs(*reinterpret_cast<const float*>(ptr) * 0x7fffffff); // assumes 0-1.0
            }

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
