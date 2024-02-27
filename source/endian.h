// Copyright 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <cinttypes>

#if __BIG_ENDIAN__
#define htole16(x) ((uint16_t)(((x) >> 8) & 0xFF) | ((uint16_t)((x)&0xFF) << 8))
#define htole32(x) (((uint32_t)htobe16(((x) >> 16) & 0xFFFF)) | ((uint32_t)htobe16((x)&0xFFFF) << 16))
#define htole64(x) (((uint64_t)htobe32(((x) >> 32) & 0xFFFFFFFF)) | ((uint64_t)htobe32((x)&0xFFFFFFFF) << 32))
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)
#else
#define htobe16(x) ((uint16_t)(((x) >> 8) & 0xFF) | ((uint16_t)((x)&0xFF) << 8))
#define htobe32(x) (((uint32_t)htobe16(((x) >> 16) & 0xFFFF)) | ((uint32_t)htobe16((x)&0xFFFF) << 16))
#define htobe64(x) (((uint64_t)htobe32(((x) >> 32) & 0xFFFFFFFF)) | ((uint64_t)htobe32((x)&0xFFFFFFFF) << 32))
#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)
#endif

#define le16toh(x) htole16(x)
#define le32toh(x) htole32(x)
#define le64toh(x) htole64(x)
#define be16toh(x) htobe16(x)
#define be32toh(x) htobe32(x)
#define be64toh(x) htobe64(x)
