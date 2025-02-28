from dataclasses import dataclass
from enum import IntFlag, IntEnum
from typing import Optional, Union
from abc import ABC, abstractmethod

from tonsdk.utils import Address
from tonsdk.boc import Cell

from .ton_utils import write_varuint, write_address, write_cell
from .my_builder import begin_cell


class SendMode(IntFlag):
    CARRRY_ALL_REMAINING_BALANCE = 128
    CARRRY_ALL_REMAINING_INCOMING_VALUE = 64
    DESTROY_ACCOUNT_IF_ZERO = 32
    IGNORE_ERRORS = 2
    PAY_GAS_SEPARATLY = 1
    NONE = 0


@dataclass
class StateInit:
    code: Cell
    data: Cell

    def to_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0, 2)
            .store_maybe_ref(self.code)
            .store_maybe_ref(self.data)
            .store_uint(0, 1)
            .end_cell()
        )


class Payload(ABC):
    @abstractmethod
    def to_request_bytes(self) -> Optional[bytes]:
        return bytes()

    @abstractmethod
    def to_message_body_cell(self) -> Cell:
        return Cell()


def is_string_ascii_printable(s: str) -> bool:
    for c in bytes(s, encoding="utf8"):
        if c < 0x20:
            return False
        if c >= 0x7f:
            return False
    return True


class PayloadID(IntEnum):
    COMMENT = 0
    JETTON_TRANSFER = 1
    NFT_TRANSFER = 2
    JETTON_BURN = 3
    ADD_WHITELIST = 4
    SINGLE_NOMINATOR_WITHDRAW = 5
    SINGLE_NOMINATOR_CHANGE_VALIDATOR = 6
    TONSTAKERS_DEPOSIT = 7
    JETTON_DAO_VOTE = 8
    CHANGE_DNS_RECORD = 9
    TOKEN_BRIDGE_PAY_SWAP = 10


class CommentPayload(Payload):
    def __init__(self, comment: str) -> None:
        if not is_string_ascii_printable(comment) or len(comment) > 120:
            raise ValueError("Comment string must be a printable ASCII string"
                             "and must be 120 chars or less")
        self.comment: str = comment

    def to_request_bytes(self) -> bytes:
        return b"".join([
            (PayloadID.COMMENT).to_bytes(4, byteorder="big"),
            len(self.comment).to_bytes(2, byteorder="big"),
            bytes(self.comment, "utf8")
        ])

    def to_message_body_cell(self) -> Cell:
        return begin_cell().store_uint(0, 32).store_bytes(bytes(self.comment, "utf8")).end_cell()


# pylint: disable-next=too-many-instance-attributes
class JettonTransferPayload(Payload):
    def __init__(self,
                 amount: int,
                 to: Address,
                 response_destination: Optional[Address] = None,
                 query_id: Optional[int] = None,
                 custom_payload: Optional[Cell] = None,
                 forward_amount: int = 0,
                 forward_payload: Optional[Cell] = None,
                 jetton_id: Optional[int] = None,
                 owner_workchain: int = 0) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.amount: int = amount
        self.destination: Address = to
        self.response_destionation: Address = (
            response_destination if response_destination is not None else to
        )
        self.custom_payload: Optional[Cell] = custom_payload
        self.forward_amount: int = forward_amount
        self.forward_payload: Optional[Cell] = forward_payload
        self.jetton_id: Optional[int] = jetton_id
        self.owner_workchain = owner_workchain

    def to_request_bytes(self) -> bytes:
        flags = 0
        if self.query_id != 0:
            flags |= 1
        if self.jetton_id is not None:
            flags |= 2

        main_body = b"".join([
            bytes([flags]),
            self.query_id.to_bytes(8, byteorder="big") if self.query_id != 0 else bytes([]),
            (b"".join([
                self.jetton_id.to_bytes(2, byteorder="big"),
                bytes([self.owner_workchain]),
            ])) if self.jetton_id is not None else bytes([]),
            write_varuint(self.amount),
            write_address(self.destination),
            write_address(self.response_destionation),
            (b"".join([
                bytes([1]),
                write_cell(self.custom_payload)
            ]) if self.custom_payload is not None else bytes([0])),
            write_varuint(self.forward_amount),
            (b"".join([
                bytes([1]),
                write_cell(self.forward_payload)
            ]) if self.forward_payload is not None else bytes([0]))
        ])
        return b"".join([
            (PayloadID.JETTON_TRANSFER).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x0f8a7ea5, 32)
            .store_uint(self.query_id, 64)
            .store_coins(self.amount)
            .store_address(self.destination)
            .store_address(self.response_destionation)
            .store_maybe_ref(self.custom_payload)
            .store_coins(self.forward_amount)
            .store_maybe_ref(self.forward_payload)
            .end_cell()
        )


