# TON Transaction Serialization

## Overview

TON Ledger App requires custom format for transactions since it have to be assembled in ledger for security.

## Amount units

TON uses 256 bit integers for amounts, but the most common encoding used in TON is `VarUInteger 16`, which only allows up to 15 byte (120 bit) integers. This app encodes amounts using 1 byte to indicate the number of bytes in the integer, and then that amount of bytes for the integer itself. We shall name this format `varuint`. In the `Cell` representations the normal `VarUInteger 16` is used (4 bits for the number of bytes, then that amount of bytes for the integer).

## Wallet Contract

TON Ledger App works only with Wallet V4 (R2) contract and doesn't support legacy wallets. This also meants that it is impossible to work with older wallets.

## Address format

Address is serialized by appending a single byte of chain - 0x00 or 0xff and then 32 bytes of address hash part (that is not just the hash of an address - in TON, the hash part of an address if the hash of the initial state of the contract), 33 bytes in total. We shall name this format `address`.

## Cells

`Cell` is a fundamental data type used in TON, and everything is stored using it. A Cell can store up to 1023 bits, and up to 4 references to other cells (which themselves can store 4 more references each and so on). When hashing a Cell for signing, in order to reference another cell, only two of its properties need to be known - its depth and its hash. We define a format to serialize cell references in requests: 2 bytes for the max depth, and 32 bytes for the hash, 34 in total. We shall name this format `cell_ref`.

## Transaction request structure

| Field | Size (bytes) or type | Description |
| --- | :---: | --- |
| `tag` | 1 | 0x00. Reserved for future. |
| `seqno` | 4 | A sequence number used to prevent message replay |
| `timeout` | 4 | Message timeout |
| `value` | `varuint` | The amount in nanotons to send to the destination address encoded as described above |
| `bounce` | 1 | 0x01 or 0x00 for bounce flag |
| `send_mode` | 1 | Send mode of the message |
| `has_state_init` | 1 | 0x01 if state init is present |
| `state_init` | 0 or `cell_ref` | The state init cell reference if `has_state_init` is 0x01 |
| `has_payload` | 1 | 0x01 if payload is present |
| `payload` | 0 or `cell_ref` | The payload cell reference if `has_payload` is 0x01 |
| `has_hints` | 1 | 0x01 if hints exists |
| `hints_id` | 0 or 4 | Hints ID (see [MESSAGES.md](./MESSAGES.md) to learn about hints IDs) |
| `hints_len` | 0 or 2 | Hints data length |
| `hints_data` | 0 or `hints_len` | Hints data |

See [MESSAGES.md](./MESSAGES.md) to learn how hints are encoded.

Note that `payload` may be passed without `hints`, but `hints` cannot be passed without `payload`. See the table below to find out about transaction types depending on whether `payload` and `hints` are present.

| `has_payload` | `has_hints` | Transaction type |
| --- | --- | --- |
| &cross; | &cross; | A simple transfer without any payload |
| &check; | &cross; | An invalid request, an error will be thrown |
| &check; | &check; | A transaction of some known types, see [MESSAGES.md](./MESSAGES.md) to learn about known types. Ledger will display all important fields |
| &cross; | &check; | An invalid request, an error will be thrown |

### Fee

You won't find any fee in the transaction structure because the TON chain has constant fees.

### Transaction Cell encoding

In order to send a transaction, it must be encoded into a Cell, the hash of this Cell must then be signed and prepended to the first Cell. This is the signed transaction which can then be wrapped as an external message and broadcasted into the TON Blockchain for execution.

Here's the TL-B schema of the transaction order recognized by the Wallet V4R2 contract used by this app (you can read about TL-B [here](https://docs.ton.org/develop/data-formats/tl-b-language)):
```
// this is a type from TON itself listed here for more clarity
// however, listing all of the types in the hierarchy down to the simplest things like Bit and uint* would take too much space and isn't the goal here, so instead you can consult https://github.com/ton-blockchain/ton/blob/master/crypto/block/block.tlb for full reference
message$_ {X:Type} info:CommonMsgInfoRelaxed
  init:(Maybe (Either StateInit ^StateInit))
  body:(Either X ^X) = MessageRelaxed X;
// CommonMsgInfoRelaxed is always of the internal type since we're always sending internal messages from wallets
// both StateInit (when it's present) and the body are always sent in references (so the respective Either bit is 1)

message_order#_ {X:Type} send_mode:uint8 message:^(MessageRelaxed X) = MessageOrder X;

// in practice, wallet v4r2 can send more than 1 message (up to 4 in fact, since a Cell can have up to 4 references), which would mean that there is more than one MessageOrder possible per TransactionOrder, but the TL-B is deliberately simplified here for easier understanding, and because the app only stores one MessageOrder
transaction_order#_ {X:Type} signature:(512 * Bit) subwallet_id:uint32 valid_until:uint32 msg_seqno:uint32 op:uint8 order:(MessageOrder X) = TransactionOrder X;
// signature is calculated from the hash of the Cell that would be constructed from the rest of the TransactionOrder data
// subwallet_id is a parameter of the wallet and just has to match what is stored inside the wallet (it is immutable)
// valid_until is the timestamp at which the transaction order becomes invalid and will not be accepted by the wallet
// msg_seqno is a variable stored inside the wallet and is used for replay protection, it is increased by 1 on each successful transaction
// op has to be set to 0 to send messages, other ops are reserved for other actions on the wallet that this app is not concerned about
```

Cell hash only depends on its data bits and references, and so the companion wallet and the app independently build the transaction Cell from the transaction request, and the transaction Cell hash is then signed by the app, the resulting hash and signature is then passed to the companion wallet (which has its own copy of the unsigned transaction Cell built independently) for broadcasting.
