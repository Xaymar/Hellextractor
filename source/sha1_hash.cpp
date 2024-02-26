/*
template<class T, class D = std::default_delete<T>>
struct shared_ptr_with_deleter : public std::shared_ptr<T> {
	explicit shared_ptr_with_deleter(T* t = nullptr) : std::shared_ptr<T>(t, D()) {}

	// Reset function, as it also needs to properly set the deleter.
	void reset(T* t = nullptr)
	{
		std::shared_ptr<T>::reset(t, D());
	}
};

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>

struct BCRYPT_ALG_HANDLE_deleter {
	void operator()(BCRYPT_ALG_HANDLE p) const
	{
		BCryptCloseAlgorithmProvider(p, 0);
	}
};
using bcrypt_alg_handle_t = shared_ptr_with_deleter<void, BCRYPT_ALG_HANDLE_deleter>;

struct BCRYPT_HASH_HANDLE_deleter {
	void operator()(BCRYPT_HASH_HANDLE p) const
	{
		BCryptDestroyHash(p);
	}
};
using bcrypt_hash_handle_t = shared_ptr_with_deleter<void, BCRYPT_HASH_HANDLE_deleter>;

#else
#pragma error("Not supported yet, do some work to make it work.")
#endif

#ifdef WIN32
{
	BCRYPT_ALG_HANDLE  alghandle;
	BCRYPT_HASH_HANDLE hashhandle;
	ULONG              bytes_copied;
	DWORD              hashobj_length;
	DWORD              hash_length;

	if (auto status = BCryptOpenAlgorithmProvider(&alghandle, BCRYPT_MD5_ALGORITHM, NULL, 0); status) {
		throw std::runtime_error("BCryptOpenAlgorithmProvider");
	}
	bcrypt_alg_handle_t alg(alghandle);

	if (auto status = BCryptGetProperty(alg.get(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashobj_length, sizeof(DWORD), &bytes_copied, 0); status) {
		throw std::runtime_error("BCryptGetProperty BCRYPT_OBJECT_LENGTH");
	}
	std::vector<char> hashobjbuf(hashobj_length, 0);

	if (auto status = BCryptGetProperty(alg.get(), BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(DWORD), &bytes_copied, 0); status) {
		throw std::runtime_error("BCryptGetProperty BCRYPT_HASH_LENGTH");
	}
	std::vector<char> hashbuf(hash_length, 0);

	if (auto status = BCryptCreateHash(alg.get(), &hashhandle, (PUCHAR)hashobjbuf.data(), hashobjbuf.size(), NULL, 0, 0); status) {
		throw std::runtime_error("BCryptCreateHash");
	}
	bcrypt_hash_handle_t hash(hashhandle);

	if (auto status = BCryptHashData(hash.get(), (PBYTE)&udata, sizeof(hd2::meshinfo_datatype_t::data_t), 0); status) {
		throw std::runtime_error("BCryptHashData");
	}
	if (auto status = BCryptFinishHash(hash.get(), (PBYTE)hashbuf.data(), hashbuf.size(), 0); status) {
		throw std::runtime_error("BCryptFinishHash");
	}

	// There should now be a hash in hashbuf.
	std::stringstream sstr;
	for (size_t p = 0; p < hashbuf.size(); p++) {
		sstr << std::format("%02" PRIX8, static_cast<uint32_t>(hashbuf[p]));
	}

	return sstr.str();
}
#else
#pragma error("Not yet implemented")
#endif
*/
