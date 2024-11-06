import re
from pathlib import Path
from application_client.ton_command_sender import BoilerplateCommandSender
from application_client.ton_response_unpacker import unpack_get_app_and_version_response

def check_version(root_path: Path, target_version: str):
    """Extract and check if the version in the Makefile matches the target version."""
    version_re = re.compile(r"^APPVERSION_(?P<part>[MNP])\s*=\s*(?P<val>\d+)", re.I)
    vers_dict = {}
    makefile = f"{root_path.parent.resolve()}/Makefile"

    # Read the file and extract the version
    with open(makefile, "r", encoding="utf-8") as f:
        for line in f:
            match = version_re.match(line)
            if match:
                part = match.group("part")
                vers_dict[part] = int(match.group("val"))

    # Ensure all parts (M, N, P) are present
    try:
        major = vers_dict['M']
        minor = vers_dict['N']
        patch = vers_dict['P']
    except KeyError:
        raise ValueError("The version in the Makefile is incomplete.")

    extracted_version = f"{major}.{minor}.{patch}"
   
    print(f"Makefile version: {extracted_version}, Target version: {target_version}")
    assert extracted_version == target_version

# Test a specific APDU asking BOLOS (and not the app) the name and version of the current app
def test_get_app_and_version(backend, default_screenshot_path):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # Send the special instruction to BOLOS
    response = client.get_app_and_version()
    # Use an helper to parse the response, assert the values
    app_name, version = unpack_get_app_and_version_response(response.data)

    check_version(default_screenshot_path, version)
    assert app_name == "TON"

