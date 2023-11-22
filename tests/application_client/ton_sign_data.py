from abc import ABC, abstractmethod
from time import time
from typing import Optional

from tonsdk.boc import Cell
from tonsdk.utils import Address

from .my_builder import begin_cell
from .ton_utils import write_address, write_cell


class SignDataRequest(ABC):
    def __init__(self, timestamp=int(time())):
        self.timestamp: int = timestamp

    def common_part(self) -> bytes:
        return b"".join([
            self.schema_crc().to_bytes(4, byteorder="big"),
            self.timestamp.to_bytes(8, byteorder="big")
        ])

    @abstractmethod
    def payload_bytes(self) -> bytes:
        return bytes()

    def to_request_bytes(self) -> bytes:
        return b"".join([
            self.common_part(),
            self.payload_bytes()
        ])

    def to_signed_data(self) -> bytes:
        return b"".join([
            self.common_part(),
            self.to_cell().bytes_hash()
        ])

    @abstractmethod
    def to_cell(self) -> Cell:
        return Cell()

    @abstractmethod
    def schema_crc(self) -> int:
        return 0


class PlaintextSignDataRequest(SignDataRequest):
    def __init__(self, text: str, timestamp=int(time())):
        super().__init__(timestamp)
        self.text: str = text

    def payload_bytes(self) -> bytes:
        return bytes(self.text, "utf8")

    def schema_crc(self) -> int:
        return 0x754bf91b

    def to_cell(self) -> Cell:
        bs = bytes(self.text, "utf8")
        return begin_cell().store_string_tail(bs).end_cell()


def encode_domain(d: str) -> Cell:
    parts = reversed(d.split("."))
    b = begin_cell()
    for p in parts:
        b.store_string(p)
        b.store_uint(0, 8)
    return b.end_cell()


class AppDataSignDataRequest(SignDataRequest):
    def __init__(self,
                 data: Cell,
                 address: Optional[Address] = None,
                 domain: Optional[str] = None,
                 ext: Optional[Cell] = None,
                 timestamp=int(time())):
        if address is None and domain is None:
            raise ValueError("address and domain cannot both be None")
        super().__init__(timestamp)
        self.data: Cell = data
        self.address: Optional[Address] = address
        self.domain: Optional[str] = domain
        self.ext: Optional[Cell] = ext

    def schema_crc(self) -> int:
        return 0x54b58535

    def to_cell(self) -> Cell:
        b = begin_cell()
        if self.address is not None:
            b.store_bit(1)
            b.store_address(self.address)
        else:
            b.store_bit(0)
        b.store_maybe_ref(encode_domain(self.domain) if self.domain is not None else None)
        b.store_ref(self.data)
        b.store_maybe_ref(self.ext)
        return b.end_cell()

    def payload_bytes(self) -> bytes:
        db = bytes(self.domain, "utf8")
        return b"".join([
            (b"".join([
                bytes([1]),
                write_address(self.address)
            ]) if self.address is not None else bytes([0])),
            (b"".join([
                bytes([1]),
                bytes([len(db)]),
                db,
            ]) if self.domain is not None else bytes([0])),
            write_cell(self.data),
            (b"".join([
                bytes([1]),
                write_cell(self.ext)
            ]) if self.ext is not None else bytes([0]))
        ])
