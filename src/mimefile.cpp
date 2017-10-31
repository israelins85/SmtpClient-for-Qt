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

#include "mimefile.h"
#include <QFileInfo>
#include <QMimeDatabase>

MimeFile::MimeFile(const QString& fileName, Disposition a_disposition)
{
    QMimeDatabase l_mmDatabase;
    QMimeType l_mmType;
    QFileInfo l_fileInfo(fileName);

    m_file = new QFile(fileName);
    setContentType("application/octet-stream");
    setContentName(l_fileInfo.fileName());
    m_stream.clear();

    if (!m_file->isOpen())
        m_file->open(QFile::ReadOnly);

    setEncoding(Base64);

    l_mmType = l_mmDatabase.mimeTypeForFile(l_fileInfo);
    if (l_mmType.isValid())
        setContentType(l_mmType.name());

    setDisposition(a_disposition);
}

MimeFile::MimeFile(const QByteArray& stream, const QString& fileName, Disposition a_disposition)
{
    QMimeDatabase l_mmDatabase;
    QMimeType l_mmType;

    m_file = nullptr;
    setContentType("application/octet-stream");
    setContentName(fileName);
    m_stream = stream;
    setEncoding(Base64);

    l_mmType = l_mmDatabase.mimeTypeForFileNameAndData(fileName, m_stream);
    if (l_mmType.isValid())
        setContentType(l_mmType.name());

    setDisposition(a_disposition);
}

MimeFile::~MimeFile()
{
  if (m_file)
      delete m_file;
}

void MimeFile::setDisposition(MimeFile::Disposition a_disposition)
{
    QString l_disposition;
    switch (a_disposition) {
        case Disposition::Undefined:
            break;
        case Disposition::Attachment:
            l_disposition = "attachment";
            break;
        case Disposition::Inline:
            l_disposition = "inline";
            break;
    }
    if (!l_disposition.isEmpty())
        setHeader("Content-Disposition", l_disposition);
}

qint64 MimeFile::contentSize() const
{
    if (m_file)
        return m_file->size();
    return m_stream.size();
}

QByteArray MimeFile::readContent(qint64 a_offset, qint64 a_bytes2Read) const
{
    if (m_file) {
        if (m_file->pos() != a_offset)
            m_file->seek(a_offset);
        return m_file->read(a_bytes2Read);
    }

    return m_stream.mid(a_offset, a_bytes2Read);
}
