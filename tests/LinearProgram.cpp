/*
This file belongs to the LIBQIF library.
A Quantitative Information Flow C++ Toolkit Library.
Copyright (C) 2013  Universidad Nacional de Río Cuarto(National University of Río Cuarto).
Author: Martinelli Fernán - fmartinelli89@gmail.com - Universidad Nacional de Río Cuarto (Argentina)
LIBQIF Version: 1.0
Date: 12th Nov 2013
========================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

=========================================================================
*/
#include "tests_aux.h"
using namespace qif::lp;


// define a type-parametrized test case (https://code.google.com/p/googletest/wiki/AdvancedGuide)
template <typename eT>
class LinearProgramTest : public BaseTest<eT> {};

TYPED_TEST_CASE_P(LinearProgramTest);


TYPED_TEST_P(LinearProgramTest, Optimal) {
	typedef TypeParam eT;
	typedef Method m_t;
	typedef Status s_t;
	const bool is_rat = std::is_same<eT, rat>::value;

	// the default acceptance range is too string for linear programs, we need a more permissive mrd
	eT md =	def_max_diff<eT>();
	eT mrd = def_max_rel_diff<float>();		// always use the mrd for floats

	#ifdef QIF_USE_ORTOOLS
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK, Solver::GLOP, Solver::CLP };
	#else
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK };
	#endif

	for(m_t method : { m_t::SIMPLEX_PRIMAL, m_t::SIMPLEX_DUAL, m_t::INTERIOR }) {
	for(Solver solver : solvers) {
	for(bool presolve : { false, true }) {
		if(method == m_t::INTERIOR    && (presolve || solver == Solver::GLOP || solver == Solver::CLP)) continue; // interior: no presolver, no GLOP support, unstable with CLP
		if(solver == Solver::INTERNAL && (presolve || method != m_t::SIMPLEX_PRIMAL)                  ) continue; // internal solver: only simplex_primal/no presolve
		if(is_rat                     && solver != Solver::INTERNAL                                   ) continue; // rat: only internal solver

		LinearProgram<eT> lp;
		lp.method = method;
		lp.solver = solver;
		lp.presolve = presolve;

		lp.from_matrix(
			format_num<eT>("1 2; 3 1"),
			format_num<eT>("1 2"),
			format_num<eT>("0.6 0.5")
		);

		EXPECT_TRUE(lp.solve());
		EXPECT_EQ(s_t::OPTIMAL, lp.status);
		EXPECT_PRED_FORMAT4(equal4<eT>, eT(46)/100, lp.objective(), md, mrd);
		expect_mat(format_num<eT>("0.6; 0.2"), lp.solution(), md, mrd);

		lp.from_matrix(
			format_num<eT>("1 1 0; 0 1 1"),
			format_num<eT>("1 1"),
			format_num<eT>("1 2 -1")
		);

		EXPECT_TRUE(lp.solve());
		EXPECT_EQ(s_t::OPTIMAL, lp.status);
		EXPECT_PRED_FORMAT4(equal4<eT>, eT(2), lp.objective(), md, mrd);
		expect_mat(format_num<eT>("0; 1; 0"), lp.solution(), md, mrd);

		lp.maximize = false;
		lp.from_matrix(
			format_num<eT>("3 -4; 1 2; 1 0"),
			format_num<eT>("12 4 1"),
			format_num<eT>("3 4"),
			{ '<', '>', '>' }
		);

		EXPECT_TRUE(lp.solve());
		EXPECT_EQ(s_t::OPTIMAL, lp.status);
		EXPECT_PRED_FORMAT4(equal4<eT>, eT(9), lp.objective(), md, mrd);
		expect_mat(format_num<eT>("1; 1.5"), lp.solution(), md, mrd);

		lp.maximize = false;
		lp.from_matrix(
			format_num<eT>("1 2 2; 2 1 2; 2 2 1"),
			format_num<eT>("20 20 20"),
			format_num<eT>("-10 -12 -12")
		);

		EXPECT_TRUE(lp.solve());
		EXPECT_EQ(s_t::OPTIMAL, lp.status);
		EXPECT_PRED_FORMAT4(equal4<eT>, eT(-136), lp.objective(), md, mrd);
		expect_mat(format_num<eT>("4; 4; 4"), lp.solution(), md, mrd);

		lp.clear();
		lp.maximize = false;
		auto v = lp.make_var(-5, infinity<eT>());
		lp.make_con(0, 0);		// glpk needs at least one constraint, add dummy one
		lp.set_obj_coeff(v, 1);

		EXPECT_TRUE(lp.solve());
		EXPECT_EQ(s_t::OPTIMAL, lp.status);
		EXPECT_PRED_FORMAT4(equal4<eT>, eT(-5), lp.objective(), md, mrd);
		expect_mat(format_num<eT>("-5"), lp.solution(), md, mrd);
	}}}
}

