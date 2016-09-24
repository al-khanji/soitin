/*

Copyright (C) 2008 Louai Al-Khanji <louai.khanji@gmail.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#include <QCoreApplication>
#include "utils.h"

namespace Utils {
    QString formatSeconds(int secs) {
        QString time;
        if (secs >= 0) {
            int hours = secs / (60 * 60);
            int minutes = (secs / 60) % 60;
            int seconds = secs % 60;

            if (hours > 0) {
                time += QString("%1:").arg(hours);
            }
            time += QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
                                    .arg(seconds, 2, 10, QLatin1Char('0'));
        }
        return time;
    }

    QString directory(Directory d) {
        QString dir;
        switch (d) {
            case Translations:
                dir = TRANSLATIONS_DIR;
                break;
            case Pixmaps:
                dir = PIXMAPS_DIR;
                break;
        }
        return dir;
    }
}
