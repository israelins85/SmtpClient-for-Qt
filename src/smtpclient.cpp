/*
  Copyright (c) 2011-2012 - Tőkés Attila

  This file is part of SmtpClient for Qt.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/

#include "smtpclient.h"

#include <QFileInfo>
#include <QByteArray>
#include <QEventLoop>
#include <QTimer>

Q_LOGGING_CATEGORY(smtpClient, "SmtpClient")

class SmtpClientPrivate {
    Q_DISABLE_COPY(SmtpClientPrivate)
    Q_DECLARE_PUBLIC(SmtpClient)
    SmtpClient * const q_ptr = nullptr;

    QString host = "localhost";
    int port;
    SmtpClient::ConnectionType connectionType;
    QString serverName;

    QMap<QString, QStringList> serverSettings;

    QString user;
    QString password;
    SmtpClient::AuthMethod authMethod = SmtpClient::AuthMethod::Auto;

    int connectionTimeout = 60000;
    int responseTimeout = 5 * 60000;
    int sendMessageTimeout = 60000;

    QString responseText;
    int responseCode;

    QTcpSocket* _socket = nullptr;

    SmtpClientPrivate(SmtpClient* q) : q_ptr(q) {}

    void processServerSettings(QStringList a_setting) {
        if (a_setting.isEmpty()) return;
        serverSettings[a_setting.takeFirst()] = a_setting;
    }

    QTcpSocket* socket() {
        if (_socket) {
            bool isSsl = (qobject_cast<QSslSocket*>(_socket) != nullptr);
            bool destroy = false;

            switch (connectionType) {
            case SmtpClient::ConnectionType::Tcp:
                destroy = isSsl;
                break;
            case SmtpClient::ConnectionType::Tls:
            case SmtpClient::ConnectionType::Ssl:
                destroy = !isSsl;
                break;
            }

            if (destroy) {
                delete _socket;
                _socket = nullptr;
            }
        }

        if (!_socket) {
            Q_Q(SmtpClient);

            switch (connectionType) {
            case SmtpClient::ConnectionType::Tcp:
                _socket = new QTcpSocket(q);
                break;
            case SmtpClient::ConnectionType::Tls:
            case SmtpClient::ConnectionType::Ssl:
                _socket = new QSslSocket(q);
                QObject::connect(_socket, SIGNAL(sslErrors(QList<QSslError>)),
                                 q, SLOT(ignoreSslErrors(QList<QSslError>)));
                break;
            }

            QObject::connect(_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                             q, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
            QObject::connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                             q, SLOT(socketError(QAbstractSocket::SocketError)));
            QObject::connect(_socket, SIGNAL(readyRead()),
                             q, SLOT(socketReadyRead()));
        }

        return _socket;
    }

public:
    ~SmtpClientPrivate() {
        if (_socket) {
            delete _socket;
            _socket = nullptr;
        }
    }
};

/* [1] Constructors and destructors */

SmtpClient::SmtpClient(const QString & host, int port, ConnectionType connectionType) :
    d_ptr(new SmtpClientPrivate(this))
{
    Q_D(SmtpClient);

    d->connectionType = connectionType;
    d->host = host;
    d->port = port;
}

SmtpClient::~SmtpClient() {}

/* [1] --- */


/* [2] Getters and Setters */

void SmtpClient::setUser(const QString &user)
{
    Q_D(SmtpClient);
    d->user = user;
}

void SmtpClient::setPassword(const QString &password)
{
    Q_D(SmtpClient);
    d->password = password;
}

void SmtpClient::setAuthMethod(AuthMethod method)
{
    Q_D(SmtpClient);
    d->authMethod = method;
}

void SmtpClient::setHost(const QString &host)
{
    Q_D(SmtpClient);
    d->host = host;
}

void SmtpClient::setPort(int port)
{
    Q_D(SmtpClient);
    d->port = port;
}

void SmtpClient::setConnectionType(ConnectionType ct)
{
    Q_D(SmtpClient);
    d->connectionType = ct;
}

