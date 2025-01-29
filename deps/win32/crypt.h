/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_CRYPT_H
#define WINDOWS_CRYPT_H

#include "windows_base.h"

/* Disable all warnings */
#if defined(_MSC_VER)
    #pragma warning(push, 0)
#endif
#if defined(__cplusplus)
extern "C" {
#endif

typedef PVOID BCRYPT_HANDLE;
typedef PVOID BCRYPT_ALG_HANDLE;
typedef PVOID BCRYPT_KEY_HANDLE;
typedef PVOID BCRYPT_HASH_HANDLE;
typedef PVOID BCRYPT_SECRET_HANDLE;

#define BCRYPT_RSA_ALGORITHM                    L"RSA"
#define BCRYPT_RSA_SIGN_ALGORITHM               L"RSA_SIGN"
#define BCRYPT_DH_ALGORITHM                     L"DH"
#define BCRYPT_DSA_ALGORITHM                    L"DSA"
#define BCRYPT_RC2_ALGORITHM                    L"RC2"
#define BCRYPT_RC4_ALGORITHM                    L"RC4"
#define BCRYPT_AES_ALGORITHM                    L"AES"
#define BCRYPT_DES_ALGORITHM                    L"DES"
#define BCRYPT_DESX_ALGORITHM                   L"DESX"
#define BCRYPT_3DES_ALGORITHM                   L"3DES"
#define BCRYPT_3DES_112_ALGORITHM               L"3DES_112"
#define BCRYPT_MD2_ALGORITHM                    L"MD2"
#define BCRYPT_MD4_ALGORITHM                    L"MD4"
#define BCRYPT_MD5_ALGORITHM                    L"MD5"
#define BCRYPT_SHA1_ALGORITHM                   L"SHA1"
#define BCRYPT_SHA256_ALGORITHM                 L"SHA256"
#define BCRYPT_SHA384_ALGORITHM                 L"SHA384"
#define BCRYPT_SHA512_ALGORITHM                 L"SHA512"
#define BCRYPT_AES_GMAC_ALGORITHM               L"AES-GMAC"
#define BCRYPT_AES_CMAC_ALGORITHM               L"AES-CMAC"
#define BCRYPT_ECDSA_P256_ALGORITHM             L"ECDSA_P256"
#define BCRYPT_ECDSA_P384_ALGORITHM             L"ECDSA_P384"
#define BCRYPT_ECDSA_P521_ALGORITHM             L"ECDSA_P521"
#define BCRYPT_ECDH_P256_ALGORITHM              L"ECDH_P256"
#define BCRYPT_ECDH_P384_ALGORITHM              L"ECDH_P384"
#define BCRYPT_ECDH_P521_ALGORITHM              L"ECDH_P521"
#define BCRYPT_RNG_ALGORITHM                    L"RNG"
#define BCRYPT_RNG_FIPS186_DSA_ALGORITHM        L"FIPS186DSARNG"
#define BCRYPT_RNG_DUAL_EC_ALGORITHM            L"DUALECRNG"

// #if (NTDDI_VERSION >= NTDDI_WIN8)
#define BCRYPT_SP800108_CTR_HMAC_ALGORITHM      L"SP800_108_CTR_HMAC"
#define BCRYPT_SP80056A_CONCAT_ALGORITHM        L"SP800_56A_CONCAT"
#define BCRYPT_PBKDF2_ALGORITHM                 L"PBKDF2"
#define BCRYPT_CAPI_KDF_ALGORITHM               L"CAPI_KDF"
#define BCRYPT_TLS1_1_KDF_ALGORITHM             L"TLS1_1_KDF"
#define BCRYPT_TLS1_2_KDF_ALGORITHM             L"TLS1_2_KDF"
// #endif

// #if (NTDDI_VERSION >= NTDDI_WINTHRESHOLD)
#define BCRYPT_ECDSA_ALGORITHM                  L"ECDSA"
#define BCRYPT_ECDH_ALGORITHM                   L"ECDH"
#define BCRYPT_XTS_AES_ALGORITHM                L"XTS-AES"
// #endif

// #if (NTDDI_VERSION >= NTDDI_WIN10_RS4)
#define BCRYPT_HKDF_ALGORITHM                   L"HKDF"
// #endif

// #if (NTDDI_VERSION >= NTDDI_WIN10_FE)
#define BCRYPT_CHACHA20_POLY1305_ALGORITHM      L"CHACHA20_POLY1305"
// #endif

#define BCRYPT_OBJECT_LENGTH        L"ObjectLength"
#define BCRYPT_ALGORITHM_NAME       L"AlgorithmName"
#define BCRYPT_PROVIDER_HANDLE      L"ProviderHandle"
#define BCRYPT_CHAINING_MODE        L"ChainingMode"
#define BCRYPT_BLOCK_LENGTH         L"BlockLength"
#define BCRYPT_KEY_LENGTH           L"KeyLength"
#define BCRYPT_KEY_OBJECT_LENGTH    L"KeyObjectLength"
#define BCRYPT_KEY_STRENGTH         L"KeyStrength"
#define BCRYPT_KEY_LENGTHS          L"KeyLengths"
#define BCRYPT_BLOCK_SIZE_LIST      L"BlockSizeList"
#define BCRYPT_EFFECTIVE_KEY_LENGTH L"EffectiveKeyLength"
#define BCRYPT_HASH_LENGTH          L"HashDigestLength"
#define BCRYPT_HASH_OID_LIST        L"HashOIDList"
#define BCRYPT_PADDING_SCHEMES      L"PaddingSchemes"
#define BCRYPT_SIGNATURE_LENGTH     L"SignatureLength"
#define BCRYPT_HASH_BLOCK_LENGTH    L"HashBlockLength"
#define BCRYPT_AUTH_TAG_LENGTH      L"AuthTagLength"

// #if (NTDDI_VERSION >= NTDDI_WIN7)
#define BCRYPT_PRIMITIVE_TYPE       L"PrimitiveType"
#define BCRYPT_IS_KEYED_HASH        L"IsKeyedHash"
// #endif

// #if (NTDDI_VERSION >= NTDDI_WIN8)
#define BCRYPT_IS_REUSABLE_HASH     L"IsReusableHash"
#define BCRYPT_MESSAGE_BLOCK_LENGTH L"MessageBlockLength"
// #endif

// #if (NTDDI_VERSION >= NTDDI_WIN8)
#define BCRYPT_PUBLIC_KEY_LENGTH    L"PublicKeyLength"
// #endif

// Additional BCryptGetProperty strings for the RNG Platform Crypto Provider
#define BCRYPT_PCP_PLATFORM_TYPE_PROPERTY    L"PCP_PLATFORM_TYPE"
#define BCRYPT_PCP_PROVIDER_VERSION_PROPERTY L"PCP_PROVIDER_VERSION"


LONG WINAPI BCryptOpenAlgorithmProvider(
    BCRYPT_ALG_HANDLE   *phAlgorithm,
    LPCWSTR pszAlgId,
    LPCWSTR pszImplementation,
    ULONG   dwFlags);

LONG WINAPI BCryptGetProperty(
    BCRYPT_HANDLE   hObject,
    LPCWSTR pszProperty,
    UCHAR*   pbOutput,
    ULONG   cbOutput,
    ULONG   *pcbResult,
    ULONG   dwFlags);

LONG WINAPI BCryptCreateHash(
    BCRYPT_ALG_HANDLE   hAlgorithm,
    BCRYPT_HASH_HANDLE  *phHash,
    UCHAR*  pbHashObject,
    ULONG   cbHashObject,
    UCHAR*  pbSecret,   // optional
    ULONG   cbSecret,   // optional
    ULONG   dwFlags);

LONG WINAPI BCryptHashData(
    BCRYPT_HASH_HANDLE  hHash,
    UCHAR*  pbInput,
    ULONG   cbInput,
    ULONG   dwFlags);

LONG WINAPI BCryptFinishHash(
    BCRYPT_HASH_HANDLE hHash,
    UCHAR*  pbOutput,
    ULONG   cbOutput,
    ULONG   dwFlags);

#if defined(__cplusplus)
}
#endif
/* Enable all warnings */
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#endif /* WINDOWS_CRYPT_H */
#endif /* _WINDOWS_ */