## UDS Communication Documentation for Flashbootloader

This document describes the concept of the UDS communication that is used for this project. Mainly there is only a suitable subset of the UDS protocol implemented so that the bootloader stays as simple as possible. Furthermore this means that there adaptions as necessary for this project. For this documentation it is assumed that the ECU is identified by ID <span style="color:green">"0xAB"</span> and the GUI is identified by ID <span style="color:yellow">"0xEE"</span>. 

### Underlaying Software architecture

![Software Architecture](../Architecture/AMOS_Flashbootloader_SW_Architecture.png)

The main logic for the UDS communication is located in the UDS Layer of the Windows GUI framework on the one side and in the UDS Layer of the MCU. In general the communication can be handled via CAN, CAN FD or Ethernet. The details for the different technologies is not part of the documentation.

## Research on UDS
To get a more detailed insight into the UDS protocol the following sources have been used:
- [CSS Electronics - UDS Explained - A Simple Intro (Unified Diagnostic Services)](https://www.csselectronics.com/pages/uds-protocol-tutorial-unified-diagnostic-services)
- [Automotive Softing - UDS ISO 14229](https://automotive.softing.com/fileadmin/sof-files/pdf/de/ae/poster/UDS_Faltposter_softing2016.pdf)

## Protocol description for Flashbootloader

- A message has a dynamic length, the maximum size is XX byte
- A message always includes a destination adresse (1 byte)
- The length information is always included in the message
- A message always needs at least a payload of 1 byte
- If the length of the message is too short, it will be discarded
- If the length of the message is too long, only the transmitted length will be evaluated
- The programming session is closed if there is no communication for XX seconds
- Invalid or unclear messages are not responded by the ECU
- General payload: [destination adress][length of message (paylod)][bytes of payload] 
- For the positive ack response 0x40 is added to the SID

## Supported Service Overview 

### Diagnostic and Communication Management
| <span style="color:red">$SID</span> | Available in Default Session | Has Sub-Function | Service Name               | 
|------|------------------------------|------------------|----------------------------|
| $10  | &#9745;                      | &#9745;          | Diagnostic Session Control |  
| $11  | &#9745;                      | &#9745;          | ECU Reset                  |
| $27  | &#9745;                      | &#9745;          | Security Access            |
| $3E  | &#9745;                      | &#9745;          | Tester Present             |

### Data Transmission
| $SID | Available in Default Session | Has Sub-Function | Service Name               | 
|------|------------------------------|------------------|----------------------------|
| $22  | &#9745;                      |                  | Read Data By Identifier    |
| $23  |                              |                  | Read Memory By Address     |
| $2E  |                              | &#9745;          | Write Memory By Address    |

### Upload | Download
| $SID | Available in Default Session | Has Sub-Function | Service Name               | 
|------|------------------------------|------------------|----------------------------|
| $34  |                              |                  | Request Download           |
| $35  |                              |                  | Request Upload             |
| $36  |                              |                  | Transfer Data              |
| $37  |                              |                  | Request Transfer Exit      |

## Common Response Codes

| HEX | Description                                     |
|-----|-------------------------------------------------|
| 10  | General reject                                  |
| 11  | Service not supported                           |
| 12  | Sub-Function not supported                      |
| 13  | Incorrect msg len or invalid format             |
| 14  | Response too long                               |
| 21  | Busy repeat request                             |
| 22  | Conditions not correct                          |
| 24  | Request sequence error                          |
| 26  | Failure prevents execution of requested action  | 
| 31  | Request out of range                            |
| 33  | Security access denied                          |
| 35  | Invalid key                                     |
| 36  | Exceeded number of attempts                     |
| 37  | Required time delay not expired                 |
| 70  | Upload/Download not accepted                    |
| 71  | Transfer data suspended                         |
| 72  | General programming failure                     |
| 73  | Wrong Block Sequence Counter                    |
| 7E  | Sub-Function not supported in active session    |
| 7F  | Service not supported in active session         |

## Session Handling

![Session Handling](./Session_Handling.png)

### Diagnostic Session Control (0x10)
- General Request: [identifier][length][<span style="color:red">\$SID</span>][Session]
  
#### Default Session
| Type | Bytes |
|---|---|
|Request| [<span style="color:green">0xAB</span>][0x02][<span style="color:red">0x10</span>][0x01] |
|Response| [<span style="color:yellow">0xEE</span>][0x02][<span style="color:red">0x50</span>][0x01] |

#### Programming Session
| Type | Bytes |
|---|---|
| Request | [<span style="color:green">0xAB</span>][0x02][<span style="color:red">0x10</span>][0x02] |
| Response | [<span style="color:yellow">0xEE</span>][0x02][<span style="color:red">0x50</span>][0x02] |

#### Wrong/Unavailable Session
| Type | Bytes |
|---|---|
| Request | [<span style="color:green">0xAB</span>][0x02][<span style="color:red">0x10</span>][0x05] |
| Response | [<span style="color:yellow">0xEE</span>][0x03][0x7F][0x10][0x31] |