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

#include "mimemultipart.h"
#include <QTime>
#include <QCryptographicHash>
#include <QIODevice>

const QString MULTI_PART_NAMES[] = {
    "multipart/mixed",         //    Mixed
    "multipart/digest",        //    Digest
    "multipart/alternative",   //    Alternative
    "multipart/related",       //    Related
    "multipart/report",        //    Report
    "multipart/signed",        //    Signed
    "multipart/encrypted"      //    Encrypted
};

MimeMultiPart::MimeMultiPart(MultiPartType type)
{
    m_multiPartType = type;
    setContentType(MULTI_PART_NAMES[m_multiPartType]);
    setEncoding(_8Bit);

    QCryptographicHash md5(QCryptographicHash::Sha1);
    md5.addData(QByteArray().append(qrand()));
    setContentBoundary(md5.result().toHex());
}

MimeMultiPart::~MimeMultiPart() {}

void MimeMultiPart::addPart(MimePart *part) {
    m_parts.append(part);
}

const QList<MimePart*> & MimeMultiPart::getParts() const {
    return m_parts;
}

void MimeMultiPart::write(QIODevice* device) const
{
    QByteArray l_contentBoundary = "--" + contentBoundary() + "\r\n";

    for (MimePart* l_mp : m_parts) {
        device->write(l_contentBoundary);
        l_mp->write(device);
    }
    device->write(l_contentBoundary);
}

void MimeMultiPart::setMultPartType(const MultiPartType type) {
    m_multiPartType = type;
    setContentType(MULTI_PART_NAMES[type]);
}

MimeMultiPart::MultiPartType MimeMultiPart::multPartType() const {
    return m_multiPartType;
}
