from application_client.ton_command_sender import BoilerplateCommandSender
from application_client.ton_response_unpacker import unpack_get_version_response

MAJOR = 2
MINOR = 0
PATCH = 0

# In this test we check the behavior of the device when asked to provide the app version
def test_version(backend):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # Send the GET_VERSION instruction
    rapdu = client.get_version()
    # Use an helper to parse the response, assert the values
    assert unpack_get_version_response(rapdu.data) == (MAJOR, MINOR, PATCH)