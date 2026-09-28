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
#include "../headers/spfunctions.hpp"
#include <type_traits>

ThemeInfo::ThemeInfo()
{
    themeId = 0;
    name = "Default";
    comments  = "Default SoftProjector Theme";
}

Theme::Theme()
{
    m_info = ThemeInfo();
}

void Theme::saveThemeNew()
{
    QSqlQuery sq;
    sq.prepare("INSERT INTO Themes (name, comment) VALUES (?,?)");
    sq.addBindValue(m_info.name);
    sq.addBindValue(m_info.comments);
    sq.exec();

    sq.exec("SELECT seq FROM sqlite_sequence WHERE name = 'Themes'");
    sq.first();
    m_info.themeId = sq.value(0).toInt();

    for (int i = 0; i < 4; ++i) {
        saveNew(i + 1, passive[i], "ThemePassive",
                "theme_id, disp, use_background, background_name, background, use_disp_1");
        saveNew(i + 1, bible[i], "ThemeBible",
                "theme_id, disp, use_shadow, use_fading, use_blur_shadow, use_background, "
                "background_name, background, text_font, text_color, text_align_v, text_align_h, "
                "caption_font, caption_color, caption_align, caption_position, use_abbr, "
                "screen_use, screen_position, use_disp_1, "
                "add_background_color_to_text, text_rec_background_color, text_gen_background_color");
        saveNew(i + 1, song[i], "ThemeSong",
                "theme_id, disp, use_shadow, use_fading, use_blur_shadow, show_stanza_title, "
                "show_key, show_number, info_color, info_font, info_align, show_song_ending, "
                "ending_color, ending_font, ending_type, ending_position, use_background, "
                "background_name, background, text_font, text_color, text_align_v, text_align_h, "
                "screen_use, screen_position, use_disp_1, "
                "add_background_color_to_text, text_rec_background_color, text_gen_background_color");
        saveNew(i + 1, announce[i], "ThemeAnnounce",
                "theme_id, disp, use_shadow, use_fading, use_blur_shadow, use_background, "
                "background_name, background, text_font, text_color, text_align_v, text_align_h, use_disp_1");
    }
}

template<typename T>
void Theme::saveNew(int screen, T &settings, const char *table, const char *columns)
{
    QSqlQuery sq;
    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table).arg(columns)
                      .arg(QString("?,").repeated(QString(columns).count(',') + 1).chopped(1));
    sq.prepare(sql);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    if constexpr (std::is_same_v<T, TextSettings>) {
        if (QString(table) == "ThemeAnnounce")
            saveAnnouncementValues(sq, settings);
        else
            saveBindValues(sq, settings);
    } else
        saveBindValues(sq, settings);
    sq.exec();
}

template<typename T>
void Theme::saveUpdate(int screen, T &settings, const char *table, const char *columns)
{
    QSqlQuery sq;
    QStringList cols = QString(columns).split(", ");
    QStringList setParts;
    for (const QString &col : cols) {
        setParts << QString("%1 = ?").arg(col.trimmed());
    }
    QString sql = QString("UPDATE %1 SET %2 WHERE theme_id = ? AND disp = ?").arg(table).arg(setParts.join(", "));
    sq.prepare(sql);
    if constexpr (std::is_same_v<T, TextSettings>) {
        if (QString(table) == "ThemeAnnounce")
            saveAnnouncementValues(sq, settings);
        else
            saveBindValues(sq, settings);
    } else
        saveBindValues(sq, settings);
    sq.addBindValue(m_info.themeId);
    sq.addBindValue(screen);
    sq.exec();
    if (sq.numRowsAffected() == 0) {
        const QByteArray allColumns = QByteArray("theme_id, disp, ") + columns;
        saveNew(screen, settings, table, allColumns.constData());
    }
}

template<typename T>
void Theme::load(int screen, T &settings, const char *table)
{
    QSqlQuery sq;
    sq.exec(QString("SELECT * FROM %1 WHERE theme_id = %2 and disp = %3").arg(table).arg(m_info.themeId).arg(screen));
    if (!sq.first())
        return;
    QSqlRecord sr = sq.record();
    if constexpr (std::is_same_v<T, TextSettings>) {
        if (QString(table) == "ThemeAnnounce")
            loadAnnouncementFromRecord(sr, settings);
        else
            loadFromRecord(sr, settings);
    } else
        loadFromRecord(sr, settings);
}

