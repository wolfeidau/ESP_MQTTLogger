/*
ESP_MQTTLogger.h - ESP8266 MQTT Data Logger library

  Copyright (c) 2014 Mark Wolfe. All rights reserved.

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
  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)

*/

#ifndef ESP_MQTTLogger_h
#define ESP_MQTTLogger_h

#include <functional>

#define DEBUG

class ESP_MQTTLogger
{
  public:
    ESP_MQTTLogger(IPAddress addr, Client& c, int port = 9000);
    ESP_MQTTLogger(Client& c, int port = 9000);
    void handleClient();
    void begin();
    void setToken(String token);
    void setInterval(int interval);
    void flip();
    bool connected();
    bool connect();
    void publish(String topic, String message);
 protected:
    void _loadMQTTUrl();
    bool _saveMQTTUrl();
    void _mqttSetup();
    bool _authenticate();
    bool _parseMQTTUrl(String mqtt_url);
    void _configureClient();
    int _port;
    String _token;
    String _mqttUrl;
    String _mqttHost;
    String _mqttUser;
    bool _sendMessage;
    ESP8266WebServer  _server;
    PubSubClient _client;
};

#endif //ESP_MQTTLogger_h
