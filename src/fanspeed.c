#include "fanspeed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IOKit/IOKitLib.h>
#include <libkern/OSAtomic.h>

io_connect_t gConn = 0;

int main(int argc, char *argv[])
{
    SMCOpen();
    SMCReadFans();

    IOServiceClose(gConn);
    return kIOReturnSuccess;
}

kern_return_t SMCOpen()
{
    kern_return_t result;
    mach_port_t   masterPort;
    io_iterator_t iterator;
    io_object_t   device;

	IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
    result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
    if (result != kIOReturnSuccess)
    {
        printf("Error: IOServiceGetMatchingServices() %08x\n", result);
        return result;
    }

    device = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (device == 0)
    {
        printf("Error: no SMC found\n");
        return result;
    }

    result = IOServiceOpen(device, mach_task_self(), 0, &gConn);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess)
    {
        printf("Error: IOServiceOpen() %08x\n", result);
        return result;
    }

    return kIOReturnSuccess;
}

kern_return_t SMCReadFans()
{
    kern_return_t returnValue = kIOReturnSuccess;;
    SMCValue_t smcValue;
    char key[5];


    SMCReadKey("FNum", &smcValue);
    int totalFans = strtouint32((char *)smcValue.bytes, smcValue.dataSize, 10);
    printf("# Fans: %d\n", totalFans);

    for (int i = 0; i < totalFans; i++) {
        sprintf(key, "F%dAc", i);
        SMCReadKey(key, &smcValue);
        // Debug
        //printf("Datatype: %s\n", smcValue.dataType);
        printf("Fan %i Current Speed: %f\n", i, GetFloatFromBytes(smcValue));

        sprintf(key, "F%dMn", i);
        SMCReadKey(key, &smcValue);
        printf("Fan %i Minimum speed: %f\n", i, GetFloatFromBytes(smcValue));

        sprintf(key, "F%dMx", i);
        SMCReadKey(key, &smcValue);
        printf("Fan %i Maximum speed: %f\n", i, GetFloatFromBytes(smcValue));

        sprintf(key, "F%dSf", i);
        SMCReadKey(key, &smcValue);
        printf("Fan %i Safe speed: %f\n", i, GetFloatFromBytes(smcValue));

        sprintf(key, "F%dTg", i);
        SMCReadKey(key, &smcValue);
        printf("Fan %i Target speed: %f\n", i, GetFloatFromBytes(smcValue));

        sprintf(key, "F%dMd", i);
        SMCReadKey(key, &smcValue);
        if (GetFloatFromBytes(smcValue)) {
            printf("Mode: forced\n");
        } else {
            printf("Mode: auto\n");
        }
    }
    return returnValue;
}

float GetFloatFromBytes(SMCValue_t value) {
    // We should check datatype first. But living on the edge is also ok.
    float returnValue = -1.f;

    memcpy(&returnValue, value.bytes, sizeof(float));

    return returnValue;
}


kern_return_t SMCReadKey(char* key, SMCValue_t *smcValue)
{
    SMCKeyData_t inputKeyData;
    SMCKeyData_t outputKeydata;
    kern_return_t result;

    memset(&inputKeyData, 0, sizeof(SMCKeyData_t));
    memset(&outputKeydata, 0, sizeof(SMCKeyData_t));
    memset(smcValue, 0, sizeof(SMCValue_t));

    inputKeyData.key = strtouint32(key, 4, 16);
    // Debug
    //printf("Key: %s %i\n", key, inputKeyData.key);
    sprintf(smcValue->key, "%s", key);

    inputKeyData.data8 = SMC_CMD_READ_KEYINFO;

    size_t   structureOutputSize;
    structureOutputSize = sizeof(SMCKeyData_t);
    result = IOConnectCallStructMethod(gConn, KERNEL_INDEX_SMC, &inputKeyData, sizeof(SMCKeyData_t), &outputKeydata, &structureOutputSize );

    if (result != kIOReturnSuccess) {
        return result;
    }

    smcValue->dataSize = outputKeydata.keyInfo.dataSize;
    //Debug
    //printf("KeyInfo DataSize: %i\n", outputKeydata.keyInfo.dataSize);
    uint32tostr(smcValue->dataType, outputKeydata.keyInfo.dataType);
    inputKeyData.keyInfo.dataSize = smcValue->dataSize;
    inputKeyData.data8 = SMC_CMD_READ_BYTES;

    result = IOConnectCallStructMethod(gConn, KERNEL_INDEX_SMC, &inputKeyData, sizeof(SMCKeyData_t), &outputKeydata, &structureOutputSize);

    if (result != kIOReturnSuccess) {
        return result;
    }

    memcpy(smcValue->bytes, outputKeydata.smcBytes, sizeof(outputKeydata.smcBytes));

    return result;
}

UInt32 strtouint32(char *input, int size, int base)
{
    UInt32 number = 0;

    for (int i = 0; i < size; i++)
    {
        if (base == 16 ) {
            number += input[i] << (size - 1 - i) * 8;
        } else {
            number += ((unsigned char) (input[i]) << (size - 1 - i) * 8);
        }
    }
    return number;
}

void uint32tostr(char *input, UInt32 value)
{
    input[0] = '\0';
    sprintf(input, "%c%c%c%c",
            (unsigned int) value >> 24,
            (unsigned int) value >> 16,
            (unsigned int) value >> 8,
            (unsigned int) value);
}