void Theme::saveThemeUpdate()
{
    QSqlQuery sq;
    sq.prepare("UPDATE Themes SET name = ?, comment = ? WHERE id = ?");
    sq.addBindValue(m_info.name);
    sq.addBindValue(m_info.comments);
    sq.addBindValue(m_info.themeId);
    sq.exec();

    for (int i = 0; i < 4; ++i) {
        saveUpdate(i + 1, passive[i], "ThemePassive",
                "use_background, background_name, background, use_disp_1");
        saveUpdate(i + 1, bible[i], "ThemeBible",
                "use_shadow, use_fading, use_blur_shadow, use_background, background_name, background, "
                "text_font, text_color, text_align_v, text_align_h, caption_font, caption_color, "
                "caption_align, caption_position, use_abbr, screen_use, screen_position, use_disp_1, "
                "add_background_color_to_text, text_rec_background_color, text_gen_background_color");
        saveUpdate(i + 1, song[i], "ThemeSong",
                "use_shadow, use_fading, use_blur_shadow, show_stanza_title, show_key, show_number, "
                "info_color, info_font, info_align, show_song_ending, ending_color, ending_font, "
                "ending_type, ending_position, use_background, background_name, background, "
                "text_font, text_color, text_align_v, text_align_h, screen_use, screen_position, "
                "use_disp_1, add_background_color_to_text, text_rec_background_color, text_gen_background_color");
        saveUpdate(i + 1, announce[i], "ThemeAnnounce",
                "use_shadow, use_fading, use_blur_shadow, use_background, background_name, background, "
                "text_font, text_color, text_align_v, text_align_h, use_disp_1");
    }
}