const QString& SmtpClient::host() const
{
    Q_D(const SmtpClient);
    return d->host;
}

const QString& SmtpClient::user() const
{
    Q_D(const SmtpClient);
    return d->user;
}

const QString& SmtpClient::password() const
{
    Q_D(const SmtpClient);
    return d->password;
}

SmtpClient::AuthMethod SmtpClient::authMethod() const
{
    Q_D(const SmtpClient);
    return d->authMethod;
}

int SmtpClient::port() const
{
    Q_D(const SmtpClient);
    return d->port;
}

SmtpClient::ConnectionType SmtpClient::connectionType() const
{
    Q_D(const SmtpClient);
    return d->connectionType;
}

const QString& SmtpClient::serverName() const
{
    Q_D(const SmtpClient);
    return d->serverName;
}

void SmtpClient::setServerName(const QString &name)
{
    Q_D(SmtpClient);
    d->serverName = name;
}

const QString & SmtpClient::responseText() const
{
    Q_D(const SmtpClient);
    return d->responseText;
}

int SmtpClient::responseCode() const
{
    Q_D(const SmtpClient);
    return d->responseCode;
}

QTcpSocket* SmtpClient::socket() {
    Q_D(SmtpClient);
    return d->socket();
}

int SmtpClient::connectionTimeout() const
{
    Q_D(const SmtpClient);
    return d->connectionTimeout;
}

void SmtpClient::setConnectionTimeout(int msec)
{
    Q_D(SmtpClient);
    d->connectionTimeout = msec;
}

int SmtpClient::responseTimeout() const
{
    Q_D(const SmtpClient);
    return d->responseTimeout;
}

void SmtpClient::setResponseTimeout(int msec)
{
    Q_D(SmtpClient);
    d->responseTimeout = msec;
}

int SmtpClient::sendMessageTimeout() const
{
    Q_D(const SmtpClient);
    return d->sendMessageTimeout;
}

void SmtpClient::setSendMessageTimeout(int msec)
{
    Q_D(SmtpClient);
    d->sendMessageTimeout = msec;
}

bool SmtpClient::connectToHost()
{
    Q_D(SmtpClient);

    qCDebug(smtpClient) << "starting connection";
    QTcpSocket* socket = d->socket();

    switch (d->connectionType)
    {
    case ConnectionType::Tcp:
    case ConnectionType::Tls:
        socket->connectToHost(d->host, d->port);
        break;
    case ConnectionType::Ssl:
        ((QSslSocket*) socket)->connectToHostEncrypted(d->host, d->port);
        break;
    }

    // Tries to connect to server
    if (!socket->waitForConnected(d->connectionTimeout)) {
        emit smtpError(SmtpErrorType::ConnectionTimeout);
        return false;
    }

    qCDebug(smtpClient) << "starting comunication";
    // Wait for the server's response
    // If the response code is not 220 (Service ready)
    // means that is something wrong with the server
    if (!waitForResponse(220, SmtpErrorType::Server)) {
        return false;
    }

    // Send a EHLO/HELO message to the server
    // The client's first command must be EHLO/HELO
    if (d->serverName.isEmpty()) {
        sendMessage("EHLO " + d->host);
    } else {
        sendMessage("EHLO " + d->serverName);
    }

    // Wait for the server's response
    // The response code needs to be 250.
    if (!waitForResponse(250, SmtpErrorType::Server)) {
        return false;
    }

    QSslSocket* sslSocket = qobject_cast<QSslSocket*>(socket);

    if (sslSocket && (sslSocket->mode() == QSslSocket::UnencryptedMode)) {
        // send a request to start TLS handshake
        sendMessage("STARTTLS");

        // Wait for the server's response
        // The response code needs to be 220.
        if (!waitForResponse(220, SmtpErrorType::Server)) {
            return false;
        };

        sslSocket->startClientEncryption();

        if (!sslSocket->waitForEncrypted(d->connectionTimeout)) {
            qCDebug(smtpClient) << socket->errorString();
            emit smtpError(SmtpErrorType::ConnectionTimeout);
            return false;
        }

        // Send ELHO one more time
        if (d->serverName.isEmpty()) {
            sendMessage("EHLO " + d->host);
        } else {
            sendMessage("EHLO " + d->serverName);
        }

        // Wait for the server's response
        // The response code needs to be 250.
        if (!waitForResponse(250, SmtpErrorType::Server)) {
            return false;
        }
    }

    // If no errors occured the function returns true.
    return true;
}

