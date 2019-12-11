using qif::rat;
using qif::uint;


namespace arma {

// register rational<1> as a real type.
template<>
struct arma_real_only<rat> {
	typedef rat result;
};

// use direct_dot_arma (generic dot product implementation) for direct_dot<rat>
//
template<>
arma_hot inline rat op_dot::direct_dot<rat>(const uword n_elem, const rat* const A, const rat* const B) {
	return op_dot::direct_dot_arma<rat>(n_elem, A, B);
}

// for abs
//
template<>
arma_inline rat eop_aux::arma_abs<rat>(const rat x) {
	return mppp::abs(x);
}

// ------------------------------------------------

// register as supported
template<>
struct is_supported_elem_type<rat> {
	static const bool value = true;
};

// armadillo's memory::acquire/release uses malloc/free for speed, which ignores the constructor
// We override it to use C++'s new/delete.
//
template<>
inline
arma_malloc
rat*
memory::acquire<rat>(const uword n_elem) {
	arma_debug_check(
		( size_t(n_elem) > (std::numeric_limits<size_t>::max() / sizeof(rat)) ),
		"arma::memory::acquire(): requested size is too large"
	);
	return ( new(std::nothrow) rat[n_elem] );
}

template<>
arma_inline
void
memory::release<rat>(rat* mem) {
	delete [] mem;
}

template<>
arma_inline
void
memory::release<const rat>(const rat* mem) {
	delete [] mem;
}

// armadillo uses memcpy to copy memory, we can't do that so we manually copy
//
template<>
arma_hot
arma_inline
void
arrayops::copy<rat>(rat* dest, const rat* src, const uword n_elem) {
	for(uint i = 0; i < n_elem; i++)
		dest[i] = src[i];
}

// conversion from strings 
//
#if ARMA_VERSION_MAJOR > 8 || (ARMA_VERSION_MAJOR == 8 && ARMA_VERSION_MINOR >= 400)
template<>
inline
bool
diskio::convert_token(rat& val, const std::string& token) {
	val = token;
	return true;
}
#endif

} // namespace arma

