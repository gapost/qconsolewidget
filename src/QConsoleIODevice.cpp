#include "QConsoleIODevice.h"
#include "QConsoleWidget.h"

QConsoleIODevice::QConsoleIODevice(QConsoleWidget *w, QObject *parent)
    : QIODevice(parent), widget_(w),
      readpos_(0), writtenSinceLastEmit_(0), readSinceLastEmit_(0)
{
    setCurrentWriteChannel(QConsoleWidget::StandardOutput);

    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

QConsoleIODevice::~QConsoleIODevice()
{
}

/*!
    \reimp
*/
qint64 QConsoleIODevice::bytesAvailable() const
{
    return (qint64)(readbuff_.size() - readpos_);
}

/*!
    \reimp
*/
qint64 QConsoleIODevice::readData(char *data, qint64 len)
{
    int b = bytesAvailable();
    if (b) {
        b = qMin((int)len, b);
        memcpy(data, readbuff_.constData() + readpos_, b);
        readpos_ += b;
    }
    return (qint64)b;
}
/*!
    \reimp
*/
qint64 QConsoleIODevice::writeData(const char *data, qint64 len)
{
    QByteArray ba(data,(int)len);
    int ch = currentWriteChannel();
    if (ch==QConsoleWidget::StandardError)
        widget_->writeStdErr(ba);
    else
        widget_->writeStdOut(ba);

    writtenSinceLastEmit_ += len;
    if (!signalsBlocked()) {
        emit bytesWritten(writtenSinceLastEmit_);
        writtenSinceLastEmit_ = 0;
    }
    return len;
}

void QConsoleIODevice::consoleWidgetInput(const QString &in)
{
    QByteArray ba = in.toLatin1();
    int sz = ba.size();
    if (bytesAvailable()) {
        if (readpos_) readbuff_ = readbuff_.mid(readpos_);
        readbuff_.append(ba);
    } else {
        readbuff_ = ba;
    }
    readpos_ = 0;

    readSinceLastEmit_ += sz;
    if (!signalsBlocked()) {
        emit readyRead();
        readSinceLastEmit_ = 0;
    }
}

