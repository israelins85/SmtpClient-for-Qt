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

#ifndef MIMETEXT_H
#define MIMETEXT_H

#include "mimepart.h"

#include "smtpexports.h"

class SMTP_EXPORT MimeText : public MimePart
{
public:
    MimeText(const QString &text = "");
    ~MimeText();

    virtual Type type() { return Type::MimeText; }

    void setText(const QString & text);
    const QString & text() const;

    qint64 contentSize() const;
    QByteArray readContent(qint64 a_offset, qint64 bytes2Read) const;

protected:
    QString m_text;
    QByteArray m_textUtf8;
};

#endif // MIMETEXT_H
