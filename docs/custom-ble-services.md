# Custom BLE services

In addition to the standard profiles, three custom BLE profiles/services are available that provide additional data for clients that implements them (e.g. currently the official [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/))

1. Extended Metrics Service (UUID: a72a5762-803b-421d-a759-f0314153da97)
2. Settings Service (UUID: 56892de1-7068-4b5a-acaa-473d97b02206)
3. Over-the-air update Service (UUID: ed249319-32c3-4e9f-83d7-7bb5aa5d5d4b)

## Extended Metrics Service

This Service currently contains three characteristics:

```text
Extended Metrics (UUID: 808a0d51-efae-4f0c-b2e0-48bc180d65c3)
```

Uses Notify to broadcast the following metrics (which may be extended in the future) as an array of consecutive bytes (i.e. currently a total of 7 bytes, Little Endian):

- avgStrokePower (16bit short in Watts)
- recoveryDuration (16bit unsigned short in seconds with a resolution of 4096)
- driveDuration (16bit unsigned short in seconds with a resolution of 4096)
- dragFactor (8bit unsigned char)

New metrics are broadcasted on every stroke (after the drive ends) or at least 4 seconds (which ever happens earlier).

```text
Handle Forces (UUID: 3d9c2760-cf91-41ee-87e9-fd99d5f129a4)
```

Uses Notify to broadcast the handle forces measured during the last drive phase. Full data is broadcasted once per stroke (after the drive ends) or at least 4 seconds (which ever happens earlier).

Considering that the number of measurements vary from stroke to stroke (since, among others, it depends on the number of impulses per rotation, the machine etc.) this characteristics may be chunked into consecutive notifies ("bursts") until all data is flushed. The chunk size (consequently the number of consecutive notifies within a burst) will depend on the MTU (max data size per broadcast) negotiated with the client (ESP32 supports 512 bytes, but for instance on android based on experience this is around 250).

The first byte in every Notify is the expected total number of chunks within the burst, the second is the current chunk number. Rest of the bytes in one Notify are 32bit floats in Little Endian. Every chunk can be parsed individually without data loss (i.e. the bytes of one float is never broken into two notifies, which prevents data loss on missed packages/notifies). Basically the last Notify within the burst is signaled by the fact that first two bytes of the data package are equal.

Below is an example of data where the MTU is 23 bytes (which practically means that 20 bytes would be available to transfer actual data). Considering the first two bytes are reserved, that leaves 18 bytes for the data, but since into that only 4 32bit float can be fitted basically a maximum of 4 floats per Notify can be sent.

Handle Forces values:
_[2.4188, 52.64054, 80.1877, 110.6412, 142.9242, 174.9786, 201.5447, 218.6083, 228.5825, 233.4143, 234.7116, 234.2218, 230.6765, 227.8172]_

Notifies:

1. [5,1,159,205,26,64,234,143,82,66,37,96,160,66,81,72,221,66]
2. [5,2,156,236,14,67,135,250,46,67,119,139,73,67,186,155,90,67]
3. [4,3,32,149,100,67,19,106,105,67,46,182,106,67,201,56,106,67]
4. [4,4,48,173,102,67,56,209,99,67]

The last Notify (4/4) has the data of only two floats while the rest has 4 each.

```text
Delta Times (UUID: ae5d11ea-62f6-4789-b809-6fc93fee92b9)
```

Uses Notify to broadcast the measured delta times if enabled. This serves mostly calibration/debugging purposes as the recorded delta times can be replayed and test various settings efficiently. This feature is disabled by default (meaning that this characteristic may not be visible). It can be enabled by defining `ENABLE_BLUETOOTH_DELTA_TIME_LOGGING true`. After that the actual notification of the measured delta times can be turned on or off via OpCode 19.

The measured delta times are broadcasted once sufficient elements to fill the max negotiated MTU (minus 3 for the header i.e. when the max data capacity) is reached or if 1 second since the last broadcast has passed.

Basically if the negotiated MTU is 255 then 63 delta times can be broadcasted ((255 - 3)/4 - assuming that unsigned integer is 4bytes on the system like on the ESP32). Actual frequency will depend on the number of impulses and the speed of the flywheel since.

