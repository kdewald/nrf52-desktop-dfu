#include "NativeBleController.h"
#include "NrfDfuServer.h"
#include "json/json.hpp"
#include "miniz/miniz.h"
#include "utils.h"

#include <cerrno>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define SCAN_DURATION_MS 2500

static bool get_bin_dat_files(std::string&, std::string&, const char*);

/**
 * main
 *
 * Test bench for DFU.
 * Usage: dfu_tester.exe <ble_address> <dfu_zip_path>
 *      -ble_address: Device BLE address in format compatible with BLE library
 *      -dfu_zip_file_path: Path to the DFU zip package
 *
 * Example usage:
 * .\bin\windows-x64\dfu_tester.exe EE4200000000 ./bin/vxx_y.zip
 * ./bin/linux/dfu_tester EE:42:00:00:00:00 ./bin/vxx_y.zip
 *
 */
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <ble_address> <dfu_zip_path>" << std::endl;
        return -1;
    }

    std::string device_dfu_ble_address(argv[1]);
    char* dfu_zip_filepath = argv[2];

    std::cout << "Starting DFU Test!" << std::endl;
    std::cout << "Initiating scan for " << SCAN_DURATION_MS << " milliseconds..." << std::endl;

    bool device_found = false;
    std::string data_file;
    std::string bin_file;

    if (!get_bin_dat_files(bin_file, data_file, dfu_zip_filepath)) {
        std::cout << "Could not parse DFU zip file!" << std::endl;
        return -1;
    }

    if (!data_file.length() || !bin_file.length()) {
        std::cout << "Empty Files" << std::endl;
        return -1;
    }

    std::cout << "Data file size: " << data_file.length() << std::endl;
    std::cout << "Bin file size: " << bin_file.length() << std::endl;

    if (!validate_mac_address(device_dfu_ble_address)) {
        std::cout << "Invalid MAC address supplied. Address must be at least 4 characters." << std::endl;
        return -1;
    }

    NativeBLE::NativeBleController ble;
    NativeBLE::CallbackHolder callback_holder;
    NativeDFU::NrfDfuServer dfu_server([&](std::string service, std::string characteristic,
                                           std::string data) { ble.write_command(service, characteristic, data); },
                                       [&](std::string service, std::string characteristic, std::string data) {
                                           ble.write_request(service, characteristic, data);
                                       },
                                       data_file, bin_file);

    callback_holder.callback_on_scan_found = [&](NativeBLE::DeviceDescriptor device) {
        if (is_mac_addr_match(device.address, device_dfu_ble_address)) {
            std::cout << "  Found: " << device.name << " (" << device.address << ")" << std::endl;
            device_found = true;
            device_dfu_ble_address = device.address;
        }
    };

    std::cout << "Starting Scan! " << std::endl;
    ble.setup(callback_holder);
    ble.scan_timeout(SCAN_DURATION_MS);

    if (!device_found) {
        std::cerr << "  Device " << device_dfu_ble_address << " could not be found." << std::endl;
        ble.dispose();
        return -1;
    } else {
        ble.connect(device_dfu_ble_address);
        std::cout << "  Connected to " << device_dfu_ble_address << "... initiating streaming..." << std::endl;

        ble.notify(NORDIC_SECURE_DFU_SERVICE, NORDIC_DFU_CONTROL_POINT_CHAR, [&](const uint8_t* data, uint32_t length) {
            std::ostringstream received_data;
            received_data << "Received length " << length << ": 0x";
            for (int i = 0; i < length; i++) {
                received_data << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(data[i]);
                received_data << " ";
            }
            received_data << std::endl;
            std::cout << received_data.str();
            // std::cout << "Calling Notify" << std::endl;
            dfu_server.notify(NORDIC_SECURE_DFU_SERVICE, NORDIC_DFU_CONTROL_POINT_CHAR,
                              std::string(reinterpret_cast<const char*>(data), length));
        });

        dfu_server.run_dfu();
        ble.disconnect();
        ble.dispose();

        if (dfu_server.get_state() == NativeDFU::DFU_FINISHED) {
            std::cout << "DFU Successful" << std::endl;
        } else {
            std::cout << "DFU Not Successful finished with state: 0x" << dfu_server.get_state() << std::endl;
        }
    }
    return 0;
}

// Reads the manifest.json file to retrieve .bin and .dat files from DFU package.
bool get_bin_dat_files(std::string& bin, std::string& dat, const char* dfu_zip_path) {
    mz_zip_archive* zip_archive = new mz_zip_archive;
    char* manifest_file;
    nlohmann::json json_manifest;
    std::string bin_filename;
    size_t bin_size;
    char* bin_contents;
    std::string dat_filename;
    size_t dat_size;
    char* dat_contents;

    mz_zip_zero_struct(zip_archive);
    mz_zip_reader_init_file(zip_archive, dfu_zip_path, 0);

    manifest_file = (char*)mz_zip_reader_extract_file_to_heap(zip_archive, "manifest.json", (size_t*)NULL,
                                                              (mz_uint)NULL);
    if (!manifest_file) {
        return false;
    }

    json_manifest = nlohmann::json::parse(manifest_file);

    bin_filename = json_manifest["manifest"]["application"]["bin_file"];
    dat_filename = json_manifest["manifest"]["application"]["dat_file"];

    dat_contents = (char*)mz_zip_reader_extract_file_to_heap(zip_archive, dat_filename.c_str(), &dat_size,
                                                             (mz_uint)NULL);
    bin_contents = (char*)mz_zip_reader_extract_file_to_heap(zip_archive, bin_filename.c_str(), &bin_size,
                                                             (mz_uint)NULL);
    if (!dat_contents || !bin_contents) {
        return false;
    }

    bin = std::string(bin_contents, bin_size);
    dat = std::string(dat_contents, dat_size);

    zip_archive->m_pFree(zip_archive->m_pAlloc_opaque, manifest_file);
    zip_archive->m_pFree(zip_archive->m_pAlloc_opaque, dat_contents);
    zip_archive->m_pFree(zip_archive->m_pAlloc_opaque, bin_contents);
    delete zip_archive;

    return true;
}