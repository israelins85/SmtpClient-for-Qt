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

#include "mimemessage.h"

#include <QDateTime>
#include "quotedprintable.h"
#include <QIODevice>
#include <typeinfo>

/* [1] Constructors and Destructors */
MimeMessage::MimeMessage() :
    hEncoding(MimePart::_8Bit)
{}

MimeMessage::~MimeMessage()
{}

/* [1] --- */


/* [2] Getters and Setters */
MimeMultiPart& MimeMessage::getContent() {
    return content;
}

void MimeMessage::setSender(const EmailAddress& e) {
    this->sender = e;
}

void MimeMessage::setReplyTo(const EmailAddress& rto) {
    replyTo = rto;
}

void MimeMessage::addRecipient(const EmailAddress& rcpt, RecipientType type)
{
    switch (type)
    {
    case To:
        recipientsTo << rcpt;
        break;
    case Cc:
        recipientsCc << rcpt;
        break;
    case Bcc:
        recipientsBcc << rcpt;
        break;
    }
}

void MimeMessage::addTo(const EmailAddress& rcpt) {
    this->recipientsTo << rcpt;
}

void MimeMessage::addCc(const EmailAddress& rcpt) {
    this->recipientsCc << rcpt;
}

void MimeMessage::addBcc(const EmailAddress& rcpt) {
    this->recipientsBcc << rcpt;
}

void MimeMessage::setSubject(const QString& subject)
{
    this->subject = subject;
}

void MimeMessage::addPart(MimePart *part)
{
    content.addPart(part);
}

void MimeMessage::setInReplyTo(const QString& inReplyTo)
{
    mInReplyTo = inReplyTo;
}

void MimeMessage::setHeaderEncoding(MimePart::Encoding hEnc)
{
    this->hEncoding = hEnc;
}

const EmailAddress& MimeMessage::getSender() const
{
    return sender;
}

const QList<EmailAddress>& MimeMessage::getRecipients(MimeMessage::RecipientType type) const
{
    switch (type)
    {
    default:
    case To:
        return recipientsTo;
    case Cc:
        return recipientsCc;
    case Bcc:
        return recipientsBcc;
    }
}

const EmailAddress& MimeMessage::getReplyTo() const {
    return replyTo;
}

const QString & MimeMessage::getSubject() const
{
    return subject;
}

const QList<MimePart*> & MimeMessage::getParts() const
{
    return content.getParts();
}

/* [2] --- */


/* [3] Public Methods */

#include <QDebug>
class ProxyIODevice : public QIODevice {
public:
    ProxyIODevice(QIODevice* source) : m_source(source) {
        open(QIODevice::ReadWrite);
    }

    // QIODevice interface
protected:
    virtual qint64 readData(char* data, qint64 maxlen) override
    {
        return m_source->read(data, maxlen);
    }
    virtual qint64 writeData(const char* data, qint64 len) override
    {
        qDebug() << QByteArray(data, len);
        return m_source->write(data, len);
    }
    virtual bool waitForReadyRead(int msecs) Q_DECL_OVERRIDE
    {
        return m_source->waitForReadyRead(msecs);
    }
    virtual bool waitForBytesWritten(int msecs) Q_DECL_OVERRIDE
    {
        return m_source->waitForBytesWritten(msecs);
    }

private:
    QIODevice* m_source;
};


bool MimeMessage::write(QIODevice* device, qint32 timeout)
{
    QString mime;

    /* =========== MIME HEADER ============ */

    /* ---------- Sender / From ----------- */
    mime = "From:";
    mime += sender.encoded(hEncoding);
    mime += "\r\n";
    /* ---------------------------------- */

    /* ------- Recipients / To ---------- */    
    if (!recipientsTo.isEmpty()) {
        mime += "To:";
        for (const EmailAddress& l_rcp : recipientsTo) {
            mime += l_rcp.encoded(hEncoding) + ",";
        }
        mime.chop(1);
        mime += "\r\n";
    }
    /* ---------------------------------- */

    /* ------- Recipients / Cc ---------- */
    if (!recipientsCc.isEmpty()) {
        mime += "Cc:";
        for (const EmailAddress& l_rcp : recipientsCc) {
            mime += l_rcp.encoded(hEncoding) + ",";
        }
        mime.chop(1);
        mime += "\r\n";
    }
    /* ---------------------------------- */

    /* ------- Recipients / Bcc ---------- */
    if (!recipientsCc.isEmpty()) {
        mime += "Bcc:";
        for (const EmailAddress& l_rcp : recipientsBcc) {
            mime += l_rcp.encoded(hEncoding) + ",";
        }
        mime.chop(1);
        mime += "\r\n";
    }
    /* ---------------------------------- */


    /* ------------ In-Reply-To ------------- */
    if (!mInReplyTo.isEmpty()) {
        mime += "In-Reply-To: <" + mInReplyTo + ">\r\n";
        mime += "References: <" + mInReplyTo + ">\r\n";
    }
    /* ---------------------------------- */

    /* ------------ Subject ------------- */
    mime += "Subject: ";
    mime += MimePart::encodeString(subject, hEncoding);
    mime += "\r\n";
    /* ---------------------------------- */

    /* ---------- Reply-To -------------- */
    if (!replyTo.getAddress().isEmpty()) {
        mime += "Reply-To: ";
        mime += replyTo.encoded(hEncoding);
        mime += "\r\n";
    }
    /* ---------------------------------- */

    mime += "MIME-Version: 1.0\r\n";

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) //Qt4 workaround since RFC2822Date isn't defined
    mime += QString("Date: %1\r\n").arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss +/-TZ"));
#else //Qt5 supported
    mime += QString("Date: %1\r\n").arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date));
#endif //support RFC2822Date

//    ProxyIODevice l_proxy(device);
//    l_proxy.write(mime.toUtf8());
//    content.write(&l_proxy);
//    return true;

    device->write(mime.toUtf8());
    if (!device->waitForBytesWritten(timeout)) return false;
    return content.write(device, timeout);
}

/* [3] --- */
