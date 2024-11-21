# ****************************************************************************
#    TON Ledger App
#    (c) 2022 Whales Corp.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ****************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif

include $(BOLOS_SDK)/Makefile.defines

ifneq ($(TARGET_NAME),TARGET_NANOS)
    DEFINES += HAVE_HARDCODED_JETTONS
endif

APPNAME      = "TON"
APPVERSION_M = 2
APPVERSION_N = 4
APPVERSION_P = 1
APPVERSION   = "$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)"

APP_SOURCE_PATH += src

ICON_NANOX = icons/nanox_app_ton.gif
ICON_NANOSP = icons/nanox_app_ton.gif
ICON_NANOS = icons/nanos_app_ton.gif
ICON_STAX = icons/ledger_stax_ton_32.gif
ICON_FLEX = icons/ledger_flex_ton_40.gif

CURVE_APP_LOAD_PARAMS = ed25519

PATH_APP_LOAD_PARAMS = "44'/607'"

VARIANT_PARAM = COIN
VARIANT_VALUES = TON

# DEBUG = 1

# HAVE_APPLICATION_FLAG_DERIVE_MASTER = 1
# HAVE_APPLICATION_FLAG_GLOBAL_PIN = 1
# HAVE_APPLICATION_FLAG_BOLOS_SETTINGS = 1
# HAVE_APPLICATION_FLAG_LIBRARY = 1

ENABLE_SWAP = 1
ENABLE_BLUETOOTH = 1
# ENABLE_NFC = 1

ENABLE_NBGL_QRCODE = 1
# ENABLE_NBGL_KEYBOARD = 1
# ENABLE_NBGL_KEYPAD = 1

# DISABLE_STANDARD_APP_FILES = 1
# DISABLE_DEFAULT_IO_SEPROXY_BUFFER_SIZE = 1 # To allow custom size declaration
# DISABLE_STANDARD_APP_DEFINES = 1 # Will set all the following disablers
# DISABLE_STANDARD_SNPRINTF = 1
# DISABLE_STANDARD_USB = 1
# DISABLE_STANDARD_WEBUSB = 1
# DISABLE_STANDARD_BAGL_UX_FLOW = 1
# DISABLE_DEBUG_LEDGER_ASSERT = 1
# DISABLE_DEBUG_THROW = 1

include $(BOLOS_SDK)/Makefile.standard_app

DEFINES += HAVE_BOLOS_APP_STACK_CANARY
