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


/* [1] Constructors and destructors */

SmtpClient::SmtpClient(const QString & host, int port, ConnectionType connectionType) :
    socket(nullptr),
    name("localhost"),
    authMethod(AuthPlain),
    connectionTimeout(60000),
    responseTimeout(5 * 60000),
    sendMessageTimeout(60000)
{
    setConnectionType(connectionType);

    this->host = host;
    this->port = port;

    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
}

SmtpClient::~SmtpClient() {
    if (socket)
        delete socket;
}

/* [1] --- */


/* [2] Getters and Setters */

void SmtpClient::setUser(const QString &user)
{
    this->user = user;
}

void SmtpClient::setPassword(const QString &password)
{
    this->password = password;
}

void SmtpClient::setAuthMethod(AuthMethod method)
{
    this->authMethod = method;
}

void SmtpClient::setHost(const QString &host)
{
    this->host = host;
}

void SmtpClient::setPort(int port)
{
    this->port = port;
}

void SmtpClient::setConnectionType(ConnectionType ct)
{
    this->connectionType = ct;

    if (socket)
        delete socket;

    switch (connectionType)
    {
    case TcpConnection:
        socket = new QTcpSocket(this);
        break;
    case SslConnection:
    case TlsConnection:
        socket = new QSslSocket(this);
    }
}

const QString& SmtpClient::getHost() const
{
    return this->host;
}

const QString& SmtpClient::getUser() const
{
    return this->user;
}

const QString& SmtpClient::getPassword() const
{
    return this->password;
}

SmtpClient::AuthMethod SmtpClient::getAuthMethod() const
{
    return this->authMethod;
}

int SmtpClient::getPort() const
{
    return this->port;
}

SmtpClient::ConnectionType SmtpClient::getConnectionType() const
{
    return connectionType;
}

const QString& SmtpClient::getName() const
{
    return this->name;
}

void SmtpClient::setName(const QString &name)
{
    this->name = name;
}

const QString & SmtpClient::getResponseText() const
{
    return responseText;
}

int SmtpClient::getResponseCode() const
{
    return responseCode;
}

QTcpSocket* SmtpClient::getSocket() {
    return socket;
}

int SmtpClient::getConnectionTimeout() const
{
    return connectionTimeout;
}

void SmtpClient::setConnectionTimeout(int msec)
{
    connectionTimeout = msec;
}

int SmtpClient::getResponseTimeout() const
{
    return responseTimeout;
}

void SmtpClient::setResponseTimeout(int msec)
{
    responseTimeout = msec;
}
int SmtpClient::getSendMessageTimeout() const
{
  return sendMessageTimeout;
}
void SmtpClient::setSendMessageTimeout(int msec)
{
  sendMessageTimeout = msec;
}

/* [2] --- */


/* [3] Public methods */

bool SmtpClient::connectToHost()
{
    switch (connectionType)
    {
    case TlsConnection:
    case TcpConnection:
        socket->connectToHost(host, port);
        break;
    case SslConnection:
        ((QSslSocket*) socket)->connectToHostEncrypted(host, port);
        break;

    }

    // Tries to connect to server
    if (!socket->waitForConnected(connectionTimeout))
    {
        emit smtpError(ConnectionTimeoutError);
        return false;
    }

    try
    {
        // Wait for the server's response
        waitForResponse();

        // If the response code is not 220 (Service ready)
        // means that is something wrong with the server
        if (responseCode != 220)
        {
            emit smtpError(ServerError);
            return false;
        }

        // Send a EHLO/HELO message to the server
        // The client's first command must be EHLO/HELO
        sendMessage("EHLO " + name);

        // Wait for the server's response
        waitForResponse();

        // The response code needs to be 250.
        if (responseCode != 250) {
            emit smtpError(ServerError);
            return false;
        }

        if (connectionType == TlsConnection) {
            // send a request to start TLS handshake
            sendMessage("STARTTLS");

            // Wait for the server's response
            waitForResponse();

            // The response code needs to be 220.
            if (responseCode != 220) {
                emit smtpError(ServerError);
                return false;
            };

            ((QSslSocket*) socket)->startClientEncryption();

            if (!((QSslSocket*) socket)->waitForEncrypted(connectionTimeout)) {
                qCDebug(smtpClient) << ((QSslSocket*) socket)->errorString();
                emit smtpError(ConnectionTimeoutError);
                return false;
            }

            // Send ELHO one more time
            sendMessage("EHLO " + name);

            // Wait for the server's response
            waitForResponse();

            // The response code needs to be 250.
            if (responseCode != 250) {
                emit smtpError(ServerError);
                return false;
            }
        }
    }
    catch (ResponseTimeoutException)
    {
        return false;
    }
    catch (SendMessageTimeoutException)
    {
        return false;
    }

    // If no errors occured the function returns true.
    return true;
}

bool SmtpClient::login()
{
    return login(user, password, authMethod);
}

