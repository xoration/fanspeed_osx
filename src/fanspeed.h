#ifndef __FANSPEED_H
#define __FANSPEED_H
#include <IOKIT/IOKitLib.h>

#define SMC_CMD_READ_KEYINFO  9
#define KERNEL_INDEX_SMC      2
#define SMC_CMD_READ_BYTES    5

typedef struct {
    char major;
    char minor;
    char build;
    char reserved[1];
    UInt16 release;
} SMCKeyData_version_t;

typedef struct {
    UInt16                version;
    UInt16                length;
    UInt32                cpuPLimit;
    UInt32                gpuPLimit;
    UInt32                memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct {
    UInt32                dataSize;
    UInt32                dataType;
    char                  dataAttributes;
} SMCKeyData_keyInfo_t;

typedef struct {
    unsigned int  key;
    SMCKeyData_version_t version;
    SMCKeyData_pLimitData_t pLimitData;
    SMCKeyData_keyInfo_t keyInfo;
    char result;
    char status;
    char data8;
    UInt32 data32;
    unsigned char smcBytes[32];
} SMCKeyData_t;

typedef struct {
    char key[5];
    UInt32 dataSize;
    char dataType[5];
    char bytes[32];
} SMCValue_t;

kern_return_t SMCOpen();
kern_return_t SMCReadKey(char* key, SMCValue_t *smcValue);
UInt32 strtouint32(char *str, int size, int base);
void uint32tostr(char *str, UInt32 val);
float GetFloatFromBytes(SMCValue_t value);
kern_return_t SMCReadFans();
//kern_return_t SMCReadKey2(UInt32Char_t key, SMCVal_t *val,io_connect_t conn);


#endif