class CustomUnsafePayload(Payload):
    def __init__(self, cell: Cell) -> None:
        self.cell: Cell = cell

    def to_request_bytes(self) -> Optional[bytes]:
        return None

    def to_message_body_cell(self) -> Cell:
        return self.cell


class NFTTransferPayload(Payload):
    def __init__(self,
                 to: Address,
                 response_destination: Optional[Address] = None,
                 query_id: Optional[int] = None,
                 custom_payload: Optional[Cell] = None,
                 forward_amount: int = 0,
                 forward_payload: Optional[Cell] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.new_owner: Address = to
        self.response_destionation: Address = (
            response_destination if response_destination is not None else to
        )
        self.custom_payload: Optional[Cell] = custom_payload
        self.forward_amount: int = forward_amount
        self.forward_payload: Optional[Cell] = forward_payload

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_address(self.new_owner),
            write_address(self.response_destionation),
            (b"".join([
                bytes([1]),
                write_cell(self.custom_payload)
            ]) if self.custom_payload is not None else bytes([0])),
            write_varuint(self.forward_amount),
            (b"".join([
                bytes([1]),
                write_cell(self.forward_payload)
            ]) if self.forward_payload is not None else bytes([0]))
        ])
        return b"".join([
            (PayloadID.NFT_TRANSFER).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x5fcc3d14, 32)
            .store_uint(self.query_id, 64)
            .store_address(self.new_owner)
            .store_address(self.response_destionation)
            .store_maybe_ref(self.custom_payload)
            .store_coins(self.forward_amount)
            .store_maybe_ref(self.forward_payload)
            .end_cell()
        )


class JettonBurnPayload(Payload):
    def __init__(self,
                 amount: int,
                 response_destination: Address,
                 query_id: Optional[int] = None,
                 custom_payload: Optional[Union[Cell, bytes]] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.amount: int = amount
        self.response_destionation: Address = response_destination
        self.custom_payload: Optional[Union[Cell, bytes]] = custom_payload

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_varuint(self.amount),
            write_address(self.response_destionation),
            ((b"".join([
                bytes([2]),
                bytes([len(self.custom_payload)]),
                self.custom_payload
            ]) if isinstance(self.custom_payload, bytes) else b"".join([
                bytes([1]),
                write_cell(self.custom_payload)
            ])) if self.custom_payload is not None else bytes([0]))
        ])
        return b"".join([
            (PayloadID.JETTON_BURN).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        cp = self.custom_payload
        if isinstance(self.custom_payload, bytes):
            cp = begin_cell().store_bytes(self.custom_payload).end_cell()

        return (
            begin_cell()
            .store_uint(0x595f07bc, 32)
            .store_uint(self.query_id, 64)
            .store_coins(self.amount)
            .store_address(self.response_destionation)
            .store_maybe_ref(cp)
            .end_cell()
        )


class AddWhitelistPayload(Payload):
    def __init__(self,
                 address: Address,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.address: Address = address

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_address(self.address)
        ])
        return b"".join([
            (PayloadID.ADD_WHITELIST).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x7258a69b, 32)
            .store_uint(self.query_id, 64)
            .store_address(self.address)
            .end_cell()
        )


class SingleNominatorWithdrawPayload(Payload):
    def __init__(self,
                 amount: int,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.amount: int = amount

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_varuint(self.amount)
        ])
        return b"".join([
            (PayloadID.SINGLE_NOMINATOR_WITHDRAW).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x1000, 32)
            .store_uint(self.query_id, 64)
            .store_coins(self.amount)
            .end_cell()
        )


class ChangeValidatorPayload(Payload):
    def __init__(self,
                 address: Address,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.address: Address = address

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_address(self.address)
        ])
        return b"".join([
            (PayloadID.SINGLE_NOMINATOR_CHANGE_VALIDATOR).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x1001, 32)
            .store_uint(self.query_id, 64)
            .store_address(self.address)
            .end_cell()
        )


