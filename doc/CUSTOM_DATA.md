# List of supported types of custom data

Taken from the [spec](https://github.com/oleganza/TEPs/blob/datasig/text/0000-data-signatures.md#standard-schema-versions).

| Schema CRC | Name | Description |
| --- | --- | --- |
| `0x754bf91b` | Plaintext | Normal text |
| `0x54b58535` | App data | Application-specific data |

# The general serialization format

| Field | Size (bytes) or type | Description |
| --- | :---: | --- |
| `schema_crc` | 4 | Big-endian schema CRC |
| `timestamp` | 8 | Timestamp to be used for signing |
| `request` | `var` | The serialized request data according to the schema (see below) |

## Plaintext (0x754bf91b)

TL-B schema:
```
plaintext text:Text = PayloadCell;

// From TEP-64:
tail#_ {bn:#} b:(bits bn) = SnakeData ~0;
cons#_ {bn:#} {n:#} b:(bits bn) next:^(SnakeData ~n) = SnakeData ~(n + 1);
text#_ {n:#} data:(SnakeData ~n) = Text;
```

Ledger request format:
| Value | Length or type | Description |
| --- | --- | --- |
| `message` | 0-250 | ASCII-only message |

## App data (0x54b58535)

TL-B schema:
```
app_data address:(Maybe MsgAddress) domain:(Maybe ^Text) data:^Cell ext:(Maybe ^Cell) = PayloadCell;

// From TEP-64:
tail#_ {bn:#} b:(bits bn) = SnakeData ~0;
cons#_ {bn:#} {n:#} b:(bits bn) next:^(SnakeData ~n) = SnakeData ~(n + 1);
text#_ {n:#} data:(SnakeData ~n) = Text;
```
(as per the spec, domain is encoded in zero-terminated reversed format, and at least one of address or domain must be present)

Ledger request format:
| Value | Length or type | Description |
| --- | --- | --- |
| `has_address` | 1 | Whether `address` is present |
| `address` | `address` | Smart contract which will receive the signed data |
| `has_domain` | 1 | Whether `domain_len` and `domain` are present |
| `domain_len` | 0 or 1 | `domain` length |
| `domain` | 0 or `domain_len` | ASCII-only app domain (not the zero-terminated reversed format!) |
| `data` | `cell_ref` | The app data |
| `has_ext` | 1 | Whether `ext` is present |
| `ext` | 0 or `cell_ref` | The `ext` cell (reserved for extensions in the standard) |