bool SmtpClient::login(const QString &user, const QString &password, AuthMethod method)
{
    try {
        if (method == AuthPlain)
        {
            // Sending command: AUTH PLAIN
            sendMessage("AUTH PLAIN");

            // Wait for 334 response code
            waitForResponse();
            if (responseCode != 334) { emit smtpError(AuthenticationFailedError); return false; }

            // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
            sendMessage(QByteArray().append((char) 0).append(user).append((char) 0).append(password).toBase64());

            // Wait for the server's response
            waitForResponse();

            // If the response is not 235 then the authentication was faild
            if (responseCode != 235)
            {
                emit smtpError(AuthenticationFailedError);
                return false;
            }
        }
        else if (method == AuthLogin)
        {
            // Sending command: AUTH LOGIN
            sendMessage("AUTH LOGIN");

            // Wait for 334 response code
            waitForResponse();
            if (responseCode != 334) { emit smtpError(AuthenticationFailedError); return false; }

            // Send the username in base64
            sendMessage(QByteArray().append(user).toBase64());

            // Wait for 334
            waitForResponse();
            if (responseCode != 334) { emit smtpError(AuthenticationFailedError); return false; }

            // Send the password in base64
            sendMessage(QByteArray().append(password).toBase64());

            // Wait for the server's responce
            waitForResponse();

            // If the response is not 235 then the authentication was faild
            if (responseCode != 235)
            {
                emit smtpError(AuthenticationFailedError);
                return false;
            }
        }
    }
    catch (ResponseTimeoutException)
    {
        // Responce Timeout exceeded
        emit smtpError(AuthenticationFailedError);
        return false;
    }
    catch (SendMessageTimeoutException)
    {
	// Send Timeout exceeded
        emit smtpError(AuthenticationFailedError);
        return false;
    }

    return true;
}

bool SmtpClient::sendMail(MimeMessage& email)
{
    try
    {
        // Send the MAIL command with the sender
        sendMessage("MAIL FROM:<" + email.getSender().getAddress() + ">");

        waitForResponse();

        if (responseCode != 250) return false;

        // Send RCPT command for each recipient
        // To (primary recipients)
        for (const EmailAddress& e : email.getRecipients(MimeMessage::To)) {
            sendMessage("RCPT TO:<" + e.getAddress() + ">");
            waitForResponse();
            if (responseCode != 250) return false;
        }

        // Cc (carbon copy)
        for (const EmailAddress& e : email.getRecipients(MimeMessage::Cc)) {
            sendMessage("RCPT TO:<" + e.getAddress() + ">");
            waitForResponse();
            if (responseCode != 250) return false;
        }

        // Bcc (blind carbon copy)
        for (const EmailAddress& e : email.getRecipients(MimeMessage::Bcc)) {
            sendMessage("RCPT TO:<" + e.getAddress() + ">");
            waitForResponse();
            if (responseCode != 250) return false;
        }

        // Send DATA command
        sendMessage("DATA");
        waitForResponse();

        if (responseCode != 354) return false;

        if (email.write(socket, sendMessageTimeout)) {
            sendMessage("");

            // Send \r\n.\r\n to end the mail data
            sendMessage(".");

            waitForResponse();

            if (responseCode != 250) return false;
        } else {
            emit smtpError(SendDataTimeoutError);
//          throw SendMessageTimeoutException();
        }
    }
    catch (ResponseTimeoutException)
    {
        return false;
    }
    catch (SendMessageTimeoutException)
    {
        return false;
    }

    return true;
}

void SmtpClient::quit()
{
    try 
    {
        sendMessage("QUIT");
    }
    catch(SmtpClient::SendMessageTimeoutException) 
    {
	//Manually close the connection to the smtp server if message "QUIT" wasn't received by the smtp server
        if(socket->state() == QAbstractSocket::ConnectedState || socket->state() == QAbstractSocket::ConnectingState || socket->state() == QAbstractSocket::HostLookupState)
            socket->disconnectFromHost();
    }
}

/* [3] --- */


/* [4] Protected methods */

void SmtpClient::waitForResponse()
{
    responseCode = -1;
    responseText.clear();

    do {
        if (!socket->canReadLine()) {
            QEventLoop l_evLoop;
            QTimer l_timer;

            connect(socket, &QTcpSocket::readyRead, &l_evLoop, &QEventLoop::quit);
            connect(&l_timer, &QTimer::timeout, &l_evLoop, [&l_evLoop, this]() {
                emit smtpError(ResponseTimeoutError);
//                throw ResponseTimeoutException();
                l_evLoop.quit();
            });

            l_timer.start(responseTimeout);

            l_evLoop.exec();
        }

        while (socket->canReadLine()) {
            // Save the server's response
            responseText = socket->readLine();
            if (responseText.endsWith("\r\n")) responseText.chop(2);
            qCDebug(smtpClient) << "RECV: " << responseText;

            // Extract the respose code from the server's responce (first 3 digits)
            responseCode = responseText.left(3).toInt();

            if (responseCode / 100 == 4)
                emit smtpError(ServerError);

            if (responseCode / 100 == 5)
                emit smtpError(ClientError);

            if (responseText[3] == ' ') { return; }
        }
    } while (true);
}

void SmtpClient::sendMessage(const QString &text)
{
    sendMessage(text.toUtf8());
}

void SmtpClient::sendMessage(const char* text)
{
    sendMessage(QByteArray(text));
}

void SmtpClient::sendMessage(const QByteArray& data)
{
    if (!socket->isOpen()) {
        return;
    }

    if (data.size() > 1024)
        qCDebug(smtpClient) << "SEND: " << (data.left(1024 - 3) + "...");
    else
        qCDebug(smtpClient) << "SEND: " << data;

    socket->write(data + "\r\n");
    if (! socket->waitForBytesWritten(sendMessageTimeout))
    {
      emit smtpError(SendDataTimeoutError);
//      throw SendMessageTimeoutException();
    }
}

/* [4] --- */


/* [5] Slots for the socket's signals */

void SmtpClient::socketStateChanged(QAbstractSocket::SocketState /*state*/)
{
}

void SmtpClient::socketError(QAbstractSocket::SocketError /*socketError*/)
{
}

void SmtpClient::socketReadyRead()
{
}

/* [5] --- */




