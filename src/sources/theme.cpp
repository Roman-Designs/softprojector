/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
***************************************************************************/

#include "../headers/theme.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

void migrateThemeTablesForVideoBackgrounds()
{
    QSqlQuery sq;
    QStringList tables = {"ThemeBible", "ThemeSong", "ThemeAnnounce", "ThemePassive"};
    bool migration_failed = false;

    for (const QString &tableName : tables) {
        qDebug() << "Migrating" << tableName << "for video backgrounds...";

        sq.exec(QString("PRAGMA table_info(%1)").arg(tableName));
        QSet<QString> existingColumns;
        while (sq.next()) {
            existingColumns.insert(sq.value("name").toString());
        }

        if (!existingColumns.contains("background_video_path")) {
            QString alterPath = QString("ALTER TABLE %1 ADD COLUMN background_video_path TEXT").arg(tableName);
            if (!sq.exec(alterPath)) {
                qWarning() << "Failed to add background_video_path to" << tableName << ":" << sq.lastError().text();
                migration_failed = true;
            } else {
                qDebug() << "Added background_video_path to" << tableName;
            }
        }

        if (!existingColumns.contains("background_video_loop")) {
            QString alterLoop = QString("ALTER TABLE %1 ADD COLUMN background_video_loop INTEGER DEFAULT 1").arg(tableName);
            if (!sq.exec(alterLoop)) {
                qWarning() << "Failed to add background_video_loop to" << tableName << ":" << sq.lastError().text();
                migration_failed = true;
            } else {
                qDebug() << "Added background_video_loop to" << tableName;
            }
        }

        if (!existingColumns.contains("background_video_fill_mode")) {
            QString alterFillMode = QString("ALTER TABLE %1 ADD COLUMN background_video_fill_mode INTEGER DEFAULT 0").arg(tableName);
            if (!sq.exec(alterFillMode)) {
                qWarning() << "Failed to add background_video_fill_mode to" << tableName << ":" << sq.lastError().text();
                migration_failed = true;
            } else {
                qDebug() << "Added background_video_fill_mode to" << tableName;
            }
        }
    }

    if (migration_failed) {
        qWarning() << "Some migration operations failed for video background columns";
    } else {
        qDebug() << "Video background migration completed successfully";
    }
}

ThemeInfo::ThemeInfo()
{
    themeId = 0;
    name = "Default";
    comments  = "Default SoftProjector Theme";
}

Theme::Theme()
{
    m_info = ThemeInfo();
    //    themeId = 0;
    //    name = "Default";
    //    comments  = "Default SoftProjector Theme";
}

void Theme::saveThemeNew()
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO Themes (name, comment) VALUES (?,?)");
    sq.addBindValue(m_info.name);
    sq.addBindValue(m_info.comments);
    sq.exec();

    // get new theme id
    sq.exec("SELECT seq FROM sqlite_sequence WHERE name = 'Themes'");
    sq.first();
    m_info.themeId = sq.value(0).toInt();
    savePassiveNew(1,passive);
    savePassiveNew(2,passive2);
    savePassiveNew(3,passive3);
    savePassiveNew(4,passive4);
    saveBibleNew(1,bible);
    saveBibleNew(2,bible2);
    saveBibleNew(3,bible3);
    saveBibleNew(4,bible4);
    saveSongNew(1,song);
    saveSongNew(2,song2);
    saveAnnounceNew(1,announce);
    saveAnnounceNew(2,announce2);
}

void Theme::savePassiveNew(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO ThemePassive (theme_id, disp, use_background, background_name, background, use_disp_1, "
               "background_video_path, background_video_loop, background_video_fill_mode) "
               "VALUES(?,?,?,?,?,?,?,?,?)");
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.exec();
}

void Theme::saveBibleNew(int screen, BibleSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO ThemeBible (theme_id, disp, use_shadow, use_fading, use_blur_shadow, use_background, "
               "background_name, background, text_font, text_color, text_align_v, text_align_h, caption_font, "
               "caption_color, caption_align, caption_position, use_abbr, screen_use, screen_position, use_disp_1, "
               "add_background_color_to_text, text_rec_background_color, text_gen_background_color, "
               "background_video_path, background_video_loop, background_video_fill_mode) "
               "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont.toString());
    sq.addBindValue((unsigned int)(settings.textColor.rgb()));
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.captionFont.toString());
    sq.addBindValue((unsigned int)(settings.captionColor.rgb()));
    sq.addBindValue(settings.captionAlignment);
    sq.addBindValue(settings.captionPosition);
    sq.addBindValue(settings.useAbbriviation);
    sq.addBindValue(settings.screenUse);
    sq.addBindValue(settings.screenPosition);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.bibleAddBKColorToText);
    sq.addBindValue((unsigned int)(settings.bibleTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(settings.bibleTextGenBKColor.rgb()));
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.exec();
}