In practice once the system measured 63 delta time value it will send Notify (or if 1 second elapses since the last Notify) to the connected clients. Please note that in certain cases this could be rather resource intensive (e.g. when there are a lot of impulses per rotation), the client should support and negotiate a minimum MTU of 100 (ESP32 NimBle stack supports up to 512bytes). If the MTU is below 100, no Notify will be sent.

The data in the Notify are 32bit unsigned integers in Little Endian.

## Settings Service

This Service currently contains three characteristics:

```text
Settings (UUID: 54e15528-73b5-4905-9481-89e5184a3364)
```

Uses Notify to broadcast and allow Read the current settings (which may be extended in the future) as an array of consecutive bytes. It notifies when a setting is changed.

Currently the Notify/Read includes the following data:

Byte 0 includes the logging and other general settings with a bit mask detailed below:

_Delta Time logging_ - whether to broadcast the measured delta times

```cpp
DeltaTimeLoggingNotSupported = (0x00 << 0U);
DeltaTimeLoggingDisabled = (0x01 << 0U);
DeltaTimeLoggingEnabled = (0x02 << 0U);
```

_SD Card logging_ - whether logging to SD Card is enabled or not

```cpp
LogToSdCardNotSupported = (0x00 << 2U);
LogToSdCardDisabled = (0x01 << 2U);
LogToSdCardEnabled = (0x02 << 2U);
```

_Log level setting_ - current log level for the monitor

```cpp
LogLevelSilent = (0x00 << 4U);
LogLevelFatal = (0x01 << 4U);
LogLevelError = (0x02 << 4U);
LogLevelWarning = (0x03 << 4U);
LogLevelInfo = (0x04 << 4U);
LogLevelTrace = (0x05 << 4U);
LogLevelVerbose = (0x06 << 4U);
```

_Runtime settings_ - whether runtime setting is enabled

```cpp
RuntimeSettingsDisabled = (0x00 << 7U);
RuntimeSettingsEnabled = (0x01 << 7U);
```

