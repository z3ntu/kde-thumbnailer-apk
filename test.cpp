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