void Theme::saveSongNew(int screen, SongSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO ThemeSong (theme_id, disp, use_shadow, use_fading, use_blur_shadow, show_stanza_title, "
               "show_key, show_number, info_color, info_font, info_align, show_song_ending, ending_color, ending_font, "
               "ending_type, ending_position, use_background, background_name, background, text_font, text_color, "
               "text_align_v, text_align_h, screen_use, screen_position, use_disp_1, "
               "add_background_color_to_text, text_rec_background_color, text_gen_background_color, "
               "background_video_path, background_video_loop, background_video_fill_mode) "
               "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.showStanzaTitle);
    sq.addBindValue(settings.showSongKey);
    sq.addBindValue(settings.showSongNumber);
    sq.addBindValue((unsigned int)(settings.infoColor.rgb()));
    sq.addBindValue(settings.infoFont.toString());
    sq.addBindValue(settings.infoAling);
    sq.addBindValue(settings.showSongEnding);
    sq.addBindValue((unsigned int)(settings.endingColor.rgb()));
    sq.addBindValue(settings.endingFont.toString());
    sq.addBindValue(settings.endingType);
    sq.addBindValue(settings.endingPosition);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont);
    sq.addBindValue((unsigned int)(settings.textColor.rgb()));
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.screenUse);
    sq.addBindValue(settings.screenPosition);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.songAddBKColorToText);
    sq.addBindValue((unsigned int)(settings.songTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(settings.songTextGenBKColor.rgb()));
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.exec();
}

void Theme::saveAnnounceNew(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO ThemeAnnounce (theme_id, disp, use_shadow, use_fading, use_blur_shadow, use_background, "
               "background_name, background, text_font, text_color, text_align_v, text_align_h, use_disp_1, "
               "background_video_path, background_video_loop, background_video_fill_mode) "
               "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont.toString());
    sq.addBindValue(settings.textColor.rgb());
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.exec();
}

void Theme::saveThemeUpdate()
{
    QSqlQuery sq;
    sq.prepare("UPDATE Themes SET name = ?, comments = ? WHERE id = ?");
    sq.addBindValue(m_info.name);
    sq.addBindValue(m_info.comments);
    sq.addBindValue(m_info.themeId);
    sq.exec();

    savePassiveUpdate(1,passive);
    savePassiveUpdate(2,passive2);
    savePassiveUpdate(3,passive3);
    savePassiveUpdate(4,passive4);
    saveBibleUpdate(1,bible);
    saveBibleUpdate(2,bible2);
    saveBibleUpdate(3,bible3);
    saveBibleUpdate(4,bible4);
    saveSongUpdate(1,song);
    saveSongUpdate(2,song2);
    saveSongUpdate(3,song3);
    saveSongUpdate(4,song4);
    saveAnnounceUpdate(1,announce);
    saveAnnounceUpdate(2,announce2);
    saveAnnounceUpdate(3,announce3);
    saveAnnounceUpdate(4,announce4);
}

void Theme::savePassiveUpdate(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("UPDATE ThemePassive SET use_background = ?, background_name = ?, background = ?, use_disp_1 = ?, "
               "background_video_path = ?, background_video_loop = ?, background_video_fill_mode = ? "
               "WHERE theme_id = ? AND disp = ?");
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.exec();
}

void Theme::saveBibleUpdate(int screen, BibleSettings &settings)
{

    QSqlQuery sq;
    sq.prepare("UPDATE ThemeBible SET use_shadow = ?, use_fading = ?, use_blur_shadow = ?, "
               "use_background = ?, background_name = ?, background = ?, text_font = ?, text_color = ?, text_align_v = ?, "
               "text_align_h = ?, caption_font = ?, caption_color = ?, caption_align = ?, caption_position = ?, "
               "use_abbr = ?, screen_use = ?, screen_position = ?, use_disp_1 = ?, "
               "add_background_color_to_text = ?, text_rec_background_color = ?, text_gen_background_color = ?, "
               "background_video_path = ?, background_video_loop = ?, background_video_fill_mode = ? "
               "WHERE theme_id = ? AND disp = ?");
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont.toString());
    sq.addBindValue((unsigned int)(settings.textColor.rgb()));
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.captionFont.toString());
    sq.addBindValue((unsigned int)(settings.captionColor.rgb()));
    sq.addBindValue(settings.captionAlignment);
    sq.addBindValue(settings.captionPosition);
    sq.addBindValue(settings.useAbbriviation);
    sq.addBindValue(settings.screenUse);
    sq.addBindValue(settings.screenPosition);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.bibleAddBKColorToText);
    sq.addBindValue((unsigned int)(settings.bibleTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(settings.bibleTextGenBKColor.rgb()));
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.exec();
}

