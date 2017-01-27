/*
 *  This file is part of kde-thumbnailer-apk
 *  Copyright (C) 2013 Ni Hui <shuizhuyuanluo@126.com>
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

#include <QDataStream>
#include <QImage>
#include <QString>
#include <KZip>
#include <KZipFileEntry>
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new ApkCreator;
    }
}

ApkCreator::ApkCreator()
{
}

ApkCreator::~ApkCreator()
{
}

static quint32 get_application_icon_resource_reference_id(const QByteArray& stream)
{
    QDataStream in(stream);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 str_manifest_index = (quint32)-1;
    quint32 str_application_index = (quint32)-1;
    quint32 str_icon_index = (quint32)-1;
    bool in_manifest_node = false;

    while (!in.atEnd()) {
        quint32 chunk_start = in.device()->pos();
        // read chunk header
        quint16 chunk_type;
        quint16 chunk_header_size;
        quint32 chunk_size;
        in >> chunk_type >> chunk_header_size >> chunk_size;

        switch (chunk_type) {
        case 0x0003/*RES_XML_TYPE*/ : {
            break;
        }
        case 0x0001/*RES_STRING_POOL_TYPE*/: {
            // string pool
            quint32 string_count;
            quint32 style_count;
            in >> string_count >> style_count;
            // skip flags
            in.skipRawData(sizeof(quint32));
            quint32 strings_start;
            in >> strings_start;
            // styles_start
            in.skipRawData(sizeof(quint32));

            // table of string indices
            quint32* string_offsets = new quint32[string_count];
            for (quint32 i = 0; i < string_count; ++i) {
                in >> string_offsets[i];
            }

            // string data
            for (quint32 i = 0; i < string_count; ++i) {
                // get string data at string_offset
                in.device()->seek(chunk_start + strings_start + string_offsets[i]);
                quint16 len;
                in >> len;
                if (len == 8 || len == 11 || len == 4) {
                    // utf-16 string
                    quint16* utf16str = new quint16[len];
                    in.readRawData((char*)utf16str, len * sizeof(quint16));
                    QString s = QString::fromUtf16(utf16str, len);
                    if (s == "manifest") {
                        str_manifest_index = i;
                    }
                    else if (s == "application") {
                        str_application_index = i;
                    }
                    else if (s == "icon") {
                        str_icon_index = i;
                    }
                    delete[] utf16str;
                    if (str_manifest_index != (quint32)-1
                        && str_application_index != (quint32)-1
                        && str_icon_index != (quint32)-1)
                        break;
                }
            }

            delete[] string_offsets;

            // skip padding and style data
            in.skipRawData(chunk_size - (in.device()->pos() - chunk_start));
            break;
        }
        case 0x0102/*RES_XML_START_ELEMENT_TYPE*/: {
            // skip line_number and comment
            in.skipRawData(2 * sizeof(quint32));
            quint32 ns;
            quint32 name;
            in >> ns >> name;

            if (name == str_manifest_index)
                in_manifest_node = true;

            if (!in_manifest_node || name != str_application_index) {
                in.skipRawData(chunk_size - (in.device()->pos() - chunk_start));
                break;
            }

            // skip attribute_start, attribute_size
            in.skipRawData(2 * sizeof(quint16));
            quint16 attribute_count;
            in >> attribute_count;
            // skip id_index, class_index, style_index
            in.skipRawData(3 * sizeof(quint16));

            // attributes
            for (quint32 i = 0; i < attribute_count; ++i) {
                quint32 ns;
                quint32 name;
                quint32 raw_value;
                in >> ns >> name >> raw_value;
                if (name != str_icon_index) {
                    // skip typed data
                    in.skipRawData(2 * sizeof(quint32));
                    continue;
                }
                // typed data
                // skip size and zero
                in.skipRawData(sizeof(quint16) + sizeof(quint8));
                quint8 data_type;
                quint32 data;
                in >> data_type >> data;
                if (raw_value == 0xffffffff && data_type == 1/*refernce type*/) {
                    return data;
                }
            }
            break;
        }
        case 0x0103/*RES_XML_END_ELEMENT_TYPE*/: {
            // skip line_number and comment
            in.skipRawData(2 * sizeof(quint32));
            quint32 ns;
            quint32 name;
            in >> ns >> name;
            if (name == str_manifest_index)
                in_manifest_node = false;
            break;
        }
        default: {
            in.skipRawData(chunk_size - 8);
            break;
        }
        }
    }

    return -1;
}

