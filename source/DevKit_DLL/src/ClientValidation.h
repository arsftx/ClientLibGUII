#pragma once
#ifndef CLIENT_VALIDATION_H
#define CLIENT_VALIDATION_H

#include <windows.h>
#include <intrin.h>  // For __cpuid

// Configuration
#define CLIENT_VERSION "1.0.0"
#define CHALLENGE_SIZE   32
#define HMAC_SIZE        32

// Shared secret (must match Filter8.0)
// "Fantasy_Filter_Secret_32_Bytes!" in hex
static const unsigned char g_SharedSecret[] = {
    0x46, 0x61, 0x6E, 0x74, 0x61, 0x73, 0x79, 0x5F, // Fantasy_
    0x46, 0x69, 0x6C, 0x74, 0x65, 0x72, 0x5F, 0x53, // Filter_S
    0x65, 0x63, 0x72, 0x65, 0x74, 0x5F, 0x33, 0x32, // ecret_32
    0x5F, 0x42, 0x79, 0x74, 0x65, 0x73, 0x21, 0x00  // _Bytes!
};

class ClientValidation
{
public:
    // Initialize validation (starts pipe thread)
    static void Initialize();
    
    // Install hooks (not used in Named Pipe approach)
    static void InstallHooks();
    
    // Check if client has been validated
    static bool IsValidated();
    
    // Generate HWID hash (CPU + Volume Serial + MAC)
    static void GenerateHWID(char* buffer, int bufferSize);
    
    // Compute HMAC-SHA256 using WinCrypt API
    static bool ComputeHMAC(
        const unsigned char* data, 
        int dataLen,
        unsigned char* outHash,
        int* outHashLen
    );
    
    // Pending challenge state (used by pipe thread)
    static unsigned char s_PendingChallenge[CHALLENGE_SIZE];
    static bool s_HasPendingChallenge;
    static DWORD s_ChallengeTimestamp;
    
    // Cached HWID
    static char s_HWID[65];
    static bool s_HWIDGenerated;
    
    // Legacy functions (kept for compatibility)
    static void OnChallengeReceived(void* pNetProcess, void* pMsg);
    static void SendValidationResponse(const unsigned char* challenge, int challengeLen);
};

#endif // CLIENT_VALIDATION_H