bool SmtpClient::login()
{
    Q_D(const SmtpClient);
    return login(d->user, d->password, d->authMethod);
}

bool SmtpClient::login(const QString &user, const QString &password, AuthMethod method)
{
    if (method == AuthMethod::Auto) {
        Q_D(const SmtpClient);

        QStringList l_suportedAuthMethods = d->serverSettings["AUTH"];

        for (QString l_authMethod : l_suportedAuthMethods) {
            if (l_authMethod.toUpper() == "PLAIN") {
                method = AuthMethod::Plain;
                break;
            }
            if (l_authMethod.toUpper() == "LOGIN") {
                method = AuthMethod::Login;
                break;
            }
        }

        if (method == AuthMethod::Auto) {
            emit smtpError(SmtpError(SmtpErrorType::AuthenticationFailed, 0,
                                     QString("Unsuported Auth Methods. Supported by Server: ") + l_suportedAuthMethods.join(' ')));
            return false;
        }
    }

    if (method == AuthMethod::Plain) {
        // Sending command: AUTH PLAIN
        sendMessage("AUTH PLAIN");

        // Wait for 334 response code
        if (!waitForResponse(334, SmtpErrorType::AuthenticationFailed)) { return false; }

        // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
        sendMessage(QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());
    } else
    if (method == AuthMethod::Login) {
        // Sending command: AUTH LOGIN
        sendMessage("AUTH LOGIN");

        // Wait for 334 response code
        if (!waitForResponse(334, SmtpErrorType::AuthenticationFailed)) { return false; }

        // Send the username in base64
        sendMessage(QByteArray().append(user).toBase64());

        // Wait for 334
        if (!waitForResponse(334, SmtpErrorType::AuthenticationFailed)) { return false; }

        // Send the password in base64
        sendMessage(QByteArray().append(password).toBase64());
    }

    // Wait for the server's responce
    // If the response is not 235 then the authentication was failed
    if (!waitForResponse(235, SmtpErrorType::AuthenticationFailed)) { return false; }

    return true;
}

bool SmtpClient::sendMail(MimeMessage& email)
{
    // Send the MAIL command with the sender
    sendMessage("MAIL FROM:<" + email.sender().address + ">");
    if (!waitForResponse(250, SmtpErrorType::Server)) return false;

    // Send RCPT command for each recipient
    // To (primary recipients)
    for (const EmailAddress& e : email.recipients(MimeMessage::RecipientType::To)) {
        sendMessage("RCPT TO:<" + e.address + ">");
        if (!waitForResponse(250, SmtpErrorType::Server)) return false;
    }

    // Cc (carbon copy)
    for (const EmailAddress& e : email.recipients(MimeMessage::RecipientType::Cc)) {
        sendMessage("RCPT TO:<" + e.address + ">");
        if (!waitForResponse(250, SmtpErrorType::Server)) return false;
    }

    // Bcc (blind carbon copy)
    for (const EmailAddress& e : email.recipients(MimeMessage::RecipientType::Bcc)) {
        sendMessage("RCPT TO:<" + e.address + ">");
        if (!waitForResponse(250, SmtpErrorType::Server)) return false;
    }

    // Send DATA command
    sendMessage("DATA");
    if (!waitForResponse(354, SmtpErrorType::Server)) return false;

    Q_D(SmtpClient);
    if (!email.write(d->socket(), d->sendMessageTimeout)) {
        emit smtpError(SmtpErrorType::SendDataTimeout);
        return false;
    }

    sendMessage("");

    // Send \r\n.\r\n to end the mail data
    sendMessage(".");
    if (!waitForResponse(250, SmtpErrorType::Server)) return false;

    return true;
}

