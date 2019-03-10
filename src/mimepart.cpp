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

#include "mimepart.h"
#include "quotedprintable.h"

#include <QCoreApplication>
#include <QIODevice>

MimePart::MimePart()
{
    m_cEncoding = _7Bit;
    m_cBoundary = "";
}

MimePart::~MimePart()
{
    return;
}

void MimePart::setHeader(const QMap<QString, QString>& header)
{
    m_header = header;
}

void MimePart::setHeader(const QString& header, const QString& value)
{
    m_header[header] = value;
}

const QMap<QString, QString>& MimePart::header() const
{
    return m_header;
}

void MimePart::setContentId(const QString & cId)
{
    m_cId = cId;
}

const QString & MimePart::contentId() const
{
    return m_cId;
}

void MimePart::setContentName(const QString & cName)
{
    m_cName = cName;
}

const QString & MimePart::contentName() const
{
    return m_cName;
}

void MimePart::setContentType(const QString & cType)
{
    m_cType = cType;
}

const QString & MimePart::contentType() const
{
    return m_cType;
}

void MimePart::setContentBoundary(const QByteArray& a_cBoundary)
{
    m_cBoundary = a_cBoundary;
}

const QByteArray&MimePart::contentBoundary() const
{
    return m_cBoundary;
}

void MimePart::setCharset(const QString & charset)
{
    m_cCharset = charset;
}

const QString & MimePart::charset() const
{
    return m_cCharset;
}

void MimePart::setEncoding(Encoding enc)
{
    m_cEncoding = enc;
}

MimePart::Encoding MimePart::encoding() const
{
    return m_cEncoding;
}

QByteArray MimePart::encodeString(const QString& a_string, MimePart::Encoding a_encode)
{
    QByteArray l_ret;
    switch (a_encode)
    {
    case MimePart::Base64:
        l_ret += "=?utf-8?B?" + QByteArray().append(a_string).toBase64() + "?=";
        break;
    case MimePart::QuotedPrintable:
        l_ret += "=?utf-8?Q?" + QuotedPrintable::encode(QByteArray().append(a_string)).replace(' ', "_").replace(':',"=3A") + "?=";
        break;
    default:
        l_ret += a_string;
    }
    return l_ret;
}

MimeContentFormatter& MimePart::contentFormatter()
{
    return m_formatter;
}

QByteArray MimePart::readContent() const {
    return readContent(0, contentSize());
}

void MimePart::writeHeader(QIODevice* device) const
{
    /* === Header Prepare === */
    QMap<QString, QString> l_header = m_header;

    /* Content-Type */
    l_header["Content-Type"] = m_cType;

    if (m_cName != "") {
        l_header["Content-Type"].append("; name=\"").append(m_cName).append("\"");
        if (l_header.contains("Content-Disposition")) {
            l_header["Content-Disposition"].append("; filename=").append(m_cName).append("");
        }
    }

    if (m_cCharset != "")
        l_header["Content-Type"].append("; charset=").append(m_cCharset);

    if (m_cBoundary != "")
        l_header["Content-Type"].append("; boundary=").append(m_cBoundary);
    /* ------------ */

    /* Content-Transfer-Encoding */
    switch (m_cEncoding)
    {
    case _7Bit:
        l_header["Content-Transfer-Encoding"] = "7bit";
        break;
    case _8Bit:
        l_header["Content-Transfer-Encoding"] = "8bit";
        break;
    case Base64:
        l_header["Content-Transfer-Encoding"] = "base64";
        break;
    case QuotedPrintable:
        l_header["Content-Transfer-Encoding"] = "quoted-printable";
        break;
    case Binary:
        l_header["Content-Transfer-Encoding"] = "binary";
        break;
    case Unknow:
        break;
    }
    /* ------------------------ */
    if (m_cEncoding == Binary)
        l_header["Content-Size"] = QString::number(contentSize());

    /* Content-Id */
    if (m_cId != nullptr)
        l_header["Content-ID"] = m_cId;
    /* ---------- */

    QMap<QString, QString>::ConstIterator l_i = l_header.constBegin(),
        l_end = l_header.constEnd();

    for (; l_i != l_end; ++l_i) {
        device->write(QString("%1: %2\r\n").arg(l_i.key(), l_i.value()).toUtf8());
    }
    /* === End of Header Prepare === */
    device->write("\r\n");
}

bool MimePart::write(QIODevice* device, qint32 a_timeout) const
{
    writeHeader(device);
    if (!device->waitForBytesWritten(a_timeout)) return false;

    /* === Content === */
    switch (m_cEncoding)
    {
    case _7Bit:
        device->write(QString(readContent()).toLatin1());
        break;
    case _8Bit:
        device->write(readContent());
        break;
    case Base64: {
        qint64 l_offset = 0;
        qint64 l_total = contentSize();
        qint32 l_chuckSize = 3 * 64 * 1024;
        QByteArray l_data;
        QByteArray l_base64;

        while (l_offset < l_total) {
            l_data = readContent(l_offset, l_chuckSize);
            l_base64 += l_data.toBase64();

            while (l_base64.size() >= 76) {
                device->write(l_base64.left(76) + "\r\n");
                l_base64 = l_base64.mid(76);
            }
            if (!device->waitForBytesWritten(a_timeout)) return false;

            l_offset += l_data.size();
            qApp->processEvents();
        }

        if (!l_base64.isEmpty())
            device->write(l_base64 + "\r\n");
    } break;
    case QuotedPrintable:
        device->write(m_formatter.format(QuotedPrintable::encode(readContent()), true).toUtf8());
        break;
    case Binary: {
        qint64 l_offset = 0;
        qint64 l_total = contentSize();
        while (l_offset < l_total) {
            QByteArray l_data = readContent(l_offset, 64 * 1024);
            device->write(l_data);
            l_offset += l_data.size();
            if (!device->waitForBytesWritten(a_timeout)) return false;
        }
    } break;
    case Unknow:
        break;
    }
    device->write("\r\n");

    return device->waitForBytesWritten(a_timeout);
    /* === End of Content === */
}

/* [4] --- */
