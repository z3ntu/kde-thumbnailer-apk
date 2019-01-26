/*
 *  This file is part of kde-thumbnailer-apk
 *  Copyright (C) 2019 Luca Weiss <luca (at) z3ntu (dot) xyz>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "apkcreator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QStringList>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    args.removeFirst();

    ApkCreator apkCreator;
    bool ret;

    if (args.isEmpty()) {
        qDebug().noquote().nospace() << "Usage: " << app.arguments().at(0) << " [apks...]";
        return 1;
    }

    foreach (const QString& file, args) {
        qDebug() << "=== Parsing app icon for" << file << "===";
        QImage image;
        ret = apkCreator.create(file, 0, 0, image);
        qDebug() << "Success:" << ret;
        if (ret) {
            QFileInfo fi(file);
            QString pngFileName = QString("%1/%2.png").arg(fi.dir().path(), fi.completeBaseName());
            image.save(QString(pngFileName));
            qDebug() << "Saved image to" << pngFileName;
        }
    }

    return 0;
}
