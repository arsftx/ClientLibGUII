#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ClientValidation.h"
#include <rpc.h>  // For UUID and UuidCreateSequential
#include <stdio.h>
#include <string.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")

// ============================================================
// Configuration
// ============================================================
#define VALIDATION_PORT 17599
#define VALIDATION_SERVER "212.87.221.207"  // Filter server IP

// ============================================================
// Static Member Initialization
// ============================================================
unsigned char ClientValidation::s_PendingChallenge[CHALLENGE_SIZE] = {0};
bool ClientValidation::s_HasPendingChallenge = false;
DWORD ClientValidation::s_ChallengeTimestamp = 0;
char ClientValidation::s_HWID[65] = {0};
bool ClientValidation::s_HWIDGenerated = false;

// TCP connection state
static SOCKET s_Socket = INVALID_SOCKET;
static bool s_bValidated = false;
static HANDLE s_hValidationThread = NULL;

// ============================================================
// Helper: Read exact number of bytes from socket
// ============================================================
static bool recv_all(SOCKET sock, char* buffer, int totalBytes)
{
    int received = 0;
    while (received < totalBytes)
    {
        int result = recv(sock, buffer + received, totalBytes - received, 0);
        if (result <= 0)
        {
            return false;
        }
        received += result;
    }
    return true;
}
// ============================================================
// HWID Generation (Native Windows API - No WMI)
// ============================================================
#include <intrin.h>

// SMBIOS structures
#pragma pack(push, 1)
struct RawSMBIOSData {
    BYTE Used20CallingMethod;
    BYTE SMBIOSMajorVersion;
    BYTE SMBIOSMinorVersion;
    BYTE DmiRevision;
    DWORD Length;
    BYTE SMBIOSTableData[1];
};

struct SMBIOSHeader {
    BYTE Type;
    BYTE Length;
    WORD Handle;
};
#pragma pack(pop)

// Get SMBIOS UUID from firmware table (Type 1 - System Information)
static void GetSMBIOSUUID(char* buffer, int size)
{
    buffer[0] = 0;
    
    DWORD bufSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (bufSize == 0) return;
    
    BYTE* data = new BYTE[bufSize];
    if (GetSystemFirmwareTable('RSMB', 0, data, bufSize) == 0)
    {
        delete[] data;
        return;
    }
    
    RawSMBIOSData* smbios = (RawSMBIOSData*)data;
    BYTE* p = smbios->SMBIOSTableData;
    BYTE* end = p + smbios->Length;
    
    while (p < end)
    {
        SMBIOSHeader* header = (SMBIOSHeader*)p;
        
        if (header->Type == 1 && header->Length >= 25) // System Information
        {
            // UUID is at offset 8, 16 bytes
            BYTE* uuid = p + 8;
            sprintf_s(buffer, size, 
                "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                uuid[3], uuid[2], uuid[1], uuid[0],
                uuid[5], uuid[4],
                uuid[7], uuid[6],
                uuid[8], uuid[9],
                uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
            break;
        }
        
        // Skip to next structure (header + data + double null terminator)
        p += header->Length;
        while (p < end - 1 && !(p[0] == 0 && p[1] == 0)) p++;
        p += 2;
    }
    
    delete[] data;
}

// Get CPU Processor ID using CPUID (stable)
static void GetCPUProcessorID(char* buffer, int size)
{
    int cpuInfo[4] = {0};
    
    // CPUID with EAX=1 returns processor signature in EAX
    __cpuid(cpuInfo, 1);
    unsigned int signature = cpuInfo[0];
    
    // CPUID with EAX=3 returns processor serial (if available, often disabled)
    __cpuid(cpuInfo, 0);
    unsigned int vendor1 = cpuInfo[1];
    unsigned int vendor2 = cpuInfo[2];
    unsigned int vendor3 = cpuInfo[3];
    
    sprintf_s(buffer, size, "%08X%08X%08X%08X", signature, vendor1, vendor2, vendor3);
}

// Get disk serial from registry (stable alternative to WMI)
static void GetDiskSerial(char* buffer, int size)
{
    buffer[0] = 0;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", 
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        char diskPath[256] = {0};
        DWORD pathSize = sizeof(diskPath);
        
        if (RegQueryValueExA(hKey, "0", NULL, NULL, (LPBYTE)diskPath, &pathSize) == ERROR_SUCCESS)
        {
            // Extract serial from disk path if available
            strncpy_s(buffer, size, diskPath, _TRUNCATE);
        }
        RegCloseKey(hKey);
    }
    
    // Fallback: use volume serial
    if (buffer[0] == 0)
    {
        DWORD volumeSerial = 0;
        if (GetVolumeInformationA("C:\\", NULL, 0, &volumeSerial, NULL, NULL, NULL, 0))
        {
            sprintf_s(buffer, size, "%08X", volumeSerial);
        }
    }
}

