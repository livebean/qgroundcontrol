/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009 - 2011 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Brief Description
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#ifndef SERIALLINK_H
#define SERIALLINK_H

class LinkInterface;
class SerialConfiguration;
class SerialLink;

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>

#ifdef __android__
#include "qserialport.h"
#else
#include <QSerialPort>
#endif
#include <QMetaType>
#include <QLoggingCategory>

// We use QSerialPort::SerialPortError in a signal so we must declare it as a meta type
Q_DECLARE_METATYPE(QSerialPort::SerialPortError)

#include "QGCConfig.h"
#include "LinkManager.h"

Q_DECLARE_LOGGING_CATEGORY(SerialLinkLog)

class SerialConfiguration : public LinkConfiguration
{
    Q_OBJECT

public:

    SerialConfiguration(const QString& name);
    SerialConfiguration(SerialConfiguration* copy);

    int  baud()         { return _baud; }
    int  dataBits()     { return _dataBits; }
    int  flowControl()  { return _flowControl; }    ///< QSerialPort Enums
    int  stopBits()     { return _stopBits; }
    int  parity()       { return _parity; }         ///< QSerialPort Enums

    const QString portName() { return _portName; }

    void setBaud        (int baud);
    void setDataBits    (int databits);
    void setFlowControl (int flowControl);          ///< QSerialPort Enums
    void setStopBits    (int stopBits);
    void setParity      (int parity);               ///< QSerialPort Enums
    void setPortName    (const QString& portName);

    static QStringList supportedBaudRates();

    /// From LinkConfiguration
    LinkType type() { return LinkConfiguration::TypeSerial; }
    void copyFrom(LinkConfiguration* source);
    void loadSettings(QSettings& settings, const QString& root);
    void saveSettings(QSettings& settings, const QString& root);
    void updateSettings();

private:
    static void _initBaudRates();

private:
    int _baud;
    int _dataBits;
    int _flowControl;
    int _stopBits;
    int _parity;
    QString _portName;
};


/**
 * @brief The SerialLink class provides cross-platform access to serial links.
 * It takes care of the link management and provides a common API to higher
 * level communication layers. It is implemented as a wrapper class for a thread
 * that handles the serial communication. All methods have therefore to be thread-
 * safe.
 *
 */
class SerialLink : public LinkInterface
{
    Q_OBJECT

    friend class SerialConfiguration;
    friend class LinkManager;

public:
    // LinkInterface

    LinkConfiguration* getLinkConfiguration();
    QString getName() const;
    void    requestReset();
    bool    isConnected() const;
    qint64  getConnectionSpeed() const;
    bool    requiresUSBMavlinkStart(void) const;

    // These are left unimplemented in order to cause linker errors which indicate incorrect usage of
    // connect/disconnect on link directly. All connect/disconnect calls should be made through LinkManager.
    bool    connect(void);
    bool    disconnect(void);

public slots:

    void readBytes();
    /**
     * @brief Write a number of bytes to the interface.
     *
     * @param data Pointer to the data byte array
     * @param size The size of the bytes array
     **/
    void writeBytes(const char* data, qint64 length);

    void linkError(QSerialPort::SerialPortError error);

protected:
    QSerialPort* _port;
    quint64 _bytesRead;
    int     _timeout;
    QMutex  _dataMutex;       // Mutex for reading data from _port
    QMutex  _writeMutex;      // Mutex for accessing the _transmitBuffer.

private slots:
    void _readBytes(void);

private:
    // Links are only created/destroyed by LinkManager so constructor/destructor is not public
    SerialLink(SerialConfiguration* config);
    ~SerialLink();

    // From LinkInterface
    virtual bool _connect(void);
    virtual void _disconnect(void);

    // Internal methods
    void _emitLinkError(const QString& errorMsg);
    bool _hardwareConnect(QSerialPort::SerialPortError& error, QString& errorString);
    bool _isBootloader();
    void _resetConfiguration();

    // Local data
    volatile bool        _stopp;
    volatile bool        _reqReset;
    QMutex               _stoppMutex;      // Mutex for accessing _stopp
    QByteArray           _transmitBuffer;  // An internal buffer for receiving data from member functions and actually transmitting them via the serial port.
    SerialConfiguration* _config;

signals:
    void aboutToCloseFlag();

};

#endif // SERIALLINK_H
