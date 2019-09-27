# afv-euroscope-bridge
Provides a bridge between the Audio for Vatsim standalone client and EuroScope 3.2.

# What Does It Do?

The code compiles into a EuroScope plugin DLL that can receive messages from, and do work in EuroScope for, the Audio for Vatsim standalone client.

## Automatic Setting of Text Frequencies

The Audio for Vatsim standalone client can send messages to the plugin, which will automatically toggle whether or not certain frequencies, as defined in EuroScope are setup to send and receive text messages.

### Message Format

All messages are strings in the following format:

```
FREQ:RCV:XMT
```

Where:

- FREQ is a 6 digit frequency
- RCV is a boolean (true or false) for whether text receive should be turned on.
- XMT is a boolean (true or false) for whether text transmit should be turned on.

### How To Send Messages

Messages can be sent using the `WM_COPYDATA` message provided by the Win32 API. These messages
should be sent to windows of the class `AfvBridgeHiddenWindowClass`.

### Sending Test Messages

When the plugin is compiled in debug mode, test messages may be sent using EuroScope's inbuilt
command functionality. The command is of the following format:

```
.afv FREQ RCV XMT
```

Where:

- FREQ is a 6 digit frequency
- RCV is a boolean (true or false) for whether text receive should be turned on.
- XMT is a boolean (true or false) for whether text transmit should be turned on.

## Automatic Monitoring of ATIS Frequencies

The plugin regularly checks the status of all defined frequencies. If a frequency has been marked as an active ATIS, the plugin will automatically enable the sending and receiving of text on that frequency.