Bytes 1-4 are the [Flywheel Inertia](./settings.md#flywheel-inertia) as a positive float represented in unsigned chars (e.g. [0, 0, 128, 63] is 1.000). Response will be InvalidOperations if the value is out of bounds.

Byte 5 (unsigned char) is the [Magic Constant](./settings.md#concept_2_magic_number) with a resolution (scale) of 35 (i.e. value of 98 translates to 2.8)

Byte 6 (unsigned char) is the [Impulses per Revolution](./settings.md#impulses_per_revolution)

Bytes 7-8 (unsigned short) is the [Sprocket Radius](./settings.md#sprocket_radius) in centimeter with a resolution (scale) of 1,000 (i.e. value of 3,200 translates to 3.2)

Byte 9 (unsigned char) is the [Sensor Debounce Time](./settings.md#rotation_debounce_time_min) in milliseconds.

Byte 10 (unsigned char) is the [Rowing Stopped Threshold](./settings.md#rowing_stopped_threshold_period) in seconds (minimum value is 4 seconds).

Byte 11 (unsigned char) is the [Goodness of Fit Threshold](./settings.md#goodness_of_fit_threshold) for drag factor calculation with a resolution (scale) by 255 (i.e. value of 246 translates to ~0.96).

Byte 12 (unsigned char) is the [Max Drag Factor Recovery Period](./settings.md#max_drag_factor_recovery_period) in seconds.

Bytes 13-14 (unsigned short) is the [Lower Drag Factor Threshold](./settings.md#lower_drag_factor_threshold).

Bytes 15-16 (unsigned short) is the [Upper Drag Factor Threshold](./settings.md#upper_drag_factor_threshold).

Byte 17 (unsigned char) is the [Drag Coefficients Array Length](./settings.md#drag_coefficients_array_length).

```text
Stroke Detection Settings (UUID: 5d9c04cd-dcec-4551-8169-8c81f14d9d9d)
```

Uses Notify to broadcast and allow Read the current stroke phase detection settings (which may be extended in the future) as an array of consecutive bytes. It notifies when a stroke detection setting is changed.

Currently the Notify/Read includes the following data (15 bytes total):

Byte 0 includes the stroke detection type and impulse data array length with a bit mask detailed below:

_Stroke Detection Type_ - method used for stroke phase detection (bits 0-1)

```cpp
Torque = 0x00;
Slope = 0x01;
Both = 0x02;
```

_Impulse Data Array Length_ - number of impulses stored for calculations (bits 2-7, value range 1-63)

Bytes 1-2 (signed short) is the [Minimum Powered Torque](./settings.md#minimum_powered_torque) with a resolution (scale) of 10,000 (i.e. value of 5,000 translates to 0.5).

Bytes 3-4 (signed short) is the [Minimum Drag Torque](./settings.md#minimum_drag_torque) with a resolution (scale) of 10,000 (i.e. value of 2,500 translates to 0.25).

Bytes 5-8 (float as unsigned int) is the [Minimum Recovery Slope Margin](./settings.md#minimum_recovery_slope_margin) as a 32-bit float represented in little-endian format.

Bytes 9-10 (signed short) is the [Minimum Recovery Slope](./settings.md#minimum_recovery_slope) with a resolution (scale) of 1,000 (i.e. value of 1,500 translates to 1.5).

Bytes 11-13 (3 bytes) encode the minimum stroke timing values with a resolution (scale) in milliseconds:

- _Minimum Recovery Time_ - encoded in bits 0-11 (12 bits, max value 4,095, representing up to ~4.1 seconds)
- _Minimum Drive Time_ - encoded in bits 12-23 (12 bits, max value 4,095, representing up to ~4.1 seconds)

Byte 14 (unsigned char) is the [Drive Handle Forces Max Capacity](./settings.md#drive_handle_forces_max_capacity) (minimum value is 1).

```text
Settings Control Point (UUID: 51ba0a00-8853-477c-bf43-6a09c36aac9f)
```

Uses Indicate and allow Write to change the settings. The structure corresponds to the standard BLE profile Control Point with the difference that custom OpCodes are used for each setting:

```cpp
    SetLogLevel = 17U,
    ChangeBleService = 18U,
    SetDeltaTimeLogging = 19U,
    SetSdCardLogging = 20U,
    SetMachineSettings = 21U,
    SetSensorSignalSettings = 22U,
    RestartDevice = 31U,
```

The response to the Write is sent via Indicate and the structure follows the BLE Control Point standard (i.e. starts with the ResponseCode - 32 -, followed by the request OpCode, than the ResponseOpCode - e.g. 1 for success or 2 for an unsupported request OpCode).

Also a Notify is sent by the Settings characteristic including the new settings.

Please note that the new BLE service structure is currently experimental and the API may be subject to change in the future.

For an example of an implementation (in Javascript) please visit the [WebGUI page]((https://github.com/Abasz/ESPRowingMonitor-WebGUI/blob/master/src/common/services/ergometer/erg-settings.service.ts)).

Please note that `SetMachineSettings`, `SetSensorSignalSettings`, `SetDragFactorSettings`, `SetStrokeDetectionSettings` OpCodes only available if firmware is compiled with [`ENABLE_RUNTIME_SETTINGS`](./settings.md#enable_runtime_settings) flag. Otherwise sending this OpCode it will return `UnsupportedOpCode`. Furthermore, device restart (e.g. sending Restart Device OpCode) is necessary for the new settings to take effect. The structure corresponds to bytes of the Settings Characteristic and Stroke Detection Settings Characteristic above as follows:

- `SetMachineSettings` follows bytes 1-8 of Settings Characteristic
- `SetSensorSignalSettings` follows bytes 9-10 of Settings Characteristic
- `SetDragFactorSEttings` follows bytes 11-17 of Settings Characteristic
- `SetStrokeDetectionSettings` follows the bytes 1-15 of Stroke Detection Settings Characteristic

## Over-the-Air updater

This service contains two characteristics:

```text
Over-the-Air receive (RX) data (UUID: fbac1540-698b-40ff-a34e-f39e5b78d1cf)
```

The purpose of this characteristic is to receive the OTA commands as well as the binary data from the clients.

```text
Over-the-Air transfer (TX) data (UUID: b31126a7-a29b-450a-b0c2-c0516f46b699)
```

The purpose of this characteristic is to send the responses (e.g. errors, data acknowledgement, confirmations, etc.) to the clients during the OTA process.

### OTA Protocol

The ESPRM OTA protocol is a two way binary message system where the clients can send commands and blob data (in unsigned char format):

```cpp
enum class OtaRequestOpCodes : unsigned char
{
    Begin = 0x00,
    Package = 0x01,
    End = 0x02,
    Abort = 0x03,
};
```

and should receive certain responses on the progress:

```cpp
enum class OtaResponseOpCodes : unsigned char
{
    Ok = 0x00,
    NotOk = 0x01,
    IncorrectFormat = 0x02,
    IncorrectFirmwareSize = 0x03,
    ChecksumError = 0x04,
    InternalStorageError = 0x0ó5,
};
```

All data transfers are in little endian format. Actual firmware data is transferred in a way (for improving transfer speed) that before writing the data to flash bytes are written to a buffer allocated on the heap, and once that is filled acknowledgement is sent to the client and flash is written.

This should mean that clients should wait for a response only when buffer is filled and should only proceed with writing on `Ok` response.

The maximum size of one full message is limited by the negotiated MTU. However the _per package payload_ (i.e. the bytes of the blob per message) is limited to MTU - 4 bytes (3bytes for BLE headers and one byte for the request OpCode). The buffer is currently 40 times the _per package payload_ (i.e. `(MTU - 4) * 40`), though these information are communicated as as part of the response message payload for the `Begin` command as buffer size could be changed.

#### Base case and happy path

1. The OTA process should be initiated by the client with the `Begin` command including the size of the new firmware in bytes as unsigned 32 bit int: `[0, 176, 113, 11, 0]` (corresponding to `Begin` with firmware size of 750,000 bytes)
2. ESPRM sends `Ok` response with the _per package payload_ size (MTU - 4) and the buffer size (_per package payload_ * 40) as two unsigned 32bit integer (i.e. 8 bytes) - [0, 250, 0, 0, 0, 16, 39, 0, 0] (corresponding to `Ok` with _per package payload_ of 250 bytes and buffer size of 10,000 bytes)
3. Client should send `Package` command with the payload (bytes of the firmware) until buffer is full and then it should wait for a response
4. Once buffer is full ESPRM will write the data to flash and once that is done send `Ok` response
5. Step 3 and 4 should be repeated until all bytes of the firmware file is transferred
6. Once all data is sent the client should send `End` command with the MD5 hash (calculated as a 128-bit cryptographic digest for all input bytes of the firmware file) as an unsigned char byte array.
7. ESPRM send `Ok` response code if install is successful and will restart

#### Special scenarios

The `Begin` command always resets the previous upload session and starts everything from scratch.  This means that in case `Begin` command is sent again before completing the OTA process, the so far sent data is deleted and the whole process reset to the beginning.

Calling `End` before transferring all data will terminate the OTA process with an MD5 checksum error (assuming that the MD5 checksum is calculated based on all bytes of the firmware file). Invalid or defected firmware file should in general not brick the board as ESP has a fails safe in case new OTA partition is not bootable and reverts back to the last working partition.

Calling `Abort` will terminate and reset the current OTA process, without initiating a new one. Response is `Ok` when everything is reset.

#### Error codes

`Begin` command will produce an error when:

- the payload (firmware size) is not 32bit (i.e. not 4 bytes long) - response OpCode of `IncorrectFormat`
- firmware file is too big - response OpCode of `IncorrectFirmwareSize`
- device partition is mal formed (e.g. no OTA partition) - response OpCode of `InternalStorageError`
- unknown error occurred while initiating OTA updater - response OpCode of `NotOk`

`Package` command will produce an error when:

- OTA process has not been started with the `Begin` command - response OpCode of `NotOk`
- file is not a valid firmware file - response OpCode of `IncorrectFirmwareSize`
- device partition is mal formed (e.g. no OTA partition) - response OpCode of `ChecksumError`
- other error while writing data to flash - response OpCode of `InternalStorageError`

`End` command will produce an error when:

- OTA process has not been started with the `Begin` command - response OpCode of `NotOk`
- the payload (MD5 hash byte array) is not a valid MD5 hash (i.e. not 16 bytes long) - response OpCode of `IncorrectFormat`
- calculated MD5 hash based on received data does not match the received hash received as part of the `End` command - response OpCode of `ChecksumError`
- unknown error occurred while installing the new firmware - response OpCode of `NotOk`
