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

#ifndef MIMEINLINEFILE_H
#define MIMEINLINEFILE_H

#include "mimefile.h"
#include "smtpexports.h"

class SMTP_EXPORT MimeInlineFile : public MimeFile
{
public:
    MimeInlineFile(const QString& fileName);
    MimeInlineFile(const QByteArray& stream, const QString& fileName);
    ~MimeInlineFile();

    virtual Type type() { return Type::MimeInlineFile; }
};

#endif // MIMEINLINEFILE_H
