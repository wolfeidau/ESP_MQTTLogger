# ESP_MQTTLogger

This library handles setting up an MQTT logger using a very basic REST interface.

# Usage

* Use curl to configure the Wifi credentials for your network replacing xxxx with the value of the Access Token, and configuring the ssid and password for your network.

```
curl -v -H "Authorization: Token xxxx" 'http://X.X.X.X:9000/mqttSetup?mqtt_url=mqtt://username:password@x.x.x.x'
```

* A username is absolutely mandatory for the URL to be valid for mqtt and it is obviously preferable to secure your MQTT server properly anyway with ACLs.

# Links

* [ESP8266 Arduino environment](https://github.com/esp8266/Arduino)
* [esptool.py](https://github.com/themadinventor/esptool)

# License

This code is released under the LGPL license see the LICENSE.md file for more details.
