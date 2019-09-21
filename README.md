# afv-euroscope-bridge
Provides a bridge between the Audio for Vatsim standalone client and EuroScope 3.2.

## What Does It Do?

The code compiles into a EuroScope plugin DLL that can receive messages from the Audio for Vatsim standalone client.
These messages tell the plugin the frequencies on which it should transmit and receive text messages.

## Message Format

All messages are strings in the following format:

```
FREQ:RCV:XMT
```

Where:

- FREQ is a 6 digit frequency
- RCV is a boolean (true or false) for whether text receive should be turned on.
- XMT is a boolean (true or false) for whether text transmit should be turned on.

## How To Send Messages

Messages can be sent using the `WM_COPYDATA` message provided by the Win32 API. These messages
should be sent to windows of the class `AfvBridgeHiddenWindowClass`.

## Sending Test Messages

When the plugin is compiled in debug mode, test messages may be sent using EuroScope's inbuilt
command functionality. The command is of the following format:

```
.afv FREQ RCV XMT
```

Where:

- FREQ is a 6 digit frequency
- RCV is a boolean (true or false) for whether text receive should be turned on.
- XMT is a boolean (true or false) for whether text transmit should be turned on.
