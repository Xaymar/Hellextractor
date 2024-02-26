#pragma once
#include <cstddef>
#include <cstdint>

struct half {
	uint16_t value;

	half()
	{
		operator=(0.0f);
	}
	half(float v)
	{
		operator=(v);
	}
	half(uint16_t v)
	{
		operator=(v);
	}

	half& operator=(float v)
	{
		auto as_uint = [](const float x) { return *(uint32_t*)&x; };

		// IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint32_t b = as_uint(v) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
		const uint32_t e = (b & 0x7F800000) >> 23; // exponent
		const uint32_t m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
		value            = (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
	}

	half& operator=(uint16_t v)
	{
		value = v;
	}

	operator float() const
	{
		auto as_float = [](const uint32_t x) { return *(float*)&x; };
		auto as_uint  = [](const float x) { return *(uint32_t*)&x; };

		// IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
		const uint32_t e = (value & 0x7C00) >> 10; // exponent
		const uint32_t m = (value & 0x03FF) << 13; // mantissa
		const uint32_t v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
		return as_float((value & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
	}

	operator uint16_t() const
	{
		return value;
	}
};
