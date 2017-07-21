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

#ifndef MIMEFILE_H
#define MIMEFILE_H

#include "mimepart.h"
#include <QFile>

#include "smtpexports.h"

class SMTP_EXPORT MimeFile : public MimePart
{
    Q_OBJECT
public:
    enum class Disposition {
        Attachment, Inline
    };

    MimeFile(const QByteArray& stream, const QString& fileName, Disposition a_disposition = Disposition::Attachment);
    MimeFile(const QString& fileName, Disposition a_disposition = Disposition::Attachment);
    ~MimeFile();

    void setDisposition(Disposition a_disposition);

    virtual Type type() { return Type::MimeFile; }

    qint64 contentSize() const;
    QByteArray readContent(qint64 a_offset, qint64 a_bytes2Read) const;

private:
    QFile* m_file = nullptr;
    QByteArray m_stream;
};

#endif // MIMEFILE_H
