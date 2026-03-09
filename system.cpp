#include "system.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#include <sstream>
#include <iomanip>

#define WORKING_BUFFER_SIZE 15000

std::string getMacAddress() {
    ULONG outBufLen = WORKING_BUFFER_SIZE;
    PIP_ADAPTER_ADDRESSES pAddresses = static_cast<PIP_ADAPTER_ADDRESSES>(malloc(outBufLen));

    if (!pAddresses) {
        return "";
    }

    ULONG result = GetAdaptersAddresses(
        AF_UNSPEC,
        GAA_FLAG_INCLUDE_PREFIX,
        nullptr,
        pAddresses,
        &outBufLen
    );

    if (result == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = static_cast<PIP_ADAPTER_ADDRESSES>(malloc(outBufLen));
        if (!pAddresses) return "";

        result = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_INCLUDE_PREFIX,
            nullptr,
            pAddresses,
            &outBufLen
        );
    }

    std::string mac;
    if (result == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurr = pAddresses;

        while (pCurr) {
            bool hasValidMac = (pCurr->PhysicalAddressLength >= 6);
            if (hasValidMac && pCurr->IfType != IF_TYPE_SOFTWARE_LOOPBACK) {
                bool allZeros = true;
                for (ULONG i = 0; i < pCurr->PhysicalAddressLength; ++i) {
                    if (pCurr->PhysicalAddress[i] != 0) {
                        allZeros = false;
                        break;
                    }
                }
                if (!allZeros) {
                    std::ostringstream oss;
                    for (ULONG i = 0; i < pCurr->PhysicalAddressLength; ++i) {
                        if (i > 0) oss << "-";
                        oss << std::hex << std::setfill('0') << std::setw(2)
                            << static_cast<int>(pCurr->PhysicalAddress[i]);
                    }
                    mac = oss.str();
                    break;
                }
            }
            pCurr = pCurr->Next;
        }
    }

    free(pAddresses);
    return mac;
}
