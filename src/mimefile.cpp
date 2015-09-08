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

/* [1] Constructors and Destructors */

MimeFile::MimeFile(QFile *file)
{
    this->file = file;
    this->cType = "application/octet-stream";
    this->cName = QFileInfo(*file).fileName();
    this->cEncoding = Base64;
}
MimeFile::MimeFile(const QByteArray& stream, const QString& fileName)
{
    this->cEncoding = Base64;
    this->cType = "application/octet-stream";
    this->file = 0;
    this->cName = fileName;
    this->content = stream;
}

MimeFile::MimeFile(QIODevice *d, const QString &name)
{
    this->file = d;
    this->cType = "application/octet-stream";
    this->cName = name;
    this->cEncoding = Base64;
}

MimeFile::~MimeFile()
{
    if(file)
        delete file;
}

/* [1] --- */


/* [2] Getters and setters */

/* [2] --- */


/* [3] Protected methods */


void MimeFile::writeContent(QIODevice &device) {
    file->open(QIODevice::ReadOnly);
    this->content = file->readAll();
    file->close();

    MimePart::writeContent(device);

    this->content.clear();
}

/* [3] --- */