void Theme::saveSongUpdate(int screen, SongSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("UPDATE ThemeSong SET use_shadow = ?, use_fading = ?, use_blur_shadow = ?, "
               "show_stanza_title = ?, show_key = ?, show_number = ?, info_color = ?, info_font = ?, info_align = ?, "
               "show_song_ending = ?, ending_color = ?, ending_font = ?, ending_type = ?, ending_position = ?, "
               "use_background = ?, background_name = ?, background = ?, text_font = ?, text_color = ?, text_align_v = ?, "
               "text_align_h = ?, screen_use = ?, screen_position = ?, use_disp_1 = ?, "
               "add_background_color_to_text = ?, text_rec_background_color = ?, text_gen_background_color = ?, "
               "background_video_path = ?, background_video_loop = ?, background_video_fill_mode = ? "
               "WHERE theme_id = ? AND disp = ?");
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.showStanzaTitle);
    sq.addBindValue(settings.showSongKey);
    sq.addBindValue(settings.showSongNumber);
    sq.addBindValue((unsigned int)(settings.infoColor.rgb()));
    sq.addBindValue(settings.infoFont.toString());
    sq.addBindValue(settings.infoAling);
    sq.addBindValue(settings.showSongEnding);
    sq.addBindValue((unsigned int)(settings.endingColor.rgb()));
    sq.addBindValue(settings.endingFont.toString());
    sq.addBindValue(settings.endingType);
    sq.addBindValue(settings.endingPosition);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont);
    sq.addBindValue((unsigned int)(settings.textColor.rgb()));
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.screenUse);
    sq.addBindValue(settings.screenPosition);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.songAddBKColorToText);
    sq.addBindValue((unsigned int)(settings.songTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(settings.songTextGenBKColor.rgb()));
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.exec();
}

void Theme::saveAnnounceUpdate(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    sq.prepare("UPDATE ThemeAnnounce SET use_shadow = ?, use_fading = ?, use_blur_shadow = ?, "
               "use_background = ?, background_name = ?, background = ?, text_font = ?, text_color = ?, "
               "text_align_v = ?, text_align_h = ?, use_disp_1 = ?, "
               "background_video_path = ?, background_video_loop = ?, background_video_fill_mode = ? "
               "WHERE theme_id = ? AND disp = ?");
    sq.addBindValue(settings.useShadow);
    sq.addBindValue(settings.useFading);
    sq.addBindValue(settings.useBlurShadow);
    sq.addBindValue(settings.useBackground);
    sq.addBindValue(settings.backgroundName);
    sq.addBindValue(pixToByte(settings.backgroundPix));
    sq.addBindValue(settings.textFont.toString());
    sq.addBindValue(settings.textColor.rgb());
    sq.addBindValue(settings.textAlignmentV);
    sq.addBindValue(settings.textAlignmentH);
    sq.addBindValue(settings.useDisp1settings);
    sq.addBindValue(settings.backgroundVideoPath);
    sq.addBindValue(settings.backgroundVideoLoop);
    sq.addBindValue(settings.backgroundVideoFillMode);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.exec();
}

void Theme::loadTheme()
{
    QSqlQuery sq;
    bool ok, allok;
    allok = false;

    sq.exec(QString("SELECT name, comment FROM Themes WHERE id = %1").arg(m_info.themeId));
    ok = sq.first();
    if(ok)
    {
        m_info.name = sq.value(0).toString();
        m_info.comments = sq.value(1).toString();
        allok = true;
    }
    else
    {
        sq.exec("SELECT id, name, comment FROM Themes");
        ok = sq.first();

        // Check at least one theme exitst
        if (ok) // If exist, then load it
        {
            m_info.themeId = sq.value(0).toInt();
            m_info.name = sq.value(1).toString();
            m_info.comments = sq.value(2).toString();
            allok = true;
        }
        else // No themes exist, creat one and exit
        {
            saveThemeNew();
            allok = false;
        }
    }

    if(allok)
    {
        loadPassive(1,passive);
        loadPassive(2,passive2);
        loadPassive(3,passive3);
        loadPassive(4,passive4);
        loadBible(1,bible);
        loadBible(2,bible2);
        loadBible(3,bible3);
        loadBible(4,bible4);
        loadSong(1,song);
        loadSong(2,song2);
        loadAnnounce(1,announce);
        loadAnnounce(2,announce2);
    }
}

