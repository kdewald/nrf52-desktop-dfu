# NRF52 Desktop DFU
A cross-platform library to perform device firmware upgrades (DFU) for Nordic's NRF52 devices. The API is straightforward and agnostic to the BLE implementation, giving developers a quick way to carry out a DFU from a desktop environment.

## Code Structure
* `src-dfu`
    * An API that is agnostic to the BLE implementation (`NrfDfuServer`)
* `src-dfu-app`
    * A small console application that uses the compiled `src-dfu` library.

## Build Instructions
We have specific scripts to compile the library on each platform. All binaries will be placed in the `bin` folder.

### Windows
* Install Visual Studio 2019, selecting the C++ toolchain.
* Install CMake from https://cmake.org/
* Run `.\toolchains\windows\windows_compile.bat` from Powershell.
  * Use `-c` or `-clean` to perform a full rebuild of the project.
  * Use `-a=<ARCH>` or `-arch=<ARCH>` to build the libraries for a specific architecture. (Supported values are `x86` and `x64`.)

### Linux
* Run `./toolchains/linux/linux_install.sh` to install the OS dependencies. This should be required only once.
* Run `./toolchains/linux/linux_compile.sh` to build the project.
  * Use `-c` or `-clean` to perform a full rebuild of the project.

### macOS
* Install Homebrew from https://brew.sh/
* Run `brew install cmake` to install CMake.
* Run `./toolchains/macos/macos_compile.sh` to build the project.
  * Use `-c` or `-clean` to perform a full rebuild of the project.
The library will be compiled into a `.dylib` and saved into `bin/darwin`.

## Usage of the DFU application.

To perform a DFU with the bundled application that we developed, the following information and files are needed: 
* The MAC address of the device (when booted in DFU mode) to be upgraded.
* Zip file containing everything for the upgrade:
  * Data File (.dat): This file contains the image type, the image cryptographic hash, etc.
  * Binary File (.bin): Application to be uploaded to the device.
  * JSON Manifest (.json): Specifies which file is which for this to work.

Usage: `dfu_app.exe <mac_address> <dfu_zip_path>`
* <mac_address>: Device MAC address in a format compatible with the BLE library. (See note for how macOS handles MAC addresses)
* <dfu_zip_file_path>: Path to the DFU zip package

#### Windows Example
* Run `.\bin\windows-x64\dfu_app.exe EE4200000000 package.zip` 

#### Linux Example
* Run `./bin/linux/dfu_app EE:42:00:00:00:00 package.zip`
  
#### MacOS Example
* Run `./bin/darwin/dfu_app {UUID} package.zip`

## Important Notes

### Functionality
* This library is focused on providing upgrade functionality when using Nordic's Secure DFU Bootloader. Some part of the internal logic is hard-coded around this, so it's possible it won't work for a passwordless DFU. We might add this functionality in the future!
* Current upgrades are not resumable. If something fails during the process, the library might fail or crash, requiring a complete restart of the process.

### macOS - MAC Addresses and UUIDs
In an effort to protect privacy, CoreBluetooth (the underlying macOS Bluetooth API) does not expose the MAC address of a device to a user. Instead, it randomizes the MAC address to a UUID (Universal Unique Identifier) that is exposed to the user. Instead, you will need to scan for devices and find the UUID of the desired device to connect to.

## License
All components within this project that have not been bundled from external creators, are licensed under the terms of the [MIT Licence](LICENCE.md).