import pytest

from application_client.ton_command_sender import BoilerplateCommandSender, Errors
from application_client.ton_response_unpacker import unpack_sign_data_response
from application_client.ton_sign_data import PlaintextSignDataRequest, SignDataRequest, AppDataSignDataRequest
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import ROOT_SCREENSHOT_PATH, check_signature_validity
from typing import List
from tonsdk.boc import Cell
from tonsdk.utils import Address


def test_sign_data(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    requests: List[SignDataRequest] = [
        PlaintextSignDataRequest("a" * 188),
        AppDataSignDataRequest(Cell(), address=Address("0:" + "0" * 64), domain="test.ton", ext=Cell())
    ]

    for (i, request) in enumerate(requests):
        # Send the sign device instruction.
        # As it requires on-screen validation, the function is asynchronous.
        # It will yield the result when the navigation is done
        with client.sign_data(path=path, data=request.to_request_bytes()):
            # Validate the on-screen request by performing the navigation appropriate for this device
            if firmware.device.startswith("nano"):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                            [NavInsID.BOTH_CLICK],
                                                            "Approve",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}")
            else:
                navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_REVIEW_TAP,
                                                            [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                            NavInsID.USE_CASE_STATUS_DISMISS],
                                                            "Hold to sign",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}")

        # The device as yielded the result, parse it and ensure that the signature is correct
        response = client.get_async_response().data
        sig, hash_b = unpack_sign_data_response(response)
        assert hash_b == request.to_cell().bytes_hash()
        assert check_signature_validity(pubkey, sig, request.to_signed_data())


def test_sign_data_refused(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/607'/0'/0'/0'/0'"

    request = PlaintextSignDataRequest("test")
    rb = request.to_request_bytes()

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_data(path=path, data=rb):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)

        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        for i in range(3):
            instructions = [NavInsID.USE_CASE_REVIEW_TAP] * i
            instructions += [NavInsID.USE_CASE_REVIEW_REJECT,
                             NavInsID.USE_CASE_STATUS_DISMISS]
            with pytest.raises(ExceptionRAPDU) as e:
                with client.sign_data(path=path, data=rb):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0