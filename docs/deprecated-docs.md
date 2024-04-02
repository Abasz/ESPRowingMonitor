# Documentation of Deprecated features

## Web interface

ESP Rowing Monitor includes a WebSocket server that sends calculated metrics to connected clients (up to a maximum of 2 clients at a time). In order to improve efficiency the data is sent in binary format. There are two types of data structure:

### Measurements

Measurements are sent in an array format as follows:

```typescript
{
    data: [number, number, number, number, number, number, number, number, Array<number>, Array<number>];
}
```

This array can be translated as follows based on the position:

0. revTime
1. distance
2. strokeTime
3. strokeCount
4. avgStrokePower
5. driveDuration
6. recoveryDuration
7. dragFactor
8. handleForces
9. deltaTimes (for logging purposes if enabled, if not one zero in the array is sent) since the last broadcast

### Settings and battery level

The settings and the battery level is no longer sent along with every broadcast (to save on execution time), but rather

1. on client connection
2. after saving new settings
3. on new battery measurement

The data follows the below structure.

```typescript
{
    logToWebSocket: boolean | undefined;
    logToSdCard: boolean | undefined;
    bleServiceFlag: BleServiceFlag;
    logLevel: LogLevel;
    batteryLevel: number;
}
```

Data is broadcasted on every stroke or every 4 seconds, whichever occurs earlier. This is not a significant issue, as most of the metrics are only available at certain known states of the rowing cycle (e.g. end of drive, end of recovery, etc.).

The WebGUI can also be served up by the ESP32 micro controller and accessed over HTTP on the ESP32s IP Address.

Please note that the filesystem of the ESP32 is rather slow, so the first load of the WebGUI may take up to half a minute. However, after the initial load, unless the files are modified, it will load instantly (potential reloading is controlled automatically via HTTP Last-Modified header).

### Network settings

These settings are required for the websocket to work

#### ENABLE_WEBSOCKET_MONITOR

Enables or disables WebSocket monitor. For more details please refer to the [Wi-Fi](#wi-fi-and-websocket-monitor) section. Default is true.

#### DISABLE_WEBSOCKET_DELTA_TIME_LOGGING

By default deltaTime logging is allowed if WebSocket monitor is enabled. However, its possible to disable it by defining `DISABLE_WEBSOCKET_DELTA_TIME_LOGGING`.

#### PORT

The port number that should be used by ESP32 MCU when creating the websocket server, default is 80.

#### ENABLE_WEBGUI

Enables or disables the WebGUI. ESP Rowing Monitor is capable os serving a WebGUI that communicates to the monitor via WebSocket. It requires `ENABLE_WEBSOCKET_MONITOR` to be true. Furthermore a filesystem image with the static resources is required. The WebGUI can be found under a separate repository [ESP Rowing Monitor WEbGUI](https://github.com/Abasz/ESPRowingMonitor-WebGUI), please refer to the [Build/Install](https://github.com/Abasz/ESPRowingMonitor-WebGUI#buildinstall) section. Default is false.

### Wi-Fi and WebSocket monitor

The Wi-Fi credentials are set up in a way that it cannot be accidentally committed to GitHub. Hence it should be set in a separate file named `wifi-config.ini` as follows:

```ini
[env:esp32]
build_flags = 
 -O2
 -std=c++2a
 -std=gnu++2a
    '-D LOCAL_SSID=My SSID'
    '-D PASSPHRASE=MySecretPWD'
```

File name is critical as that is the name that platformio.ini includes when compiling. Please note if no wifi-config.ini is provided (or the `LOCAL_SSID` and/or `PASSPHRASE` is not provided as a macro variable) WebSocket monitor will be disabled by the compiler.