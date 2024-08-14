from math import ceil
from hashlib import sha256
from typing import List

from tonsdk.utils import Address
from tonsdk.boc import Cell
from tonsdk.contract.wallet import WalletV4ContractR2, WalletV3ContractR2


TON_PROOF_PREFIX   = b"ton-proof-item-v2/"
TON_CONNECT_PREFIX = b"\xff\xffton-connect"


def write_varuint(n: int) -> bytes:
    bitlen = len(bin(n)) - 2
    bytelen = ceil(bitlen / 8)
    return b"".join([bytes([bytelen]), n.to_bytes(bytelen, byteorder="big")])


def write_address(addr: Address) -> bytes:
    return b"".join([
        bytes([0xff if addr.wc == -1 else 0]),
        bytes(addr.hash_part)
    ])


def write_cell(cell: Cell) -> bytes:
    return b"".join([
        bytes(cell.get_max_depth_as_array()),
        cell.bytes_hash()
    ])


def build_ton_proof_message(workchain: int,
                            pubkey: bytes,
                            domain: str,
                            timestamp: int,
                            payload: bytes) -> bytes:
    # we don't have private_key but the lib is buggy and requires it anyway
    wallet = WalletV4ContractR2(public_key=pubkey, wc=workchain, private_key=bytes())

    addr = wallet.address
    domain_b = bytes(domain, "utf8")
    inner = b"".join([
        TON_PROOF_PREFIX,
        addr.wc.to_bytes(4, byteorder="big"),
        bytes(addr.hash_part),
        len(domain_b).to_bytes(4, byteorder="little"),
        domain_b,
        timestamp.to_bytes(8, byteorder="little"),
        payload
    ])
    inner_hash = sha256(inner).digest()
    return sha256(b"".join([
        TON_CONNECT_PREFIX,
        inner_hash
    ])).digest()


def build_ton_proof_message_v3r2(workchain: int,
                            pubkey: bytes,
                            domain: str,
                            timestamp: int,
                            payload: bytes) -> bytes:
    # we don't have private_key but the lib is buggy and requires it anyway
    wallet = WalletV3ContractR2(public_key=pubkey, wc=workchain, private_key=bytes())

    addr = wallet.address
    domain_b = bytes(domain, "utf8")
    inner = b"".join([
        TON_PROOF_PREFIX,
        addr.wc.to_bytes(4, byteorder="big"),
        bytes(addr.hash_part),
        len(domain_b).to_bytes(4, byteorder="little"),
        domain_b,
        timestamp.to_bytes(8, byteorder="little"),
        payload
    ])
    inner_hash = sha256(inner).digest()
    return sha256(b"".join([
        TON_CONNECT_PREFIX,
        inner_hash
    ])).digest()


def split_message(message: bytes, max_size: int) -> List[bytes]:
    return [message[x:x + max_size] for x in range(0, len(message), max_size)]
