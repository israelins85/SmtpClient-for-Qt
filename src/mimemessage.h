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

#ifndef MIMEMESSAGE_H
#define MIMEMESSAGE_H

#include "mimepart.h"
#include "mimemultipart.h"
#include "emailaddress.h"
#include <QList>

#include "smtpexports.h"

class QIODevice;
class SMTP_EXPORT MimeMessage : public QObject
{
public:

    enum class RecipientType {
        To,                 // primary
        Cc,                 // carbon copy
        Bcc                 // blind carbon copy
    };
    Q_ENUM(RecipientType)

    MimeMessage();
    ~MimeMessage();

    void setSender(const EmailAddress& e);
    void setReplyTo(const EmailAddress& rto);
    void addRecipient(const EmailAddress& rcpt, RecipientType type = RecipientType::To);
    void addTo(const EmailAddress& rcpt);
    void addCc(const EmailAddress& rcpt);
    void addBcc(const EmailAddress& rcpt);
    void setSubject(const QString & subject);
    void addPart(MimePart* part);

    void setInReplyTo(const QString& inReplyTo);

    void setHeaderEncoding(MimePart::Encoding);

    const EmailAddress & sender() const;
    const QList<EmailAddress> & recipients(RecipientType type = RecipientType::To) const;
    const QString & subject() const;
    const QList<MimePart*> & parts() const;
    const EmailAddress& replyTo() const;

    MimeMultiPart& content();

    bool write(QIODevice* device, qint32 timeout);

protected:
    EmailAddress m_sender;
    EmailAddress m_replyTo;
    QString m_mInReplyTo;
    QList<EmailAddress> m_recipientsTo, m_recipientsCc, m_recipientsBcc;
    QString m_subject;
    MimeMultiPart m_content;
    MimePart::Encoding m_hEncoding;
};

#endif // MIMEMESSAGE_H
