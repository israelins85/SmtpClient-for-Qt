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

#include "mimecontentformatter.h"

MimeContentFormatter::MimeContentFormatter(int max_length) :
    m_maxLength(max_length)
{}

QString MimeContentFormatter::format(const QString &content, bool quotedPrintable) const {

    QString out;

    int chars = 0;
    for (int i = 0; i < content.length() ; ++i) {
        chars++;
        if (!quotedPrintable) {
            if (chars > m_maxLength) {
                    out.append("\r\n");
                    chars = 1;
            }
        }
        else {
            if (content[i] == '\n') {       // new line
                out.append(content[i]);
                chars = 0;
                continue;
            }

            if ((chars > m_maxLength - 1)
                || ((content[i] == '=') && (chars > m_maxLength - 3) )) {
                out.append('=');
                out.append("\r\n");
                chars = 1;
            }

        }
        out.append(content[i]);
    }

    return out;

}

void MimeContentFormatter::setMaxLength(int l) {
    m_maxLength = l;
}

int MimeContentFormatter::maxLength() const {
    return m_maxLength;
}
