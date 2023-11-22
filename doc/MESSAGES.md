# List of supported messages and their descriptors

This list contains a number of messages that ledger could assemble and display critical information (hints) about what's this transaction is about.

| ID | Message | Description |
| --- | --- | --- |
| 0x00 | Message with comment | Typical transaction with a comment |
| 0x01 | Jetton transfer | [TEP-74 transfer message](https://github.com/ton-blockchain/TEPs/blob/master/text/0074-jettons-standard.md#1-transfer) |

# 0x00: Message with comment

This is just a usual transaction with a comment, but it have it's limitations:
* Only ASCII-printable symbols are supported
* 120 symbols max

### TL-B
```
message#00000000 text:string = MsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `message` | 0-120 | ASCII-only message |

# 0x01: Jetton transfer

### TL-B
```
transfer#0f8a7ea5 query_id:uint64 amount:(VarUInteger 16) destination:MsgAddress
                 response_destination:MsgAddress custom_payload:(Maybe ^Cell)
                 forward_ton_amount:(VarUInteger 16) forward_payload:(Either Cell ^Cell)
                 = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `amount` | `varuint` | Jetton amount |
| `destination` | `address` | Whom to transfer jettons to |
| `response_destination` | `address` | Whom to transfer the excess of TON to |
| `has_custom_payload` | 1 | Whether `custom_payload` is present |
| `custom_payload` | 0 or `cell_ref` | `custom_payload` for the message |
| `forward_amount` | `varuint` | Amount of TON to forward to the receiver |
| `has_forward_payload` | 1 | Whether `forward_payload` is present |
| `forward_payload` | 0 or `cell_ref` | `forward_payload` for the message |