static QStringList get_application_icon_resource_path(const QByteArray& stream, quint32 reference_id)
{
    quint32 res_type = (reference_id >> 16) & 0xff;
    quint32 res_index = reference_id & 0xffff;

    QStringList iconpaths;

    QDataStream in(stream);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 string_pool_start = 0;

    while (!in.atEnd()) {
        quint32 chunk_start = in.device()->pos();
        // read chunk header
        quint16 chunk_type;
        quint16 chunk_header_size;
        quint32 chunk_size;
        in >> chunk_type >> chunk_header_size >> chunk_size;

        switch (chunk_type) {
        case 0x0002/*RES_TABLE_TYPE*/: {
            quint32 package_count;
            in >> package_count;
            break;
        }
        case 0x0001/*RES_STRING_POOL_TYPE*/: {
            // string pool
            if (string_pool_start == 0)
                string_pool_start = chunk_start;
            in.skipRawData(chunk_size - (in.device()->pos() - chunk_start));
            break;
        }
        case 0x0200/*RES_TABLE_PACKAGE_TYPE*/: {
            quint32 id;
            quint16 name[128];
            in >> id;
            in.readRawData((char*)name, 128 * sizeof(quint16));
            // skip type_strings last_public_type key_strings last_public_key
            in.skipRawData(4 * sizeof(quint32));
            break;
        }
        case 0x0201/*RES_TABLE_TYPE_TYPE*/: {
            quint8 id;
            in >> id;
            if (id != res_type) {
                in.skipRawData(chunk_size - (in.device()->pos() - chunk_start));
                break;
            }
            quint8 res0;
            quint16 res1;
            quint32 entry_count;
            quint32 entries_start;
            in >> res0;
            in >> res1;
            in >> entry_count;
            in >> entries_start;
            {
                quint32 size;
                in >> size;
                in.skipRawData(size - 4);
            }

            if (res_index < entry_count) {
                in.skipRawData(res_index * sizeof(quint32));
                quint32 entry_index;
                in >> entry_index;
                if (entry_index != 0xffffffff) {
                    in.device()->seek(chunk_start + entries_start + entry_index);
                    // skip entry_size entry_flag entry_key
                    in.skipRawData(2 * sizeof(quint32));
                    // typed data, skip size zero data_type
                    in.skipRawData(sizeof(quint32));
                    quint32 data;
                    in >> data;

                    // get string from string_pool_start
                    quint32 oldpos = in.device()->pos();
                    in.device()->seek(string_pool_start);
                    // skip chunk header string_count style_count flags
                    in.skipRawData(5 * sizeof(quint32));
                    quint32 strings_start;
                    in >> strings_start;
                    // skip styles_start
                    in.skipRawData(sizeof(quint32));

                    // table of string indices
                    in.skipRawData(data * sizeof(quint32));
                    quint32 string_offset;
                    in >> string_offset;

                    // get string data at string_offset
                    in.device()->seek(string_pool_start + strings_start + string_offset);
                    quint16 len;
                    in >> len;
                    // utf-16 string
                    quint16* utf16str = new quint16[len];
                    in.readRawData((char*)utf16str, len * sizeof(quint16));
                    iconpaths << QString::fromUtf16(utf16str, len);
                    delete[] utf16str;

                    in.device()->seek(oldpos);
                }
            }
            in.skipRawData(chunk_size - (in.device()->pos() - chunk_start));
            break;
        }
        default: {
            in.skipRawData(chunk_size - 8);
            break;
        }
        }
    }

    return iconpaths;
}

bool ApkCreator::create( const QString& path, int width, int height, QImage& img )
{
    KZip zip( path );
    if ( !zip.open( QIODevice::ReadOnly ) )
        return false;

    // get iconid from AndroidManifest.xml
    const KArchiveEntry* manifestEntry = zip.directory()->entry( "AndroidManifest.xml" );
    const KZipFileEntry* manifestFile = static_cast<const KZipFileEntry*>(manifestEntry);
    if ( !manifestFile )
        return false;

    quint32 iconid = get_application_icon_resource_reference_id( manifestFile->data() );
    if ( iconid == (quint32)-1 )
        return false;

    // get iconpaths from resources.arsc
    const KArchiveEntry* resourcesEntry = zip.directory()->entry( "resources.arsc" );
    const KZipFileEntry* resourcesFile = static_cast<const KZipFileEntry*>(resourcesEntry);
    if ( !resourcesFile )
        return false;

    QStringList iconpaths = get_application_icon_resource_path( resourcesFile->data(), iconid );
    if ( iconpaths.isEmpty() )
        return false;

    // read image from package at iconpaths
    foreach (const QString& iconpath, iconpaths) {
        const KArchiveEntry* iconEntry = zip.directory()->entry( iconpath );
        const KZipFileEntry* iconFile = static_cast<const KZipFileEntry*>(iconEntry);
        if ( !iconFile )
            return false;

        QImage icon;
        icon.loadFromData( iconFile->data() );
        if ( icon.width() * icon.height() > img.width() * img.height() )
            img = icon;
    }

    if ( img.isNull() )
        return false;

    return true;
}
