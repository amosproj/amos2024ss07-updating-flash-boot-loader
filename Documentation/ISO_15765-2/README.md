# ISO 15765-2 / ISO-TP 

TODO: make this pretty

source of info currently [here](https://piembsystech.com/can-tp-protocol/)

[this is also nice](https://munich.dissec.to/kb/chapters/isotp/isotp.html).

| Frame Type            |  Byte 0                                        |  Byte 1         |    Byte 2    |  ...  |    Byte 7    |
|-----------------------|------------------------------------------------|-----------------|--------------|-------|--------------|
| Bit offset            | Bit 7 to 4 |     Bit 3 to 0                    | Bit 15 to 8     | Bit 23 to 16 |  ...  | Bit 63 to 56 |
| Single Frame          | FT = 0x00  | SFDL = 0x01 - 0x07                |   Data A        |    Data B    |  ...  |     Data G   |
| First Frame           | FT = 0x01  |         MFDL = 0x01 - 0x1000 (1 - 4096)             |    Data A    |  ...  |     Data G   |
| Consecutive Frame     | FT = 0x02  | Sequence/Index Number 0x00 - 0x15 |   Data A        |    Data B    |  ...  |     Data G   |
| Flow Control          | FT = 0x03  | FC Flag (0x00/0x01/0x02)          | Block Size (BS) |     STmin    |         NULL         |
