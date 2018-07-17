#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QVector>
#include <QAudioInput>


class AudioInput : public QObject {

    Q_OBJECT

    Q_PROPERTY(qreal lastPeakValue READ lastPeakValue NOTIFY lastPeakValueChanged)
    Q_PROPERTY(int lastChunkSize MEMBER m_lastChunkSize NOTIFY lastChunkSizeChanged)

public:
    explicit AudioInput(QObject* parent = 0);

signals:
    void lastPeakValueChanged();
    void lastChunkSizeChanged();

public slots:
    void startCapture(QString inputName = "");

    int bufferSize() const { return m_bufferSize; }
    qreal lastPeakValue() const;
    QString deviceName() const { return m_deviceInfo.deviceName(); }

private:
    void updateFromRawData(QByteArray audioData);

protected:
    int m_bufferSize;
    QAudioDeviceInfo m_deviceInfo;
    QAudioInput* m_audioInput;
    QAudioFormat m_format;

    qreal m_maxLevelOfLastChunk;
    quint32 m_gloablMaxLevel;
    QVector<qreal> m_history;
    int m_historyIndex;
    int m_lastChunkSize;
};

#endif // AUDIOINPUT_H
