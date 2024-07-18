# Possible Improvements

## Flow Control
Currently ISO-TP is only somewhat implemented.
The fragmentation is according to the standard,
but we use a custom implementation of flow-control that just echoes back the counter of the message as a kind of Acknowledge packet.

Maybe a standard compliant implementation could lead to better flashing performance, if the Flow-Control message isn't sent every packet, but only every couple of messages.

## CAN-Connector Errors
Currently errors with the CAN-Bus connection are shown in the console window(s), but there is no clear additional indicator of the vector adapter status.
Another indicator could be helpful.

## UDS Key mechanism
The key mechanism for login isn't implemented yet. Currently the switch to the programming mode can be done without additional authentification.

## Timestamps in Console
Currently there aren't any timestamps in the logs / console.

## Handling of UDS message reception in separate thread
The UI-Relevant UDS messages are sent from the UI thread -> if no response from the MCU, there is small delays in the order of 100ms.
Not very noticeable, but seperate thread would be better.

## Powersaving mode of PC sometimes disconnects Vector Adapter
Currently the connection has to be manually established again after power saving mode, by selecting "CAN" and the bitrate.
Maybe add automatic reconnection.

## "Custom" UDS Request Upload
Our UDS request upload semantics differ from the standard.
The standard sends back the contents of the flash, but as we only need the checksum for validation
we only send that instead.
Check if this is acceptable or if another service or custom service should be used instead.