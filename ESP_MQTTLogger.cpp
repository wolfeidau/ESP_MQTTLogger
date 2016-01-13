/*
ESP_MQTTLogger.cpp - ESP8266 MQTT Data Logger library

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
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "ESP_MQTTLogger.h"

#define DEBUG_OUTPUT Serial

const char * MQTT_AUTHORIZATION_HEADER = "Authorization";

ESP_MQTTLogger::ESP_MQTTLogger(IPAddress addr, Client& c, int port) :
  _server(addr, port),
  _client(c)
{
}

ESP_MQTTLogger::ESP_MQTTLogger(Client& c, int port) :
  _server(port),
  _client(c)
{
}

void ESP_MQTTLogger::handleClient() {
  _server.handleClient();
  if (_client.connected()) {
    _client.loop();
  }
}

void ESP_MQTTLogger::begin() {

  // filesystem setup
  if (!SPIFFS.begin()) {

    DEBUG_OUTPUT.println("Failed to mount file system");

  }

  _loadMQTTUrl(); // attempt to load the mqtt credentials

  _server.on("/mqttSetup", [this]() {
    _mqttSetup();
  });

  //ask server to track these headers which we need for authentication
  const char * headerkeys[] = {"Authorization"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  _server.collectHeaders(headerkeys, headerkeyssize);

  _server.begin();

}

void ESP_MQTTLogger::setToken(String token) {
  _token = token;
}

bool ESP_MQTTLogger::connected() {
  return _client.connected();
}

bool ESP_MQTTLogger::connect() {
  if (_mqttUrl != "") {
    return _client.connect(MQTT::Connect("arduinoClient")
       .set_auth(_mqttUser, ""));
  }
}

void ESP_MQTTLogger::publish(String topic, String message) {
  if (connected()) {
    char tmp[15];
    sprintf(tmp, "esp8266-%06x", ESP.getChipId());
    String t = String(tmp) + String('/') + topic;
    _client.publish(t, message);
  }
}

void ESP_MQTTLogger::_loadMQTTUrl() {

  File mqttUrlFile = SPIFFS.open("/mqtt_url.txt", "r");

  if (mqttUrlFile) {

#ifdef DEBUG
   DEBUG_OUTPUT.println("MQTT URL config exists.");
#endif

    _mqttUrl = mqttUrlFile.readString();

    if (_parseMQTTUrl(_mqttUrl)){
      _configureClient();
    }

    return;
  }

  DEBUG_OUTPUT.println("WARN: Failed to open URL config");
}

bool ESP_MQTTLogger::_saveMQTTUrl() {

  File mqttUrlFile = SPIFFS.open("/mqtt_url.txt", "w");
  if (!mqttUrlFile) {

    DEBUG_OUTPUT.println("WARN: Failed to open mqtt_url.txt file for writing");

    return false;
  }

#ifdef DEBUG
  DEBUG_OUTPUT.print("saving mqttUrl: ");
  DEBUG_OUTPUT.println(_mqttUrl);
#endif

  mqttUrlFile.print(_mqttUrl);

  return true;
}

void ESP_MQTTLogger::_mqttSetup() {

  if (!_authenticate()){
    _server.send(401, "text/plain", "Unauthorised");
    return;
  }

  if(_server.hasArg("mqtt_url")) {

    if (!_parseMQTTUrl(_server.arg("mqtt_url"))){
      _server.send(400, "text/plain", "Invalid URL");
      return;
    }

    _mqttUrl = _server.arg("mqtt_url");

#ifdef DEBUG
    DEBUG_OUTPUT.print("configuring mqttUrl: ");
    DEBUG_OUTPUT.println(_mqttUrl);
#endif

    if (!_saveMQTTUrl()) {
    _server.send(500, "text/plain", "Save configuration failed");
    }
    _server.send(200, "text/plain", "OK");

    return;
  }

  _server.send(400, "text/plain", "");
}

bool ESP_MQTTLogger::_authenticate(){

  if(_server.hasHeader(MQTT_AUTHORIZATION_HEADER)) {
    String authReq = _server.header(MQTT_AUTHORIZATION_HEADER);

#ifdef DEBUG
    DEBUG_OUTPUT.print("authReq: ");
    DEBUG_OUTPUT.println(authReq);
#endif

    if(authReq.startsWith("Token")){
      authReq = authReq.substring(6);
      authReq.trim();

      if (authReq.equals(_token)){
        return true;
      }
    }
  }

  return false;
}

// This is a very primitive URL parsing routine which needs
// to be replaced at some stage.
bool ESP_MQTTLogger::_parseMQTTUrl(String mqtt_url){
  if (!mqtt_url.startsWith("mqtt://")) {
    return false;
  }
  if (mqtt_url.indexOf("@") <= 0) {
    return false;
  }

  mqtt_url = mqtt_url.substring(7);

  int at = mqtt_url.indexOf("@");

  _mqttUser = mqtt_url.substring(0, at-1);
  _mqttHost = mqtt_url.substring(at+1);

#ifdef DEBUG
    DEBUG_OUTPUT.print("user: ");
    DEBUG_OUTPUT.println(_mqttUser);
    DEBUG_OUTPUT.print("host: ");
    DEBUG_OUTPUT.println(_mqttHost);
#endif

    return true;
}

void ESP_MQTTLogger::_configureClient() {
#ifdef DEBUG
  DEBUG_OUTPUT.println("configure MQTT client");
#endif
  _client.set_server(String(_mqttHost));
}