void Theme::loadPassive(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    QSqlRecord sr;
    sq.exec(QString("SELECT * FROM ThemePassive WHERE theme_id = %1 and disp = %2").arg(m_info.themeId).arg(screen));
    sq.first();
    sr = sq.record();
    settings.useBackground = sr.field("use_background").value().toBool();
    settings.backgroundName = sr.field("background_name").value().toString();
    settings.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    settings.useDisp1settings = sr.field("use_disp_1").value().toBool();

    QString videoPath = sr.field("background_video_path").value().toString();
    if (!videoPath.isEmpty() && QFile::exists(videoPath)) {
        settings.backgroundVideoPath = videoPath;
        settings.backgroundVideoLoop = sr.field("background_video_loop").value().toBool();
        settings.backgroundVideoFillMode = sr.field("background_video_fill_mode").value().toInt();
    } else {
        if (!videoPath.isEmpty()) {
            qWarning() << "Video background file not found:" << videoPath << "for ThemePassive screen" << screen;
        }
        settings.backgroundVideoPath.clear();
        settings.backgroundVideoLoop = true;
        settings.backgroundVideoFillMode = 0;
    }
}

void Theme::loadBible(int screen, BibleSettings &settings)
{
    QSqlQuery sq;
    QSqlRecord sr;
    sq.exec(QString("SELECT * FROM ThemeBible WHERE theme_id = %1 and disp = %2").arg(m_info.themeId).arg(screen));
    sq.first();
    sr = sq.record();
    settings.useShadow = sr.field("use_shadow").value().toBool();
    settings.useFading = sr.field("use_fading").value().toBool();
    settings.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    settings.useBackground = sr.field("use_background").value().toBool();
    settings.backgroundName = sr.field("background_name").value().toString();
    settings.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    settings.textFont.fromString(sr.field("text_font").value().toString());
    settings.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    settings.textAlignmentV = sr.field("text_align_v").value().toInt();
    settings.textAlignmentH = sr.field("text_align_h").value().toInt();
    settings.captionFont.fromString(sr.field("caption_font").value().toString());
    settings.captionColor = QColor::fromRgb(sr.field("caption_color").value().toUInt());
    settings.captionAlignment = sr.field("caption_align").value().toInt();
    settings.captionPosition = sr.field("caption_position").value().toInt();
    settings.useAbbriviation = sr.field("use_abbr").value().toBool();
    settings.screenUse = sr.field("screen_use").value().toInt();
    settings.screenPosition = sr.field("screen_position").value().toInt();
    settings.useDisp1settings = sr.field("use_disp_1").value().toBool();
    settings.bibleAddBKColorToText = sr.field("add_background_color_to_text").value().toBool();
    settings.bibleTextRecBKColor = QColor::fromRgb(sr.field("text_rec_background_color").value().toUInt());
    settings.bibleTextGenBKColor = QColor::fromRgb(sr.field("text_gen_background_color").value().toUInt());

    QString videoPath = sr.field("background_video_path").value().toString();
    if (!videoPath.isEmpty() && QFile::exists(videoPath)) {
        settings.backgroundVideoPath = videoPath;
        settings.backgroundVideoLoop = sr.field("background_video_loop").value().toBool();
        settings.backgroundVideoFillMode = sr.field("background_video_fill_mode").value().toInt();
    } else {
        if (!videoPath.isEmpty()) {
            qWarning() << "Video background file not found:" << videoPath << "for ThemeBible screen" << screen;
        }
        settings.backgroundVideoPath.clear();
        settings.backgroundVideoLoop = true;
        settings.backgroundVideoFillMode = 0;
    }
}

