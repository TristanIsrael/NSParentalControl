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

#include "logger.h"
#include <mutex>
#include <cstring>
#include <cassert>
#include <switch.h>

constexpr const char* LogFilename = "/config/parental_control/overlay.log";

namespace alefbet {
    namespace pctrl {

        namespace {
            static std::mutex s_mutex;                
            static constinit char g_format_buffer[2 * 4 * 1024];
            static FsFileSystem sdmc;
            static bool ready = false;
            static FsFile handle;
            static size_t offset = 0;
            static bool opened = false;                
        }

        typedef struct HipcMetadata {
            u32 type;
            u32 num_send_statics;
            u32 num_send_buffers;
            u32 num_recv_buffers;
            u32 num_exch_buffers;
            u32 num_data_words;
            u32 num_recv_statics; // also accepts HIPC_AUTO_RECV_STATIC
            u32 send_pid;
            u32 num_copy_handles;
            u32 num_move_handles;
        } HipcMetadata;

        namespace logger {           
            namespace {
                static LogLevel loglevel = INFO;
            }

            /*! @brief Returns the current log level.
             * @return The current LogLevel. */
            LogLevel currentLogLevel() {
                return loglevel;
            }
            
            /*! @brief Initializes the SD card filesystem for logging.
             * @return true if successful, false otherwise. */
            bool prepare() {
                if(ready) return true;

                ready = R_SUCCEEDED(fsOpenSdCardFileSystem(&sdmc));

                return ready;
            }

            /*! @brief Opens the log file for writing, creating it if it doesn't exist.
             * @return true if the file is open, false otherwise. */
            bool openFile() {
                if(opened) return true;
                if(!prepare()) return false;

                // Try to open the file
                opened = R_SUCCEEDED(fsFsOpenFile(&sdmc, LogFilename, FsOpenMode_Write | FsOpenMode_Append, &handle));
                //opened = R_SUCCEEDED(OpenFile(std::addressof(handle), LogFilename, OpenMode_Write | OpenMode_AllowAppend));                

                // If the file could not be opened 
                if(!opened) {
                    // try to create a new one
                    //if(R_SUCCEEDED(CreateFile(LogFilename, 0))) {
                    if(R_SUCCEEDED(fsFsCreateFile(&sdmc, LogFilename, 0, 0))) {
                        // Then open it
                        //opened = R_SUCCEEDED(OpenFile(std::addressof(handle), LogFilename, OpenMode_Write | OpenMode_AllowAppend));
                        opened = R_SUCCEEDED(fsFsOpenFile(&sdmc, LogFilename, FsOpenMode_Write | FsOpenMode_Append, &handle));
                    }
                }                 
                
                return opened;
            }
            
            /*! @brief Deletes the log file. */
            void clearLog() {
                if(opened) return;

                if(prepare()) {
                    fsFsDeleteFile(&sdmc, LogFilename);
                }
                //DeleteFile(LogFilename);
            }          

            /*! @brief Logs a debug message if the current log level permits.
             * @param fmt The format string (printf-style).
             * @param ... Variable arguments for the format string. */
            void logDebug(const char *fmt, ...) {
                if(loglevel > DEBUG) return;
                                
                va_list args;
                va_start(args, fmt);
                priv::logToFile(fmt, args);
                va_end(args);
            }

            /*! @brief Logs an info message if the current log level permits.
             * @param fmt The format string (printf-style).
             * @param ... Variable arguments for the format string. */
            void logInfo(const char *fmt, ...) {
                if(loglevel > INFO) return;

                va_list args;
                va_start(args, fmt);
                priv::logToFile(fmt, args);
                va_end(args);
            }

            /*! @brief Logs an error message if the current log level permits.
             * @param fmt The format string (printf-style).
             * @param ... Variable arguments for the format string. */
            void logError(const char *fmt, ...) {
                if(loglevel > ERROR) return;

                va_list args;
                va_start(args, fmt);
                priv::logToFile(fmt, args);
                va_end(args);
            }

            namespace priv {
                /*! @brief Internal function to write a formatted log message to the log file.
                 * @param fmt The format string (printf-style).
                 * @param args The va_list of arguments. */
                void logToFile(const char* fmt, va_list args) {
                    std::lock_guard<std::mutex> lock(s_mutex);

                    if(openFile()) {
                        //std::va_list vl;
                        //va_start(vl, fmt);
                        vsnprintf(g_format_buffer, sizeof(g_format_buffer), fmt, args);
                        //va_end(vl);    
                        
                        size_t size = std::strlen(g_format_buffer);
                        //if(R_SUCCEEDED(WriteFile(handle, offset, g_format_buffer, size, WriteOption::Flush))) {
                        if(R_SUCCEEDED(fsFileWrite(&handle, offset, g_format_buffer, size, FsWriteOption_Flush))) {
                            offset += size;                        
                        }

                        closeFile();
                    }                                        
                }
            }

            /*! @brief Logs the contents of a HIPC metadata header for debugging.
             * @param hdr Pointer to the HIPC metadata header. */
            void debugHipcMetaHeader(void* hdr) {
                HipcMetadata* h = (HipcMetadata*)hdr;

                logDebug("HipcHeader contents:\n");
                logDebug("  type               : %u\n", h->type);
                logDebug("  num_send_statics   : %u\n", h->num_send_statics);
                logDebug("  num_send_buffers   : %u\n", h->num_send_buffers);
                logDebug("  num_recv_buffers   : %u\n", h->num_recv_buffers);
                logDebug("  num_exch_buffers   : %u\n", h->num_exch_buffers);
                logDebug("  num_data_words     : %u\n", h->num_data_words);
                logDebug("  num_recv_statics   : %u\n", h->num_recv_statics);
                logDebug("  send_pid           : %u\n", h->send_pid);
                logDebug("  num_copy_handles   : %u\n", h->num_copy_handles);
                logDebug("  num_move_handles   : %u\n", h->num_move_handles);
            }

            /*! @brief Closes the log file handle. */
            void closeFile() {
                if(opened) {
                    //CloseFile(handle);
                    fsFileClose(&handle);
                    opened = false;
                }
            }

            /*! @brief Sets the current log level and logs the change.
             * @param level The new log level to set. */
            void setLogLevel(LogLevel level)
            {
                loglevel = level;

                std::string strloglevel = "Unknown";
                switch(loglevel) {
                    case DEBUG: strloglevel = "DEBUG"; break;
                    case INFO: strloglevel = "INFO"; break;
                    case ERROR: strloglevel = "ERROR"; break;
                }

                logInfo("[Logger] Log level set to %s\n", strloglevel.c_str());                
            }
        }
    }

}