static void SHA256Hash(const char* input, char* output, int outputSize)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE hash[32] = {0};
    DWORD hashLen = 32;
    
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
        {
            CryptHashData(hHash, (BYTE*)input, (DWORD)strlen(input), 0);
            CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    
    for (DWORD i = 0; i < hashLen && (int)(i * 2 + 2) < outputSize; i++)
    {
        sprintf_s(output + i * 2, 3, "%02X", hash[i]);
    }
}

void ClientValidation::GenerateHWID(char* buffer, int bufferSize)
{
    if (s_HWIDGenerated)
    {
        strcpy_s(buffer, bufferSize, s_HWID);
        return;
    }
    
    char anakartUUID[64] = {0};
    char cpuInfo[64] = {0};
    char diskSerial[256] = {0};
    char combined[512] = {0};
    
    // HWID = Hash(AnakartUUID + CPUInfo + DiskSerial)
    GetSMBIOSUUID(anakartUUID, sizeof(anakartUUID));
    GetCPUProcessorID(cpuInfo, sizeof(cpuInfo));
    GetDiskSerial(diskSerial, sizeof(diskSerial));
    
    // Combine: AnakartUUID + CPUInfo + DiskSerial
    sprintf_s(combined, sizeof(combined), "%s|%s|%s", 
        anakartUUID[0] ? anakartUUID : "UUID",
        cpuInfo[0] ? cpuInfo : "CPU",
        diskSerial[0] ? diskSerial : "DISK");
    
    SHA256Hash(combined, s_HWID, sizeof(s_HWID));
    s_HWIDGenerated = true;
    
    strcpy_s(buffer, bufferSize, s_HWID);
}

