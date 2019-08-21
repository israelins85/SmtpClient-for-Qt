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

#include "emailaddress.h"

/* [1] Constructors and Destructors */

EmailAddress::EmailAddress()
{}

EmailAddress::EmailAddress(const char* a_address)
{
    address = a_address;
}

EmailAddress::EmailAddress(const QString & a_address)
{
    address = a_address;
}

EmailAddress::EmailAddress(const QString& a_address, const QString& a_name)
{
    address = a_address;
    name = a_name;
}

EmailAddress::~EmailAddress()
{}

bool EmailAddress::isEmpty() const
{
    return address.isEmpty();
}

QByteArray EmailAddress::encoded(MimePart::Encoding a_encode) const
{
    QByteArray l_encoded;

    if (!name.isEmpty()) {
        l_encoded += MimePart::encodeString(name, a_encode);
    }
    l_encoded += " <" + address + ">";

    return l_encoded;
}