void SmtpClient::quit()
{
    if (!sendMessage("QUIT")) {
        Q_D(SmtpClient);
        QTcpSocket* socket = d->socket();
        // Manually close the connection to the smtp server if message "QUIT" wasn't received by the smtp server
        socket->disconnectFromHost();
    }
}

void SmtpClient::ignoreSslErrors(const QList<QSslError>& errors)
{
    for (QSslError err : errors) {
        qCDebug(smtpClient) << err.errorString();
    }

    Q_D(SmtpClient);
    QTcpSocket* socket = d->socket();
    QSslSocket* sslSocket = qobject_cast<QSslSocket*>(socket);
    if (sslSocket) sslSocket->ignoreSslErrors(errors);
}

bool SmtpClient::waitForResponse(qint32 a_expectedCode, SmtpErrorType a_onErrorType)
{
    Q_D(SmtpClient);
    QTcpSocket* socket = d->socket();

    if (!socket->isOpen()) {
        emit smtpError(SmtpErrorType::Client);
        return false;
    }

    d->responseCode = -1;
    d->responseText.clear();

    if (!socket->isReadable()) {
        emit smtpError(SmtpErrorType::Client);
        return false;
    }

    if (!socket->waitForReadyRead(500)) {
        QEventLoop l_evLoop;
        QTimer l_timer;

        connect(socket, &QTcpSocket::readyRead, &l_evLoop, &QEventLoop::quit);
        connect(&l_timer, &QTimer::timeout, &l_evLoop, [&l_evLoop, this]() {
            emit smtpError(SmtpErrorType::ResponseTimeout);
            l_evLoop.quit();
        });

        l_timer.start(d->responseTimeout);

        l_evLoop.exec();
    }

    while (socket->canReadLine()) {
        // Save the server's response
        d->responseText = socket->readLine();
        if (d->responseText.endsWith("\r\n")) d->responseText.chop(2);
        qCDebug(smtpClient) << "RECV: " << d->responseText;

        // Extract the respose code from the server's responce (first 3 digits)
        d->responseCode = d->responseText.leftRef(3).toInt();
        QString l_text = d->responseText.mid(4);

        if ((d->responseCode == 220) && d->serverName.isEmpty())
            d->serverName = l_text.split(' ').first();

        if (d->responseCode == 250)
            d->processServerSettings(l_text.split(' '));

        if (d->responseCode / 100 == 4)
            emit smtpError(SmtpError(SmtpErrorType::Server, d->responseCode, d->responseText));

        if (d->responseCode / 100 == 5)
            emit smtpError(SmtpError(SmtpErrorType::Client, d->responseCode, d->responseText));

        if (d->responseText[3] == ' ') { break; }
    }

    if (a_expectedCode < 0) return true;

    if (d->responseCode == a_expectedCode)
        return true;

    emit smtpError(SmtpError(a_onErrorType, d->responseCode, d->responseText));
    return false;
}


bool SmtpClient::sendMessage(const QString &text)
{
    return sendMessage(text.toUtf8());
}

bool SmtpClient::sendMessage(const char* text)
{
    return sendMessage(QByteArray(text));
}

bool SmtpClient::sendMessage(const QByteArray& data)
{
    Q_D(SmtpClient);
    QTcpSocket* socket = d->socket();

    if (!socket->isOpen()) {
        return false;
    }

    if (data.size() > 1024)
        qCDebug(smtpClient) << "SEND: " << (data.left(1024 - 3) + "...");
    else
        qCDebug(smtpClient) << "SEND: " << data;

    socket->write(data + "\r\n");
    if (!socket->waitForBytesWritten(d->sendMessageTimeout)) {
        emit smtpError(SmtpErrorType::SendDataTimeout);
        return false;
    }

    return true;
}

void SmtpClient::socketStateChanged(QAbstractSocket::SocketState /*state*/)
{
}

void SmtpClient::socketError(QAbstractSocket::SocketError /*socketError*/)
{
}

void SmtpClient::socketReadyRead()
{
}
