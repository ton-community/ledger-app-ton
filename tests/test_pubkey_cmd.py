import pytest

from application_client.ton_command_sender import BoilerplateCommandSender, Errors, AddressDisplayFlags
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH


# In this test we check that the GET_PUBLIC_KEY works in non-confirmation mode
def test_get_public_key_no_confirm(backend):
    for path in ["m/44'/607'/0'/0'/0'/0'", "m/44'/607'/0'/0'/1'/0'"]:
        client = BoilerplateCommandSender(backend)
        response = client.get_public_key(path=path).data
        assert len(response) == 32


# In this test we check that the GET_PUBLIC_KEY works in confirmation mode
def test_get_public_key_confirm_accepted(firmware, backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"
    with client.get_public_key_with_confirmation(path, AddressDisplayFlags.NONE):
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            instructions = [
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavIns(NavInsID.TOUCH, (65, 520) if firmware.device == "stax" else (80, 440)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
            ]
            navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                           test_name,
                                           instructions)
    response = client.get_async_response().data
    assert len(response) == 32


def test_get_public_key_confirm_accepted_v3r2(firmware, backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"
    with client.get_public_key_with_confirmation(path, AddressDisplayFlags.NONE, is_v3r2=True):
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            instructions = [
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavIns(NavInsID.TOUCH, (65, 520) if firmware.device == "stax" else (80, 440)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
            ]
            navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                           test_name,
                                           instructions)
    response = client.get_async_response().data
    assert len(response) == 32


# # In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
def test_get_public_key_confirm_refused(firmware, backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.get_public_key_with_confirmation(path, AddressDisplayFlags.NONE):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)
        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        instructions_set = [
            [
                NavInsID.USE_CASE_REVIEW_REJECT,
            ],
            [
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
            ]
        ]
        for i, instructions in enumerate(instructions_set):
            with pytest.raises(ExceptionRAPDU) as e:
                with client.get_public_key_with_confirmation(path, AddressDisplayFlags.NONE):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0

def test_get_public_key_bad_path(firmware, backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    paths = ["m/44'/608'/0'/0'/0'/0'", "m/44'/607'"]

    for path in paths:
        with pytest.raises(ExceptionRAPDU) as e:
            with client.get_public_key_with_confirmation(path, AddressDisplayFlags.NONE):
                pass
        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_BAD_BIP32_PATH
        assert len(e.value.data) == 0
