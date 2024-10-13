**General Discription:**

Data Mapping

| Discrete input (1byte) |     | Coil output(1 byte) |     |
| --- | --- | --- | --- |
| Bit |     | Bit |     |
| 0   | Input Signal 1 | 0   | Output Signal 1 |
| 1   | Input Signal 2 | 1   | Output Signal 2 |
| 2   | Input Signal 3 | 2   | Output Signal 3 |
| 3   | Input Signal 4 | 3   | Output Signal 4 |
| 4   | Input Signal 5 | 4   | Output Signal 5 |
| 5   | Input Signal 6 | 5   | Output Signal 6 |
| 6   | Input Signal 7 | 6   | Output Signal 7 |
| 7   | Input Signal 8 | 7   | Output Signal 8 |
| Input Register (4 bytes) |     | Output Register (4 bytes) |     |
| 1   | Analog input 1 | 1   | Analog output 1 |
| 2   | Analog input 2 | 2   | Analog output 2 |
| 3   | Analog input 3 | 3   | Analog output 3 |
| 4   | Analog input 4 | 4   | Analog output 4 |

**Protocol description:**

**Read digital register (1 byte)**

Message Request:

| Slave Address | Function Code | CRC |
| --- | --- | --- |
| 1 byte | 0x01 | 2 bytes |

Message response:

| Slave Address | Function Code | Data | CRC check |
| --- | --- | --- | --- |
| 1 byte | 1 byte | 1 bytes | 2 bytes |

Example:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x01 | 0x01 |
| Data | \-  | 0x05 |
| CRC 1 | 0xCD | 0xB1 |
| CRC 2 | 0xB0 | 0x96 |

![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/1.png)
**Read holding register (n bytes)**

Message Request:

| Slave Address | Function Code | CRC |
| --- | --- | --- |
| 1 byte | 0x02 | 2 bytes |

Message response:

| Slave Address | Function Code | Data | CRC check |
| --- | --- | --- | --- |
| 1 byte | 1 byte | 4 bytes | 2 bytes |

Example:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x02 | 0x02 |
| Data | \-  | 0x01 0x05 0x07 0xCA |
| CRC 1 | 0x8D | 0xE8 |
| CRC 2 | 0xB1 | 0xD1 |

![]
![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/2.png)
**Write Coil**

Message Request:

| Slave Address | Function Code | Data | CRC |
| --- | --- | --- | --- |
| 1 byte | 0x03 | 1 byte | 2 bytes |

Message response:

| Slave Address | Function Code | Data | CRC check |
| --- | --- | --- | --- |
| 1 byte | 1 byte | 1 bytes | 2 bytes |

Example:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x03 | 0x03 |
| Data | \-  | 0x01 |
| CRC 1 | 0xB1 | 0xB1 |
| CRC 2 | 0x35 | 0x35 |


![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/3.png)
**Write Holding Register**

Message Request:

| Slave Address | Function Code | Data | CRC |
| --- | --- | --- | --- |
| 1 byte | 0x04 | 4 byte | 2 bytes |

Message response:

| Slave Address | Function Code | Data | CRC check |
| --- | --- | --- | --- |
| 1 byte | 1 byte | 4 bytes | 2 bytes |

Example:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x04 | 0x04 |
| Data | 0x01 0x01 0x01 0x01 | 0x01 0x01 0x01 0x01 |
| CRC 1 | 0x63 | 0x63 |
| CRC 2 | 0x27 | 0x27 |


![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/4.png)
**Error**

Message Request:

| Slave Address | Function Code | CRC |
| --- | --- | --- |
| 1 byte |     | 2 bytes |

Message response:

Error code: 0x00

Exception Error code:

\+ 0x00 – _CRC mismatch_

\+ 0x01 – _Invalid Function Code_

| Slave Address | Error code | Exception Code | CRC check |
| --- | --- | --- | --- |
| 1 byte | 0x00 | 0x01 | 2 bytes |

Example:

CRC mismatch:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x02 | 0x00 |
| Data | \-  | 0x00 |
| CRC 1 | 0x8D | 0x70 |
| CRC 2 | 0xB0 | 0x05 |


![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/5.png)
Invalid Function Code:

|     | Request | Response |
| --- | --- | --- |
| Slave adress | 0x10 | 0x10 |
| Function code | 0x05 | 0x00 |
| Data | \-  | 0x01 |
| CRC 1 | 0xCC | 0xB1 |
| CRC 2 | 0x73 | 0xC5 |


![](/Project%20custom%20protocol/Project%20Custom%20Protocol/docs/Images/6.png)
