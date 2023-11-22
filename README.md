# TON Ledger App

This is a Nano App for Ledger S/X/S+ for TON blockchain.

* 🚀 Compatible with latest Wallet v4 contract
* 🔎 Absolutely minimal footprint and very straight forward code that easy to audit and contribute
* 📱 Compatible with TON wallet apps

Developed by [Whales Corp.](https://whalescorp.com)

## Installation

Right now TON app is not available through Ledger Live and have to be sideloaded to Ledger Nano S+. We support Ledger Nano S, but we generally recommend a new one. Everything have to be done on Linux on x64 CPU with docker installed.
* Checkout this repository
* Connect Ledger to the computer, turn it on and unlock.
* Start and login into environment. Run in terminal: `./env.sh`
* Run `make load` to build and upload app to the Ledger

## Programmatic Usage

There are some experimental (but stable) libraries:
* [ton-ledger-ts](https://github.com/ton-community/ton-ledger-ts)

## Documentation

* [Base Transaction](doc/TRANSACTION.md)
* [Supported TON messages](doc/MESSAGES.md)
* [Low level ADPU](doc/APDU.md)
* [Commands](doc/COMMANDS.md)
* [Supported custom data formats](doc/CUSTOM_DATA.md)

## Development

Use the ledger app builder docker image:
```bash
docker run --rm -ti  -v "$(realpath .):/app" --privileged -v "/dev/bus/usb:/dev/bus/usb" ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder-lite:latest
```

Inside the container:
* `make` - to build app
* `make load` - to build and upload to a Ledger
* `make clean` - to clean build
* `make scan-build` - for Clang static analyzer
* `cmake -Bbuild -H. && make -C build && CTEST_OUTPUT_ON_FAILURE=1 make -C build test` - for unit tests in `unit-tests` directory