void Theme::loadTheme()
{
    QSqlQuery sq;
    bool ok, allok = false;

    sq.exec(QString("SELECT name, comment FROM Themes WHERE id = %1").arg(m_info.themeId));
    ok = sq.first();
    if (ok) {
        m_info.name = sq.value(0).toString();
        m_info.comments = sq.value(1).toString();
        allok = true;
    } else {
        sq.exec("SELECT id, name, comment FROM Themes");
        ok = sq.first();
        if (ok) {
            m_info.themeId = sq.value(0).toInt();
            m_info.name = sq.value(1).toString();
            m_info.comments = sq.value(2).toString();
            allok = true;
        } else {
            saveThemeNew();
            allok = false;
        }
    }

    if (allok) {
        for (int i = 0; i < 4; ++i) {
            load(i + 1, passive[i], "ThemePassive");
            load(i + 1, bible[i], "ThemeBible");
            load(i + 1, song[i], "ThemeSong");
            load(i + 1, announce[i], "ThemeAnnounce");
        }
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

void saveBindValues(QSqlQuery &sq, TextSettings &s)
{
    sq.addBindValue(s.useBackground);
    sq.addBindValue(s.backgroundName);
    sq.addBindValue(pixToByte(s.backgroundPix));
    sq.addBindValue(s.useDisp1settings);
}

void saveAnnouncementValues(QSqlQuery &sq, TextSettings &s)
{
    sq.addBindValue(s.useShadow);
    sq.addBindValue(s.useFading);
    sq.addBindValue(s.useBlurShadow);
    sq.addBindValue(s.useBackground);
    sq.addBindValue(s.backgroundName);
    sq.addBindValue(pixToByte(s.backgroundPix));
    sq.addBindValue(s.textFont.toString());
    sq.addBindValue((unsigned int)s.textColor.rgb());
    sq.addBindValue(s.textAlignmentV);
    sq.addBindValue(s.textAlignmentH);
    sq.addBindValue(s.useDisp1settings);
}

void saveBindValues(QSqlQuery &sq, BibleSettings &s)
{
    sq.addBindValue(s.useShadow);
    sq.addBindValue(s.useFading);
    sq.addBindValue(s.useBlurShadow);
    sq.addBindValue(s.useBackground);
    sq.addBindValue(s.backgroundName);
    sq.addBindValue(pixToByte(s.backgroundPix));
    sq.addBindValue(s.textFont.toString());
    sq.addBindValue((unsigned int)(s.textColor.rgb()));
    sq.addBindValue(s.textAlignmentV);
    sq.addBindValue(s.textAlignmentH);
    sq.addBindValue(s.captionFont.toString());
    sq.addBindValue((unsigned int)(s.captionColor.rgb()));
    sq.addBindValue(s.captionAlignment);
    sq.addBindValue(s.captionPosition);
    sq.addBindValue(s.useAbbriviation);
    sq.addBindValue(s.screenUse);
    sq.addBindValue(s.screenPosition);
    sq.addBindValue(s.useDisp1settings);
    sq.addBindValue(s.bibleAddBKColorToText);
    sq.addBindValue((unsigned int)(s.bibleTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(s.bibleTextGenBKColor.rgb()));
}

void saveBindValues(QSqlQuery &sq, SongSettings &s)
{
    sq.addBindValue(s.useShadow);
    sq.addBindValue(s.useFading);
    sq.addBindValue(s.useBlurShadow);
    sq.addBindValue(s.showStanzaTitle);
    sq.addBindValue(s.showSongKey);
    sq.addBindValue(s.showSongNumber);
    sq.addBindValue((unsigned int)(s.infoColor.rgb()));
    sq.addBindValue(s.infoFont.toString());
    sq.addBindValue(s.infoAling);
    sq.addBindValue(s.showSongEnding);
    sq.addBindValue((unsigned int)(s.endingColor.rgb()));
    sq.addBindValue(s.endingFont.toString());
    sq.addBindValue(s.endingType);
    sq.addBindValue(s.endingPosition);
    sq.addBindValue(s.useBackground);
    sq.addBindValue(s.backgroundName);
    sq.addBindValue(pixToByte(s.backgroundPix));
    sq.addBindValue(s.textFont.toString());
    sq.addBindValue((unsigned int)(s.textColor.rgb()));
    sq.addBindValue(s.textAlignmentV);
    sq.addBindValue(s.textAlignmentH);
    sq.addBindValue(s.screenUse);
    sq.addBindValue(s.screenPosition);
    sq.addBindValue(s.useDisp1settings);
    sq.addBindValue(s.songAddBKColorToText);
    sq.addBindValue((unsigned int)(s.songTextRecBKColor.rgb()));
    sq.addBindValue((unsigned int)(s.songTextGenBKColor.rgb()));
}

void loadFromRecord(QSqlRecord &sr, TextSettings &s)
{
    s.useBackground = sr.field("use_background").value().toBool();
    s.backgroundName = sr.field("background_name").value().toString();
    s.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    s.useDisp1settings = sr.field("use_disp_1").value().toBool();
}

void loadAnnouncementFromRecord(QSqlRecord &sr, TextSettings &s)
{
    s.useShadow = sr.field("use_shadow").value().toBool();
    s.useFading = sr.field("use_fading").value().toBool();
    s.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    s.useBackground = sr.field("use_background").value().toBool();
    s.backgroundName = sr.field("background_name").value().toString();
    s.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    s.textFont.fromString(sr.field("text_font").value().toString());
    s.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    s.textAlignmentV = sr.field("text_align_v").value().toInt();
    s.textAlignmentH = sr.field("text_align_h").value().toInt();
    s.useDisp1settings = sr.field("use_disp_1").value().toBool();
}

void loadFromRecord(QSqlRecord &sr, BibleSettings &s)
{
    s.useShadow = sr.field("use_shadow").value().toBool();
    s.useFading = sr.field("use_fading").value().toBool();
    s.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    s.useBackground = sr.field("use_background").value().toBool();
    s.backgroundName = sr.field("background_name").value().toString();
    s.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    s.textFont.fromString(sr.field("text_font").value().toString());
    s.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    s.textAlignmentV = sr.field("text_align_v").value().toInt();
    s.textAlignmentH = sr.field("text_align_h").value().toInt();
    s.captionFont.fromString(sr.field("caption_font").value().toString());
    s.captionColor = QColor::fromRgb(sr.field("caption_color").value().toUInt());
    s.captionAlignment = sr.field("caption_align").value().toInt();
    s.captionPosition = sr.field("caption_position").value().toInt();
    s.useAbbriviation = sr.field("use_abbr").value().toBool();
    s.screenUse = sr.field("screen_use").value().toInt();
    s.screenPosition = sr.field("screen_position").value().toInt();
    s.useDisp1settings = sr.field("use_disp_1").value().toBool();
    s.bibleAddBKColorToText = sr.field("add_background_color_to_text").value().toBool();
    s.bibleTextRecBKColor = QColor::fromRgb(sr.field("text_rec_background_color").value().toUInt());
    s.bibleTextGenBKColor = QColor::fromRgb(sr.field("text_gen_background_color").value().toUInt());
}

void loadFromRecord(QSqlRecord &sr, SongSettings &s)
{
    s.useShadow = sr.field("use_shadow").value().toBool();
    s.useFading = sr.field("use_fading").value().toBool();
    s.useBlurShadow = sr.field("use_blur_shadow").value().toBool();
    s.showStanzaTitle = sr.field("show_stanza_title").value().toBool();
    s.showSongKey = sr.field("show_key").value().toBool();
    s.showSongNumber = sr.field("show_number").value().toBool();
    s.infoColor = QColor::fromRgb(sr.field("info_color").value().toUInt());
    s.infoFont.fromString(sr.field("info_font").value().toString());
    s.infoAling = sr.field("info_align").value().toInt();
    s.showSongEnding = sr.field("show_song_ending").value().toBool();
    s.endingColor = QColor::fromRgb(sr.field("ending_color").value().toUInt());
    s.endingFont.fromString(sr.field("ending_font").value().toString());
    s.endingType = sr.field("ending_type").value().toInt();
    s.endingPosition = sr.field("ending_position").value().toInt();
    s.useBackground = sr.field("use_background").value().toBool();
    s.backgroundName = sr.field("background_name").value().toString();
    s.backgroundPix.loadFromData(sr.field("background").value().toByteArray());
    s.textFont.fromString(sr.field("text_font").value().toString());
    s.textColor = QColor::fromRgb(sr.field("text_color").value().toUInt());
    s.textAlignmentV = sr.field("text_align_v").value().toInt();
    s.textAlignmentH = sr.field("text_align_h").value().toInt();
    s.screenUse = sr.field("screen_use").value().toInt();
    s.screenPosition = sr.field("screen_position").value().toInt();
    s.useDisp1settings = sr.field("use_disp_1").value().toBool();
    s.songAddBKColorToText = sr.field("add_background_color_to_text").value().toBool();
    s.songTextRecBKColor = QColor::fromRgb(sr.field("text_rec_background_color").value().toUInt());
    s.songTextGenBKColor = QColor::fromRgb(sr.field("text_gen_background_color").value().toUInt());
}
