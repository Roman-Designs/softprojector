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

#ifndef THEME_HPP
#define THEME_HPP

#include <QtSql>
#include <array>
#include "settings.hpp"

class ThemeInfo
{
public:
    ThemeInfo();
    int themeId;
    QString name;
    QString comments;
};

class Theme
{
public:
    Theme();
    TextSettingsBase common;
    std::array<TextSettings, 4> passive;
    std::array<BibleSettings, 4> bible;
    std::array<SongSettings, 4> song;
    std::array<TextSettings, 4> announce;

public slots:
    void saveThemeNew();
    void saveThemeUpdate();
    void loadTheme();
    void setThemeId(int id){m_info.themeId = id;}
    int getThemeId(){return m_info.themeId;}
    void setThemeInfo(ThemeInfo info);
    ThemeInfo getThemeInfo();

private:
    ThemeInfo m_info;

    template<typename T>
    void saveNew(int screen, T &settings, const char *table, const char *columns);
    template<typename T>
    void saveUpdate(int screen, T &settings, const char *table, const char *columns);
    template<typename T>
    void load(int screen, T &settings, const char *table);
};

#endif // THEME_HPP
