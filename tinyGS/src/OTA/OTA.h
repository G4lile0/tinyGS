/*
  OTA.h - On The Air Update Class
  
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

#ifndef OTA_H
#define OTA_H

#define SECURE_OTA // Comment this line if you are not using SSL for OTA (Not recommended)

#include <HTTPClient.h>
#include <HTTPUpdate.h>

constexpr auto MIN_TIME_BEFORE_UPDATE = 600000;
constexpr auto TIME_BETTWEN_UPDATE_CHECK = 3600000;
constexpr auto OTA_URL = "https://tinygs/firmware/updates/update.bin?station=";

#ifdef SECURE_OTA
    static const char DSTroot_CA_update[] PROGMEM = R"EOF(
    -----BEGIN CERTIFICATE-----
    MIIFJDCCBAygAwIBAgISA4sFj+FWbCixGLUvw3IzQ77/MA0GCSqGSIb3DQEBCwUA
    MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
    EwJSMzAeFw0yMTAxMTcyMDQ4MDNaFw0yMTA0MTcyMDQ4MDNaMBoxGDAWBgNVBAMT
    D25vZGUudGlueWdzLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
    ALRIqgk7C32ALt7gyqPIxzV6uiv6qLtvh+i1HZuunz2NcGyD/M1/qC3n+2/8MZ4c
    j9s1WyoPs5+aXU+59Kb7yIKNV8tHPiSI2HhI04EbuGOPxGNmkXzzSHeDuatnKF0Q
    Zv3cAuufNiapW3tOF9vk4NtUSemwxF7PwpYv8YEf/WnIIi44hg3el73pDosaCGGi
    StgWTuTBEWgJ9IgxhWfoy3Ow+7AbMapU/MvtNaQVTUR5MclSHEXIoF4sSqk81eoX
    7CMgWiJR1Gaf+CYUEqGLXrNZ8uYmpOqR2b1eZ7zInSKtu4kmyZDZyfe7yenLIzS6
    QqoFZE+sHByxqFw4YCKZvP0CAwEAAaOCAkowggJGMA4GA1UdDwEB/wQEAwIFoDAd
    BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNV
    HQ4EFgQUXwjbt4+v50XerBQWpszvEEZnGhYwHwYDVR0jBBgwFoAUFC6zF7dYVsuu
    UAlA5h+vnYsUwsYwVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8v
    cjMuby5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9yMy5pLmxlbmNyLm9y
    Zy8wGgYDVR0RBBMwEYIPbm9kZS50aW55Z3MuY29tMEwGA1UdIARFMEMwCAYGZ4EM
    AQIBMDcGCysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0dHA6Ly9jcHMubGV0
    c2VuY3J5cHQub3JnMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDwAHcAXNxDkv7mq0VE
    sV6a1FbmEDf71fpH3KFzlLJe5vbHDsoAAAF3ElHBxAAABAMASDBGAiEAyLha1SRA
    ZQMBdS73r/gm0N6agSCvUk8QNU/QA6ikO2YCIQDN9bhnl4yQm6MApt8+Y0711/45
    uQglW9eKpp3hmnUhUwB1APZclC/RdzAiFFQYCDCUVo7jTRMZM7/fDC8gC8xO8WTj
    AAABdxJRwcAAAAQDAEYwRAIgRunWbjzKLWXlHB9wMA4znUSPVLkwLuGq84Yh0mnG
    OHcCIG/iF8+r8JZIDI34hlfz4jPJ91XuakQXprzTX5yoA67NMA0GCSqGSIb3DQEB
    CwUAA4IBAQCNyN6zIElO860JiE+AKkgNUJjLsqAx3y+o4KkrWL6Y/2CgYHZKpPhv
    4t/9AG0w0cI0vjXtnBG/LaCb7g7U7SG3K/pfs2MTNyyvKyTuasTgiLapEju2+XBg
    Hec431YtnQwk1ALKapXEieQ1lA1AZLogB78Adws8QcI/laVhU7XFHPJkHW0YiQOU
    j7Jj8Oz9XfR+MPUzpoENalZdQ8QkqwkqEkchJebPGiGWGyFwf9NIEU+RgiKs1COM
    YX01a5MdJdZox2B2nPkwVrUdaUFois/WKMjG9Io3p3pjHeIS+/6ZxGXdWelvp9JN
    1hmQ/Xz7PYCYFe4fTdobcjdWQ0fzQ2q7
    -----END CERTIFICATE-----
    )EOF";
#endif

class OTA
{
public:
    static void loop();
    static void update();
};

#endif