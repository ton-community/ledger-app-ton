from enum import IntEnum, IntFlag
from typing import Generator, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.bip import pack_derivation_path

from .ton_utils import split_message


MAX_APDU_LEN: int = 255

CLA: int = 0xE0

class P1(IntEnum):
    P1_NONE = 0x00

    P1_CONFIRM = 0x01

    P1_NON_CONFIRM = 0x00

class P2(IntFlag):
    P2_NONE = 0x00

    P2_FIRST = 0x01

    P2_MORE = 0x02

class InsType(IntEnum):
    GET_VERSION       = 0x03
    GET_APP_NAME      = 0x04
    GET_PUBLIC_KEY    = 0x05
    SIGN_TX           = 0x06
    GET_ADDRESS_PROOF = 0x08
    SIGN_DATA         = 0x09
    GET_APP_SETTINGS  = 0x0A

class Errors(IntEnum):
    SW_DENY                    = 0x6985
    SW_WRONG_P1P2              = 0x6A86
    SW_WRONG_DATA_LENGTH       = 0x6A87
    SW_INS_NOT_SUPPORTED       = 0x6D00
    SW_CLA_NOT_SUPPORTED       = 0x6E00
    SW_WRONG_RESPONSE_LENGTH   = 0xB000
    SW_DISPLAY_ADDRESS_FAIL    = 0xB002
    SW_DISPLAY_AMOUNT_FAIL     = 0xB003
    SW_WRONG_TX_LENGTH         = 0xB004
    SW_TX_PARSING_FAIL         = 0xB010
    SW_WRONG_SIGN_DATA_LENGTH  = 0xB005
    SW_SIGN_DATA_PARSING_FAIL  = 0xB011
    SW_BAD_STATE               = 0xB007
    SW_SIGNATURE_FAIL          = 0xB008
    SW_REQUEST_TOO_LONG        = 0xB00B
    SW_BAD_BIP32_PATH          = 0XB0BD
    SW_BLIND_SIGNING_DISABLED  = 0xBD00

class AddressDisplayFlags(IntFlag):
    NONE = 0
    TESTNET = 1
    MASTERCHAIN = 2


class BoilerplateCommandSender:
    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend


    def get_app_and_version(self) -> RAPDU:
        return self.backend.exchange(cla=0xB0,  # specific CLA for BOLOS
                                     ins=0x01,  # specific INS for get_app_and_version
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_version(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_VERSION,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_app_name(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_APP_NAME,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_public_key(self, path: str) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_PUBLIC_KEY,
                                     p1=P1.P1_NON_CONFIRM,
                                     p2=AddressDisplayFlags.NONE,
                                     data=pack_derivation_path(path))


    def get_app_settings(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_APP_SETTINGS,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    @contextmanager
    def get_public_key_with_confirmation(self,
                                         path: str,
                                         display_flags: AddressDisplayFlags,
                                         is_v3r2: bool = False,
                                         subwallet_id: int = 698983191,
                                        ) -> Generator[None, None, None]:
        specifiers = bytes([])
        if is_v3r2 or subwallet_id != 698983191:
            display_flags |= 4
            specifiers = b"".join([
                bytes([1 if is_v3r2 else 0]),
                subwallet_id.to_bytes(4, byteorder="big"),
            ])
        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.GET_PUBLIC_KEY,
                                         p1=P1.P1_CONFIRM,
                                         p2=display_flags,
                                         data=b"".join([
                                             pack_derivation_path(path),
                                             specifiers,
                                         ])) as response:
            yield response

    @contextmanager
    def get_address_proof(self,
                          path: str,
                          display_flags: AddressDisplayFlags,
                          domain: str,
                          timestamp: int,
                          payload: bytes,
                          is_v3r2: bool = False,
                          subwallet_id: int = 698983191) -> Generator[None, None, None]:
        domain_b = bytes(domain, "utf8")
        specifiers = bytes([])
        if is_v3r2 or subwallet_id != 698983191:
            display_flags |= 4
            specifiers = b"".join([
                bytes([1 if is_v3r2 else 0]),
                subwallet_id.to_bytes(4, byteorder="big"),
            ])
        req_bytes = b"".join([
            pack_derivation_path(path),
            specifiers,
            bytes([len(domain_b)]),
            domain_b,
            timestamp.to_bytes(8, byteorder="big"),
            payload
        ])
        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.GET_ADDRESS_PROOF,
                                         p1=P1.P1_CONFIRM,
                                         p2=display_flags,
                                         data=req_bytes) as response:
            yield response

    @contextmanager
    def sign_tx(self, path: str, transaction: bytes) -> Generator[None, None, None]:
        self.backend.exchange(cla=CLA,
                              ins=InsType.SIGN_TX,
                              p1=P1.P1_NONE,
                              p2=(P2.P2_FIRST | P2.P2_MORE),
                              data=pack_derivation_path(path))
        messages = split_message(transaction, MAX_APDU_LEN)

        for msg in messages[:-1]:
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_TX,
                                  p1=P1.P1_NONE,
                                  p2=P2.P2_MORE,
                                  data=msg)

        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.SIGN_TX,
                                         p1=P1.P1_NONE,
                                         p2=P2.P2_NONE,
                                         data=messages[-1]) as response:
            yield response

    @contextmanager
    def sign_data(self, path: str, data: bytes) -> Generator[None, None, None]:
        self.backend.exchange(cla=CLA,
                              ins=InsType.SIGN_DATA,
                              p1=P1.P1_NONE,
                              p2=(P2.P2_FIRST | P2.P2_MORE),
                              data=pack_derivation_path(path))
        messages = split_message(data, MAX_APDU_LEN)

        for msg in messages[:-1]:
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_DATA,
                                  p1=P1.P1_NONE,
                                  p2=P2.P2_MORE,
                                  data=msg)

        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.SIGN_DATA,
                                         p1=P1.P1_NONE,
                                         p2=P2.P2_NONE,
                                         data=messages[-1]) as response:
            yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
