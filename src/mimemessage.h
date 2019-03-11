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

    enum RecipientType {
        To,                 // primary
        Cc,                 // carbon copy
        Bcc                 // blind carbon copy
    };

    MimeMessage();
    ~MimeMessage();

    void setSender(const EmailAddress& e);
    void setReplyTo(const EmailAddress& rto);
    void addRecipient(const EmailAddress& rcpt, RecipientType type = To);
    void addTo(const EmailAddress& rcpt);
    void addCc(const EmailAddress& rcpt);
    void addBcc(const EmailAddress& rcpt);
    void setSubject(const QString & subject);
    void addPart(MimePart* part);

    void setInReplyTo(const QString& inReplyTo);

    void setHeaderEncoding(MimePart::Encoding);

    const EmailAddress & getSender() const;
    const QList<EmailAddress> & getRecipients(RecipientType type = To) const;
    const QString & getSubject() const;
    const QList<MimePart*> & getParts() const;
    const EmailAddress& getReplyTo() const;

    MimeMultiPart& getContent();

    bool write(QIODevice* device, qint32 timeout);

protected:
    EmailAddress sender;
    EmailAddress replyTo;
    QString mInReplyTo;
    QList<EmailAddress> recipientsTo, recipientsCc, recipientsBcc;
    QString subject;
    MimeMultiPart content;
    MimePart::Encoding hEncoding;
};

#endif // MIMEMESSAGE_H