void Theme::loadSong(int screen, SongSettings &settings)
{
    QSqlQuery sq;
    QSqlRecord sr;
    sq.exec(QString("SELECT * FROM ThemeSong WHERE theme_id = %1 and disp = %2").arg(m_info.themeId).arg(screen));
    sq.first();
    sr = sq.record();
    settings.useShadow = sr.field("use_shadow").value().toBool();
    settings.useFading = sr.field("use_fading").value().toBool();
    settings.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    settings.showStanzaTitle = sr.field("show_stanza_title").value().toBool();
    settings.showSongKey = sr.field("show_key").value().toBool();
    settings.showSongNumber = sr.field("show_number").value().toBool();
    settings.infoColor = QColor::fromRgb(sr.field("info_color").value().toUInt());
    settings.infoFont.fromString(sr.field("info_font").value().toString());
    settings.infoAling = sr.field("info_align").value().toInt();
    settings.showSongEnding = sr.field("show_song_ending").value().toBool();
    settings.endingColor = QColor::fromRgb(sr.field("ending_color").value().toUInt());
    settings.endingFont.fromString(sr.field("ending_font").value().toString());
    settings.endingType = sr.field("ending_type").value().toInt();
    settings.endingPosition = sr.field("ending_position").value().toInt();
    settings.useBackground = sr.field("use_background").value().toBool();
    settings.backgroundName = sr.field("background_name").value().toString();
    settings.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    settings.textFont.fromString(sr.field("text_font").value().toString());
    settings.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    settings.textAlignmentV = sr.field("text_align_v").value().toInt();
    settings.textAlignmentH = sr.field("text_align_h").value().toInt();
    settings.screenUse = sr.field("screen_use").value().toInt();
    settings.screenPosition = sr.field("screen_position").value().toInt();
    settings.useDisp1settings = sr.field("use_disp_1").value().toBool();
    settings.songAddBKColorToText = sr.field("add_background_color_to_text").value().toBool();
    settings.songTextRecBKColor = QColor::fromRgb(sr.field("text_rec_background_color").value().toUInt());
    settings.songTextGenBKColor = QColor::fromRgb(sr.field("text_gen_background_color").value().toUInt());

    QString videoPath = sr.field("background_video_path").value().toString();
    if (!videoPath.isEmpty() && QFile::exists(videoPath)) {
        settings.backgroundVideoPath = videoPath;
        settings.backgroundVideoLoop = sr.field("background_video_loop").value().toBool();
        settings.backgroundVideoFillMode = sr.field("background_video_fill_mode").value().toInt();
    } else {
        if (!videoPath.isEmpty()) {
            qWarning() << "Video background file not found:" << videoPath << "for ThemeSong screen" << screen;
        }
        settings.backgroundVideoPath.clear();
        settings.backgroundVideoLoop = true;
        settings.backgroundVideoFillMode = 0;
    }
}

void Theme::loadAnnounce(int screen, TextSettings &settings)
{
    QSqlQuery sq;
    QSqlRecord sr;
    sq.exec(QString("SELECT * FROM ThemeAnnounce WHERE theme_id = %1 and disp = %2").arg(m_info.themeId).arg(screen));
    sq.first();
    sr = sq.record();
    settings.useShadow = sr.field("use_shadow").value().toBool();
    settings.useFading = sr.field("use_fading").value().toBool();
    settings.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    settings.useBackground = sr.field("use_background").value().toBool();
    settings.backgroundName = sr.field("background_name").value().toString();
    settings.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    settings.textFont.fromString(sr.field("text_font").value().toString());
    settings.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    settings.textAlignmentV = sr.field("text_align_v").value().toInt();
    settings.textAlignmentH = sr.field("text_align_h").value().toInt();
    settings.useDisp1settings = sr.field("use_disp_1").value().toBool();

    QString videoPath = sr.field("background_video_path").value().toString();
    if (!videoPath.isEmpty() && QFile::exists(videoPath)) {
        settings.backgroundVideoPath = videoPath;
        settings.backgroundVideoLoop = sr.field("background_video_loop").value().toBool();
        settings.backgroundVideoFillMode = sr.field("background_video_fill_mode").value().toInt();
    } else {
        if (!videoPath.isEmpty()) {
            qWarning() << "Video background file not found:" << videoPath << "for ThemeAnnounce screen" << screen;
        }
        settings.backgroundVideoPath.clear();
        settings.backgroundVideoLoop = true;
        settings.backgroundVideoFillMode = 0;
    }
}

void Theme::setThemeInfo(ThemeInfo info)
{
    m_info.themeId = info.themeId;
    m_info.name = info.name;
    m_info.comments = info.comments;
}

ThemeInfo Theme::getThemeInfo()
{
    return m_info;
}
