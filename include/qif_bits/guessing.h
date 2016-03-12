
namespace guessing {

template<typename eT>
eT entropy(const Prob<eT>& pi) {
	Prob<eT> sorted = sort(pi);

	eT sum(0);
	for(uint x = 1; x <= sorted.n_cols; ++x) {
		sum += x * sorted.at(sorted.n_cols - x);
	}
	return sum;
}

template<typename eT>
eT post_entropy(const Prob<eT>& pi, const Chan<eT>& C) {
	channel::check_prior_size(pi, C);

	eT result(0);
	for(uint y = 0; y < C.n_cols; y++) {
		//create the vector vy = pi(1)* C[1,y] ... pi(x)* C[x,y]
		Prob<eT> vy = pi % arma::trans(C.col(y));
		result += entropy<eT>(vy);
	}
	return result;
}

template<typename eT>
eT add_leakage(const Prob<eT>& pi, const Chan<eT>& C) {
	return post_entropy(pi, C) - entropy(pi);
}

template<typename eT>
eT mult_leakage(const Prob<eT>& pi, const Chan<eT>& C) {
	return entropy(pi) / post_entropy(pi, C);
}

template<typename eT>
eT mulg_leakage(const Prob<eT>& pi, const Chan<eT>& C) {
	return real_ops<eT>::log2(mult_leakage(pi, C));
}

} // namespace guessing
