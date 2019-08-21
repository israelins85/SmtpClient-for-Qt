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

#include "mimetext.h"

MimeText::MimeText(const QString &txt)
{
    setContentType("text/plain");
    setCharset("utf-8");
    setEncoding(Encoding::_8Bit);
    setText(txt);
}

MimeText::~MimeText() { }

void MimeText::setText(const QString & text)
{
    m_text = text;
    m_textUtf8 = m_text.toUtf8();
}

const QString& MimeText::text() const
{
    return m_text;
}

qint64 MimeText::contentSize() const
{
    return m_textUtf8.size();
}

QByteArray MimeText::readContent(qint64 a_offset, qint64 bytes2Read) const
{
    return m_textUtf8.mid(qint32(a_offset), qint32(bytes2Read));
}
