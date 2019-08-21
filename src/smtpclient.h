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

#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QtNetwork/QSslSocket>
#include <QLoggingCategory>

#include "mimemessage.h"
#include "smtpexports.h"

Q_DECLARE_LOGGING_CATEGORY(smtpClient)

class SmtpClientPrivate;
class SMTP_EXPORT SmtpClient : public QObject
{
    Q_OBJECT
    QScopedPointer<SmtpClientPrivate> const d_ptr;
//     Q_DECLARE_PRIVATE(SmtpClient)
    inline SmtpClientPrivate* d_func() {
        return reinterpret_cast<SmtpClientPrivate *>(qGetPtrHelper(d_ptr));
    }
    inline const SmtpClientPrivate* d_func() const {
        return reinterpret_cast<const SmtpClientPrivate *>(qGetPtrHelper(d_ptr));
    }
    friend class SmtpClientPrivate;

public:
    enum class AuthMethod {
        Auto,
        Plain,
        Login
    };
    Q_ENUM(AuthMethod)

    enum class SmtpErrorType {
        NoError,
        ConnectionTimeout,
        ResponseTimeout,
        SendDataTimeout,
        AuthenticationFailed,
        Server,    // 4xx smtp error
        Client     // 5xx smtp error
    };
    Q_ENUM(SmtpErrorType)

    class SmtpError {
    public:
        SmtpError(SmtpErrorType type, qint32 code = -1, QString message = QString()) :
            type(type), code(code), message(message) {}

        SmtpErrorType type = SmtpErrorType::NoError;
        qint32 code = -1;
        QString message;
    };

    enum class ConnectionType {
        Tcp,
        Ssl,
        Tls,
    };
    Q_ENUM(ConnectionType)

    SmtpClient(const QString & host = "localhost", int port = 25, ConnectionType ct = ConnectionType::Ssl);

    ~SmtpClient();

    const QString& host() const;
    void setHost(const QString &host);

    int port() const;
    void setPort(int port);

    const QString& serverName() const;
    void setServerName(const QString &name);

    ConnectionType connectionType() const;
    void setConnectionType(ConnectionType ct);

    const QString & user() const;
    void setUser(const QString &user);

    const QString & password() const;
    void setPassword(const QString &password);

    SmtpClient::AuthMethod authMethod() const;
    void setAuthMethod(AuthMethod method);

    const QString & responseText() const;
    int responseCode() const;

    int connectionTimeout() const;
    void setConnectionTimeout(int msec);

    int responseTimeout() const;
    void setResponseTimeout(int msec);
    
    int sendMessageTimeout() const;
    void setSendMessageTimeout(int msec);

    QTcpSocket* socket();

    bool connectToHost();

    bool login();
    bool login(const QString &user, const QString &password, AuthMethod method = AuthMethod::Login);

    bool sendMail(MimeMessage& email);

    void quit();

protected slots:
    void ignoreSslErrors(const QList<QSslError> & errors);

protected:
    bool waitForResponse(qint32 a_expectedCode, SmtpClient::SmtpErrorType a_onErrorType);

    bool sendMessage(const QString &text);
    bool sendMessage(const char* text);
    bool sendMessage(const QByteArray &text);

protected slots:
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketError(QAbstractSocket::SocketError error);
    void socketReadyRead();

signals:
    void smtpError(SmtpClient::SmtpError e);
};

#endif // SMTPCLIENT_H
