# List of supported messages and their descriptors

This list contains a number of messages that ledger could assemble and display critical information (hints) about what's this transaction is about.

| ID | Message | Description |
| --- | --- | --- |
| 0x00 | Message with comment | Typical transaction with a comment |
| 0x01 | Jetton transfer | [TEP-74 transfer message](https://github.com/ton-blockchain/TEPs/blob/master/text/0074-jettons-standard.md#1-transfer) |
| 0x02 | NFT transfer | [TEP-62 transfer message](https://github.com/ton-blockchain/TEPs/blob/master/text/0062-nft-standard.md#1-transfer) |
| 0x03 | Jetton burn | [TEP-74 burn message](https://github.com/ton-blockchain/TEPs/blob/master/text/0074-jettons-standard.md#2-burn) |
| 0x04 | Add whitelist | [Vesting add whitelist message](https://github.com/ton-blockchain/vesting-contract?tab=readme-ov-file#whitelist) |
| 0x05 | Single Nominator withdraw | [Single nominator withdraw message](https://github.com/orbs-network/single-nominator/tree/main?tab=readme-ov-file#1-withdraw) |
| 0x06 | Single Nominator change validator address | [Single nominator change validator message](https://github.com/orbs-network/single-nominator/tree/main?tab=readme-ov-file#2-change-validator) |
| 0x07 | Tonstakers deposit | [Tonstakers deposit message](https://github.com/ton-blockchain/liquid-staking-contract/blob/be2ee6d1e746bd2bb0f13f7b21537fb30ef0bc3b/contracts/interaction.tlb#L52) |
| 0x08 | Jetton DAO vote for proposal | [Jetton DAO vote for proposal message](https://github.com/EmelyanenkoK/jetton_dao/blob/02fed5d124effd57ea50be77044b209ad800a621/contracts/voting.tlb#L61) |
| 0x09 | Change DNS record | [Change DNS record message](https://github.com/ton-blockchain/dns-contract/blob/d08131031fb659d2826cccc417ddd9b98476f814/func/nft-item.fc#L204) |
| 0x0A | Token bridge pay for swap | [Token bridge pay for swap message](https://github.com/ton-blockchain/token-bridge-func/blob/3346a901e3e8e1a1e020fac564c845db3220c238/src/func/jetton-bridge/op-codes.fc#L20) |

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
| `flags` | 1 | Bit +1 - whether `query_id` is present, bit +2 - whether this is a known jetton (bit +2 is not available on nano S) |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if not present. Present only when `flags & 1` |
| `jetton_id` | 0 or 2 | ID of the known jetton. Present only when `flags & 2` |
| `owner_workchain` | 0 or 1 | Workchain of owner. Present only when `flags & 2` |
| `amount` | `varuint` | Jetton amount |
| `destination` | `address` | Whom to transfer jettons to |
| `response_destination` | `address` | Whom to transfer the excess of TON to |
| `has_custom_payload` | 1 | Whether `custom_payload` is present |
| `custom_payload` | 0 or `cell_ref` | `custom_payload` for the message |
| `forward_amount` | `varuint` | Amount of TON to forward to the receiver |
| `has_forward_payload` | 1 | Whether `forward_payload` is present |
| `forward_payload` | 0 or `cell_ref` | `forward_payload` for the message |

# 0x02: NFT transfer

### TL-B
```
transfer#5fcc3d14 query_id:uint64 new_owner:MsgAddress response_destination:MsgAddress custom_payload:(Maybe ^Cell) forward_amount:(VarUInteger 16) forward_payload:(Either Cell ^Cell) = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `new_owner` | `address` | Whom to transfer jettons to |
| `response_destination` | `address` | Whom to transfer the excess of TON to |
| `has_custom_payload` | 1 | Whether `custom_payload` is present |
| `custom_payload` | 0 or `cell_ref` | `custom_payload` for the message |
| `forward_amount` | `varuint` | Amount of TON to forward to the receiver |
| `has_forward_payload` | 1 | Whether `forward_payload` is present |
| `forward_payload` | 0 or `cell_ref` | `forward_payload` for the message |

# 0x03: Jetton burn

### TL-B
```
burn#595f07bc query_id:uint64 amount:(VarUInteger 16)
              response_destination:MsgAddress custom_payload:(Maybe ^Cell)
              = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `amount` | `varuint` | Jetton amount |
| `response_destination` | `address` | Whom to transfer the excess of TON to |
| `custom_payload_type` | 1 | 0 - none, 1 - `cell_ref`, 2 - `cell_inline` |
| `custom_payload` | 0 or `cell_ref` or `cell_inline` | `custom_payload` for the message |

# 0x04: Add whitelist

### TL-B
```
add_whitelist#7258a69b query_id:uint64 address:MsgAddress = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `address` | `address` | Whom to add to the whitelist |

# 0x05: Single Nominator withdraw

### TL-B
```
withdraw#1000 query_id:uint64 amount:Coins = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `amount` | `varuint` | Amount to withdraw |

# 0x06: Single Nominator change validator address

### TL-B
```
change_validator_adddress#1001 query_id:uint64 new_validator_address:MsgAddress = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `address` | `address` | New validator address |

# 0x07: Tonstakers deposit

### TL-B
```
deposit#47d54391 query_id:uint64 app_id:uint64 = InternalMsgBody;
```
or
```
deposit#47d54391 query_id:uint64 = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `has_app_id` | 1 | Whether `app_id` is present |
| `app_id` | 0 or 8 | `app_id` for the message, will not be stored if `!has_app_id` |

# 0x08: Jetton DAO vote for proposal

### TL-B
```
vote#69fb306c query_id:uint64 voting_address:MsgAddressInt expiration_date:uint48 vote:Bool need_confirmation:Bool = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `voting_address` | `address` | Voting address |
| `expiration_date` | 6 | Expiration timestamp |
| `vote` | 1 | Vote flag |
| `need_confirmation` | 1 | Need confirmation flag |

# 0x09: Change DNS record

### TL-B
```
change_dns_record#4eb1f0f9 query_id:uint64 key:uint256 = InternalMsgBody;
```
or
```
change_dns_record#4eb1f0f9 query_id:uint64 key:uint256 value:^Cell = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `has_value` | 1 | Whether to set the value. If 0, the record will be deleted |
| `record_type` | 1 | Record type (see below for available types) |
| `record_data` | ? | Continued record data, depends on record type (see below for types) |

### Record types

#### 0x00: Wallet

`key` is `sha256("wallet")` (will be set by the device automatically).
`record_data` layout (if `has_value`):
| Value | Length or type | Description |
| --- | --- | --- |
| `address` | `address` | Wallet address |
| `has_capabilities` | 1 | Whether capabilities should be stored |
| `is_wallet` | 0 or 1 | Whether to store the `is_wallet` capability. Only present if `has_capabilities` |

If `!has_value`, then `record_data` is empty (0 bytes long)

##### TL-B
```
cap_is_wallet#2177 = SmcCapability;

cap_list_nil$0 = SmcCapList;
cap_list_next$1 head:SmcCapability tail:SmcCapList = SmcCapList;

dns_smc_address#9fd3 smc_addr:MsgAddressInt flags:(## 8) { flags <= 1 }
  cap_list:flags . 0?SmcCapList = DNSRecord;
```

#### 0x01: Unknown

Used for unknown keys. Key will be converted to base64 and displayed, value cell's hash will be converted to base64 and displayed. The key will not be hashed again.

`record_data` layout:
| Value | Length or type | Description |
| --- | --- | --- |
| `key` | 32 | Record key |
| `value` | 0 or `cell_ref` | Value cell. Only present if `has_value` |

# 0x0A: Token bridge pay for swap

### TL-B
```
pay_swap#8 query_id:uint64 swap_id:uint256 = InternalMsgBody;
```

### Hints
| Value | Length or type | Description |
| --- | --- | --- |
| `has_query_id` | 1 | Whether `query_id` is present |
| `query_id` | 0 or 8 | `query_id` for the message, 0 will be used if `!has_query_id` |
| `swap_id` | 32 | The swap ID |
