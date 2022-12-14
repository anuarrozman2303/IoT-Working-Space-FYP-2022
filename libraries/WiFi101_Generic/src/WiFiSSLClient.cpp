/**********************************************************************************************************************************
  WiFiSSLClient.cpp - Library for Arduino Wifi shield.

  Based on and modified from

  1) WiFi101 Library                 https://github.com/arduino-libraries/WiFi101

  Forked then modified by Khoi Hoang https://github.com/khoih-prog/WiFi101

  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      14/11/2022 Fix severe limitation to permit sending much larger data than total 4K
 ***********************************************************************************************************************************/

#include "WiFiSSLClient.h"

////////////////////////////////////////

WiFiSSLClient::WiFiSSLClient() :
  WiFiClient()
{
}

////////////////////////////////////////

WiFiSSLClient::WiFiSSLClient(uint8_t sock) :
  WiFiClient(sock)
{
}

////////////////////////////////////////

int WiFiSSLClient::connect(IPAddress ip, uint16_t port)
{
  return WiFiClient::connectSSL(ip, port);
}

////////////////////////////////////////

int WiFiSSLClient::connect(const char* host, uint16_t port)
{
  return WiFiClient::connectSSL(host, port);
}

////////////////////////////////////////

