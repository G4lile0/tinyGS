/*
  Logger.cpp - Log class
  
  Copyright (C) 2020 -2021 @G4lile0, @gmag12 and @dev_4m1g0

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Logger.h"

char Log::logIdx = 1;
Log::LoggingLevels Log::logLevel = LOG_LEVEL;
char Log::log[MAX_LOG_SIZE] = "";

void Log::console(const char* formatP, ...)
{
  va_list arg;
  char buffer[256];
  va_start(arg, formatP);
  vsnprintf_P(buffer, sizeof(buffer), formatP, arg);
  va_end(arg);
  AddLog(LOG_LEVEL_NONE, buffer);
}

void Log::error(const char* formatP, ...)
{
  va_list arg;
  char buffer[256];
  va_start(arg, formatP);
  vsnprintf_P(buffer, sizeof(buffer), formatP, arg);
  va_end(arg);
  AddLog(LOG_LEVEL_ERROR, buffer);
}

void Log::info(const char* formatP, ...)
{
  va_list arg;
  char buffer[256];
  va_start(arg, formatP);
  vsnprintf_P(buffer, sizeof(buffer), formatP, arg);
  va_end(arg);
  AddLog(LOG_LEVEL_INFO, buffer);
}

void Log::debug(const char* formatP, ...)
{
  va_list arg;
  char buffer[256];
  va_start(arg, formatP);
  vsnprintf_P(buffer, sizeof(buffer), formatP, arg);
  va_end(arg);
  AddLog(LOG_LEVEL_DEBUG, buffer);
}

// Based on arendst/Tasmota addLog (support.ino)
void Log::AddLog(Log::LoggingLevels level, const char* logData)
{
  if (level > Log::logLevel)
    return;

  char timeStr[10];  // "13:45:21 "
  time_t currentTime = time (NULL);
  if (currentTime > 0) {
      struct tm *timeinfo = localtime (&currentTime);
      snprintf_P (timeStr, sizeof (timeStr), "%02d:%02d:%02d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  }
  else {
      timeStr[0] = '\0';
  }
  
  Serial.printf (PSTR ("%s%s\n"), timeStr, logData);

  // Delimited, zero-terminated buffer of log lines.
  // Each entry has this format: [index][log data]['\1']
  while (logIdx == log[0] ||  // If log already holds the next index, remove it
          strlen(log) + strlen(logData) + 13 > MAX_LOG_SIZE)  // 13 = idx + time + '\1' + '\0'
  {
    char* it = log;
    it++;                                // Skip web_log_index
    it += strchrspn(it, '\1');           // Skip log line
    it++;                                // Skip delimiting "\1"
    memmove(log, it, MAX_LOG_SIZE -(it-log));  // Move buffer forward to remove oldest log line
  }
  
  snprintf_P(log, sizeof(log), PSTR("%s%c%s%s\1"), log, logIdx++, timeStr, logData);

  logIdx &= 0xFF;
  if (!logIdx) 
    logIdx++;       // Index 0 is not allowed as it is the end of char string*/
}

void Log::getLog(uint32_t idx, char** entry_pp, size_t* len_p)
{
  char* entry_p = nullptr;
  size_t len = 0;

  if (idx) {
    char* it = log;
    do {
      uint32_t cur_idx = *it;
      it++;
      size_t tmp = strchrspn(it, '\1');
      tmp++;                             // Skip terminating '\1'
      if (cur_idx == idx) {              // Found the requested entry
        len = tmp;
        entry_p = it;
        break;
      }
      it += tmp;
    } while (it < log + MAX_LOG_SIZE && *it != '\0');
  }
  *entry_pp = entry_p;
  *len_p = len;
}

// Get span until single character in string
size_t Log::strchrspn(const char *str1, int character)
{
  size_t ret = 0;
  char *start = (char*)str1;
  char *end = strchr(str1, character);
  if (end) ret = end - start;
  return ret;
}

char Log::getLogIdx()
{
  return logIdx;
}

void Log::setLogLevel(LoggingLevels level)
{
  logLevel = level;
}