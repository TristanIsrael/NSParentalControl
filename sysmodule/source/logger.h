/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <string>
#include <cstdarg>

namespace alefbet {
    namespace pctrl { 
        namespace logger {
            typedef enum {
                DEBUG = 0,                
                INFO = 5,
                ERROR = 10
            } LogLevel;

            bool openFile();
            void clearLog();            
            void debugHipcMetaHeader(void* hdr);
            //void debugHipcDataHeader(void* hdr);
            void closeFile();            
            void logDebug(const char *fmt, ...);
            void logInfo(const char *fmt, ...);
            void logError(const char *fmt, ...);
            
            void setLogLevel(LogLevel level);
            LogLevel currentLogLevel();            

            namespace priv {
                void logToFile(const char *fmt, va_list);
            }
        }
    }
}

