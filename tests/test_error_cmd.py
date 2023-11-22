import pytest

from ragger.error import ExceptionRAPDU
from application_client.ton_command_sender import CLA, InsType, P1, P2, Errors


# Ensure the app returns an error when a bad CLA is used
def test_bad_cla(backend):
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA + 1, ins=InsType.GET_VERSION)
    assert e.value.status == Errors.SW_CLA_NOT_SUPPORTED


# Ensure the app returns an error when a bad INS is used
def test_bad_ins(backend):
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA, ins=0xff)
    assert e.value.status == Errors.SW_INS_NOT_SUPPORTED


# Ensure the app returns an error when a bad P1 or P2 is used
def test_wrong_p1p2(backend):
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA, ins=InsType.GET_VERSION, p1=P1.P1_NONE + 1, p2=P2.P2_NONE)
    assert e.value.status == Errors.SW_WRONG_P1P2
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA, ins=InsType.GET_VERSION, p1=P1.P1_NONE, p2=P2.P2_NONE + 1)
    assert e.value.status == Errors.SW_WRONG_P1P2
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA, ins=InsType.GET_APP_NAME, p1=P1.P1_NONE + 1, p2=P2.P2_NONE)
    assert e.value.status == Errors.SW_WRONG_P1P2
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange(cla=CLA, ins=InsType.GET_APP_NAME, p1=P1.P1_NONE, p2=P2.P2_NONE + 1)
    assert e.value.status == Errors.SW_WRONG_P1P2


# Ensure the app returns an error when a bad data length is used
def test_wrong_data_length(backend):
    # APDUs must be at least 5 bytes: CLA, INS, P1, P2, Lc.
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange_raw(b"E0030000")
    assert e.value.status == Errors.SW_WRONG_DATA_LENGTH
    # APDUs advertises a too long length
    with pytest.raises(ExceptionRAPDU) as e:
        backend.exchange_raw(b"E003000005")
    assert e.value.status == Errors.SW_WRONG_DATA_LENGTH


def test_invalid_state(backend):
    with pytest.raises(ExceptionRAPDU) as e:
        r = backend.exchange(cla=CLA,
                         ins=InsType.SIGN_TX,
                         p1=P1.P1_NONE,
                         p2=P2.P2_FIRST,
                         data=b"abcde")
        print(r)
    assert e.value.status == Errors.SW_WRONG_P1P2