// ============================================================
// HMAC-SHA256 Computation
// ============================================================
bool ClientValidation::ComputeHMAC(
    const unsigned char* data, 
    int dataLen,
    unsigned char* outHash,
    int* outHashLen)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BOOL success = FALSE;
    
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        return false;
    }
    
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        CryptReleaseContext(hProv, 0);
        return false;
    }
    
    // Hash: SharedSecret (31 bytes, excluding null) + data
    if (!CryptHashData(hHash, g_SharedSecret, 31, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    
    if (!CryptHashData(hHash, data, dataLen, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    
    DWORD hashLen = 32;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, outHash, &hashLen, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    
    *outHashLen = (int)hashLen;
    success = TRUE;
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    
    return success == TRUE;
}

// ============================================================
// TCP Socket Validation Thread
// ============================================================
static DWORD WINAPI ValidationThreadProc(LPVOID lpParam)
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }
    
    // Wait a bit for game to initialize
    Sleep(1000);
    
    // Try to connect to validation server
    for (int retry = 0; retry < 10; retry++)
    {
        s_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_Socket == INVALID_SOCKET)
        {
            Sleep(2000);
            continue;
        }
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(VALIDATION_PORT);
        serverAddr.sin_addr.s_addr = inet_addr(VALIDATION_SERVER);
        
        if (connect(s_Socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            closesocket(s_Socket);
            s_Socket = INVALID_SOCKET;
            Sleep(2000);
            continue;
        }
        
        break;
    }
    
    if (s_Socket == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }
    
    unsigned char buffer[512] = {0};
    int pos = 0;
    
    // 1. Generate and send version + HWID
    char hwid[65] = {0};
    ClientValidation::GenerateHWID(hwid, sizeof(hwid));
    
    int versionLen = (int)strlen(CLIENT_VERSION);
    int hwidLen = (int)strlen(hwid);
    
    // Write: [versionLen:2][version][hwidLen:2][hwid]
    *(unsigned short*)(buffer + pos) = (unsigned short)versionLen;
    pos += 2;
    memcpy(buffer + pos, CLIENT_VERSION, versionLen);
    pos += versionLen;
    *(unsigned short*)(buffer + pos) = (unsigned short)hwidLen;
    pos += 2;
    memcpy(buffer + pos, hwid, hwidLen);
    pos += hwidLen;
    
    if (send(s_Socket, (char*)buffer, pos, 0) == SOCKET_ERROR)
    {
        closesocket(s_Socket);
        WSACleanup();
        return 1;
    }
    
    // 2. Read response code (1 byte)
    memset(buffer, 0, sizeof(buffer));
    if (!recv_all(s_Socket, (char*)buffer, 1))
    {
        closesocket(s_Socket);
        WSACleanup();
        return 1;
    }
    
    unsigned char responseCode = buffer[0];
    
    if (responseCode == 0)
    {
        // REJECTED
        closesocket(s_Socket);
        WSACleanup();
        return 1;
    }
    else if (responseCode == 1)
    {
        // CHALLENGE - read exactly 32 bytes
        if (!recv_all(s_Socket, (char*)ClientValidation::s_PendingChallenge, CHALLENGE_SIZE))
        {
            closesocket(s_Socket);
            WSACleanup();
            return 1;
        }
        
        // 3. Compute SHA256 response
        int signDataLen = CHALLENGE_SIZE + versionLen + hwidLen;
        unsigned char* signData = new unsigned char[signDataLen];
        memcpy(signData, ClientValidation::s_PendingChallenge, CHALLENGE_SIZE);
        memcpy(signData + CHALLENGE_SIZE, CLIENT_VERSION, versionLen);
        memcpy(signData + CHALLENGE_SIZE + versionLen, hwid, hwidLen);
        
        unsigned char signature[32] = {0};
        int signatureLen = 0;
        
        if (!ClientValidation::ComputeHMAC(signData, signDataLen, signature, &signatureLen))
        {
            delete[] signData;
            closesocket(s_Socket);
            WSACleanup();
            return 1;
        }
        
        delete[] signData;
        
        // 4. Send signature back
        if (send(s_Socket, (char*)signature, 32, 0) == SOCKET_ERROR)
        {
            closesocket(s_Socket);
            WSACleanup();
            return 1;
        }
        
        // 5. Read final result
        memset(buffer, 0, sizeof(buffer));
        if (!recv_all(s_Socket, (char*)buffer, 1))
        {
            closesocket(s_Socket);
            WSACleanup();
            return 1;
        }
        
        responseCode = buffer[0];
        if (responseCode == 2)
        {
            // ACCEPTED
            s_bValidated = true;
        }
    }
    else if (responseCode == 2)
    {
        // Direct ACCEPTED (no challenge needed)
        s_bValidated = true;
    }
    
    closesocket(s_Socket);
    WSACleanup();
    s_Socket = INVALID_SOCKET;
    
    return 0;
}

// ============================================================
// Public Interface
// ============================================================
void ClientValidation::Initialize()
{
    char hwid[65] = {0};
    GenerateHWID(hwid, sizeof(hwid));
    
    // Start validation thread
    s_hValidationThread = CreateThread(NULL, 0, ValidationThreadProc, NULL, 0, NULL);
}

void ClientValidation::InstallHooks()
{
    // No hooks needed for TCP Socket approach
}

bool ClientValidation::IsValidated()
{
    return s_bValidated;
}

// Legacy functions (kept for compatibility but not used)
void ClientValidation::OnChallengeReceived(void* pNetProcess, void* pMsg)
{
    // Not used in TCP Socket approach
}

void ClientValidation::SendValidationResponse(const unsigned char* challenge, int challengeLen)
{
    // Not used in TCP Socket approach
}