class TonstakersDepositPayload(Payload):
    def __init__(self,
                 app_id: Optional[int] = None,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.app_id: Optional[int] = app_id

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            (b"".join([
                bytes([1]),
                self.app_id.to_bytes(8, byteorder="big")
            ]) if self.app_id is not None else bytes([0]))
        ])
        return b"".join([
            (PayloadID.TONSTAKERS_DEPOSIT).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        b = (
            begin_cell()
            .store_uint(0x47d54391, 32)
            .store_uint(self.query_id, 64)
        )

        if self.app_id is not None:
            b = b.store_uint(self.app_id, 64)

        return b.end_cell()


class JettonDAOVotePayload(Payload):
    def __init__(self,
                 voting_address: Address,
                 expiration_date: int,
                 vote: bool,
                 need_confirmation: bool,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.voting_address: Address = voting_address
        self.expiration_date: int = expiration_date
        self.vote: bool = vote
        self.need_confirmation: bool = need_confirmation

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            write_address(self.voting_address),
            self.expiration_date.to_bytes(6, byteorder="big"),
            bytes([1 if self.vote else 0]),
            bytes([1 if self.need_confirmation else 0])
        ])
        return b"".join([
            (PayloadID.JETTON_DAO_VOTE).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(0x69fb306c, 32)
            .store_uint(self.query_id, 64)
            .store_address(self.voting_address)
            .store_uint(self.expiration_date, 48)
            .store_bit(self.vote)
            .store_bit(self.need_confirmation)
            .end_cell()
        )


class ChangeDNSWalletPayload(Payload):
    def __init__(self,
                 wallet: Optional[Address],
                 has_capabilities: bool,
                 is_wallet: bool,
                 query_id: Optional[int] = None) -> None:
        if is_wallet and not has_capabilities:
            raise ValueError("DNS wallet record cannot be a wallet without capabilities")
        self.query_id: int = query_id if query_id is not None else 0
        self.wallet: Optional[Address] = wallet
        self.has_capabilities: bool = has_capabilities
        self.is_wallet: bool = is_wallet

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            bytes([1 if self.wallet is not None else 0]),
            bytes([0]),
            (b"".join([
                write_address(self.wallet),
                bytes([1 if self.has_capabilities else 0]),
                bytes([1 if self.is_wallet else 0]) if self.has_capabilities else bytes([])
            ]) if self.wallet is not None else bytes([]))
        ])
        return b"".join([
            (PayloadID.CHANGE_DNS_RECORD).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        b = (
            begin_cell()
            .store_uint(0x4eb1f0f9, 32)
            .store_uint(self.query_id, 64)
            .store_bytes(bytes([0xe8, 0xd4, 0x40, 0x50, 0x87, 0x3d, 0xba, 0x86, 0x5a, 0xa7, 0xc1,
                                0x70, 0xab, 0x4c, 0xce, 0x64, 0xd9, 0x08, 0x39, 0xa3, 0x4d, 0xcf,
                                0xd6, 0xcf, 0x71, 0xd1, 0x4e, 0x02, 0x05, 0x44, 0x3b, 0x1b]))
        )

        if self.wallet is not None:
            rb = (
                begin_cell()
                .store_uint(0x9fd3, 16)
                .store_address(self.wallet)
                .store_uint(1 if self.has_capabilities else 0, 8)
            )

            if self.has_capabilities:
                if self.is_wallet:
                    rb = (
                        rb.store_uint(1, 1)
                        .store_uint(0x2177, 16)
                    )

                rb = rb.store_uint(0, 1)

            b = b.store_ref(rb.end_cell())

        return b.end_cell()


class ChangeDNSPayload(Payload):
    def __init__(self,
                 key: bytes,
                 value: Optional[Cell],
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.key: bytes = key
        self.value: Optional[Cell] = value

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            bytes([1 if self.value is not None else 0]),
            bytes([1]),
            self.key,
            write_cell(self.value) if self.value is not None else bytes([])
        ])
        return b"".join([
            (PayloadID.CHANGE_DNS_RECORD).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        b = (
            begin_cell()
            .store_uint(0x4eb1f0f9, 32)
            .store_uint(self.query_id, 64)
            .store_bytes(self.key)
        )

        if self.value is not None:
            b = b.store_ref(self.value)

        return b.end_cell()


class TokenBridgePaySwapPayload(Payload):
    def __init__(self,
                 swap_id: bytes,
                 query_id: Optional[int] = None) -> None:
        self.query_id: int = query_id if query_id is not None else 0
        self.swap_id: bytes = swap_id

    def to_request_bytes(self) -> bytes:
        main_body = b"".join([
            (b"".join([
                bytes([1]),
                self.query_id.to_bytes(8, byteorder="big")
            ]) if self.query_id != 0 else bytes([0])),
            self.swap_id
        ])
        return b"".join([
            (PayloadID.TOKEN_BRIDGE_PAY_SWAP).to_bytes(4, byteorder="big"),
            len(main_body).to_bytes(2, byteorder="big"),
            main_body
        ])

    def to_message_body_cell(self) -> Cell:
        return (
            begin_cell()
            .store_uint(8, 32)
            .store_uint(self.query_id, 64)
            .store_bytes(self.swap_id)
            .end_cell()
        )


# pylint: disable-next=too-many-instance-attributes
class Transaction:
    def __init__(self,
                 to: Address,
                 send_mode: SendMode,
                 seqno: int,
                 timeout: int,
                 bounce: bool,
                 amount: int,
                 state_init: Optional[StateInit] = None,
                 payload: Optional[Payload] = None,
                 subwallet_id: Optional[int] = None,
                 include_wallet_op: bool = True) -> None:
        self.to: Address = to
        self.send_mode: SendMode = send_mode
        self.seqno: int = seqno
        self.timeout: int = timeout
        self.bounce: bool = bounce
        self.amount: int = amount
        self.state_init: Optional[StateInit] = state_init
        self.payload: Optional[Payload] = payload
        self.subwallet_id: Optional[int] = subwallet_id
        self.include_wallet_op: bool = include_wallet_op

    def header_bytes(self) -> bytes:
        if not self.include_wallet_op or self.subwallet_id is not None:
            return b"".join([
                bytes([1]),
                (
                    (self.subwallet_id if self.subwallet_id is not None else 698983191)
                    .to_bytes(4, byteorder="big")
                ),
                bytes([1 if self.include_wallet_op else 0])
            ])

        return bytes([0])

    def to_request_bytes(self) -> bytes:
        return b"".join([
            self.header_bytes(),
            self.seqno.to_bytes(4, byteorder="big"),
            self.timeout.to_bytes(4, byteorder="big"),
            write_varuint(self.amount),
            write_address(self.to),
            bytes([1 if self.bounce else 0]),
            bytes([self.send_mode]),
            self.state_init_part_bytes(),
            self.payload_part_bytes()
        ])

    def state_init_part_bytes(self) -> bytes:
        if self.state_init is None:
            return bytes([0])

        si_cell = self.state_init.to_cell()
        return b"".join([
            bytes([1]),
            write_cell(si_cell)
        ])

    def payload_part_bytes(self) -> bytes:
        if self.payload is None:
            return bytes([0, 0])

        payload_bytes = self.payload.to_request_bytes()
        payload_cell = self.payload.to_message_body_cell()
        return b"".join([
            bytes([1]),
            write_cell(payload_cell),
            (b"".join([
                bytes([1]),
                payload_bytes
            ]) if payload_bytes is not None else bytes([0]))
        ])

    def order_cell(self) -> Cell:
        b = (
            begin_cell()
            .store_uint(1, 2)
            .store_bit(self.bounce)
            .store_uint(0, 3)
            .store_address(self.to)
            .store_coins(self.amount)
            .store_uint(0, 1 + 4 + 4 + 64 + 32)
        )
        if self.state_init is None:
            b = b.store_bit(0)
        else:
            b = b.store_uint(3, 2).store_ref(self.state_init.to_cell())
        b = b.store_maybe_ref(None if self.payload is None else self.payload.to_message_body_cell())
        return b.end_cell()

    def transfer_cell(self) -> Cell:
        b = (
            begin_cell()
            .store_uint(698983191 if self.subwallet_id is None else self.subwallet_id, 32)
            .store_uint(self.timeout, 32)
            .store_uint(self.seqno, 32)
        )

        if self.include_wallet_op:
            b = b.store_uint(0, 8)

        return (
            b
            .store_uint(self.send_mode, 8)
            .store_ref(self.order_cell())
            .end_cell()
        )