TYPED_TEST_P(LinearProgramTest, Infeasible) {
	typedef TypeParam eT;
	typedef Method m_t;
	typedef Status s_t;
	const bool is_rat = std::is_same<eT, rat>::value;

	#ifdef QIF_USE_ORTOOLS
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK, Solver::GLOP, Solver::CLP };
	#else
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK };
	#endif

	for(m_t method : { m_t::SIMPLEX_PRIMAL, m_t::SIMPLEX_DUAL, m_t::INTERIOR }) {
	for(Solver solver : solvers) {
	for(bool presolve : { false, true }) {
		if(method == m_t::INTERIOR    && (presolve || solver == Solver::GLOP || solver == Solver::CLP)) continue; // interior: no presolver, no GLOP support, unstable with CLP
		if(solver == Solver::INTERNAL && (presolve || method != m_t::SIMPLEX_PRIMAL)                  ) continue; // internal solver: only simplex_primal/no presolve
		if(is_rat                     && solver != Solver::INTERNAL                                   ) continue; // rat: only internal solver

		LinearProgram<eT> lp;
		lp.method = method;
		lp.solver = solver;
		lp.presolve = presolve;

		s_t status = method == m_t::INTERIOR
				? s_t::INFEASIBLE_OR_UNBOUNDED	// sometimes we just know that the problem is infeasible OR unbounded
				: s_t::INFEASIBLE;

		lp.from_matrix(
			format_num<eT>("1; 1"),
			format_num<eT>("3 2"),
			format_num<eT>("1"),
			{ '>', '<' }
		);

		EXPECT_FALSE(lp.solve());
		EXPECT_EQ(status, lp.status);

		lp.from_matrix(
			format_num<eT>("1; -1"),
			format_num<eT>("3 -2"),
			format_num<eT>("4"),
			{ '>', '>' }
		);

		EXPECT_FALSE(lp.solve());
		EXPECT_EQ(status, lp.status);
	}}}
}

TYPED_TEST_P(LinearProgramTest, Unbounded) {
	typedef TypeParam eT;
	typedef Method m_t;
	typedef Status s_t;
	const bool is_rat = std::is_same<eT, rat>::value;

	#ifdef QIF_USE_ORTOOLS
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK, Solver::GLOP, Solver::CLP };
	#else
	std::vector<Solver> solvers = { Solver::INTERNAL, Solver::GLPK };
	#endif

	for(m_t method : { m_t::SIMPLEX_PRIMAL, m_t::SIMPLEX_DUAL, m_t::INTERIOR }) {
	for(Solver solver : solvers) {
	for(bool presolve : { false, true }) {
		if(method == m_t::INTERIOR    && (presolve || solver == Solver::GLOP || solver == Solver::CLP)) continue; // interior: no presolver, no GLOP support, unstable with CLP
		if(solver == Solver::INTERNAL && (presolve || method != m_t::SIMPLEX_PRIMAL)                  ) continue; // internal solver: only simplex_primal/no presolve
		if(is_rat                     && solver != Solver::INTERNAL                                   ) continue; // rat: only internal solver

		// EXTRA conditions only for unbounded
		if(solver == Solver::GLOP || solver == Solver::CLP) continue; // OR-tools/DUAL seems unstable with unbounded problems (TODO: investigae)

		LinearProgram<eT> lp;
		lp.method = method;
		lp.solver = solver;
		lp.presolve = presolve;

		s_t status = solver != Solver::GLPK || (method == m_t::SIMPLEX_PRIMAL && !presolve)
				? s_t::UNBOUNDED
				: s_t::INFEASIBLE_OR_UNBOUNDED;	// sometimes we just know that the problem is infeasible OR unbounded

		lp.maximize = false;
		lp.from_matrix(
			format_num<eT>("1"),
			format_num<eT>("2"),
			format_num<eT>("-1"),
			{ '>' }
		);

		EXPECT_FALSE(lp.solve());
		EXPECT_EQ(status, lp.status);
	}}}
}


REGISTER_TYPED_TEST_CASE_P(LinearProgramTest, Optimal, Infeasible, Unbounded);

INSTANTIATE_TYPED_TEST_CASE_P(LinearProgram, LinearProgramTest, AllTypes);

