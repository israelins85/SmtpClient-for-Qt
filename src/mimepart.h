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

#ifndef MIMEPART_H
#define MIMEPART_H

#include <QObject>
#include <QMap>
#include "mimecontentformatter.h"

#include "smtpexports.h"

class QIODevice;

class SMTP_EXPORT MimePart : public QObject
{
    Q_OBJECT
public:
    enum Encoding {
        _7Bit,
        _8Bit,
        Base64,
        QuotedPrintable,
        Binary
    };

    MimePart();
    ~MimePart();

    enum class Type { MimePart, MimeMultiPart, MimeFile, MimeText, MimeHtml };
    virtual Type type() { return Type::MimePart; }

    const QMap<QString, QString>& header() const;

    void setHeader(const QMap<QString, QString> & header);
    void setHeader(const QString& header, const QString& value);

    void setContentId(const QString & m_cId);
    const QString & contentId() const;

    void setContentName(const QString & m_cName);
    const QString & contentName() const;

    void setContentType(const QString & m_cType);
    const QString & contentType() const;

    void setContentBoundary(const QByteArray & m_cType);
    const QByteArray & contentBoundary() const;

    void setCharset(const QString & charset);
    const QString & charset() const;

    void setEncoding(Encoding enc);
    Encoding encoding() const;

    static QByteArray encodeString(const QString& a_string, Encoding a_encode);

    MimeContentFormatter& contentFormatter();

    virtual qint64 contentSize() const = 0;
    virtual QByteArray readContent() const;
    virtual QByteArray readContent(qint64 a_offset, qint64 bytes2Read) const = 0;

    virtual void writeHeader(QIODevice* device) const;
    virtual void write(QIODevice* device) const;

private:
    QMap<QString, QString> m_header;

    QString m_cId;
    QString m_cName;
    QString m_cType;
    QString m_cCharset;
    QByteArray m_cBoundary;
    Encoding m_cEncoding;

    MimeContentFormatter m_formatter;

    /* [4] --- */
};

#endif // MIMEPART_H
