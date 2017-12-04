/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Optimiser component that performs function inlining for arbitrary functions.
 */
#pragma once

#include <libjulia/Aliases.h>

#include <libsolidity/interface/Exceptions.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace dev
{
namespace julia
{

/**
 * Optimiser component that modifies an AST in place, inlining arbitrary functions.
 *
 * Code of the form
 *
 * function f(a, b) -> c { ... }
 * h(g(x(...), f(arg1(...), arg2(...)), y(...)), z(...))
 *
 * is transformed into
 *
 * function f(a, b) -> c, d { ... }
 *
 * let z1 := z(...) let y1 := f(...) let a2 := arg2(...) let a1 := arg1(...)
 * let c1 := 0
 * { code of f, with replacements: a -> a1, b -> a2, c -> c1, d -> d1 }
 * h(g(x(...), c1, y1), z1)
 *
 * No temporary variable is created for expressions that are "movable"
 * (i.e. they are "pure", have no side-effects and also do not depend on other code
 * that might have side-effects).
 *
 * This component can only be used on sources with unique names.
 */
class FullInliner: public boost::static_visitor<std::vector<Statement>>
{
public:
	FullInliner(Block& _block):
		m_block(_block)
	{}

	void run();

	// These return statements to be prefixed as soon as we reach the block layer.

	std::vector<Statement> operator()(Literal&) { return {}; }
	std::vector<Statement> operator()(Instruction&) { solAssert(false, ""); return {}; }
	std::vector<Statement> operator()(Identifier&) { return {}; }
	std::vector<Statement> operator()(FunctionalInstruction& _instr);
	std::vector<Statement> operator()(FunctionCall&);
	std::vector<Statement> operator()(Label&) { solAssert(false, ""); return {}; }
	std::vector<Statement> operator()(StackAssignment&) { solAssert(false, ""); return {}; }
	std::vector<Statement> operator()(Assignment& _assignment);
	std::vector<Statement> operator()(VariableDeclaration& _varDecl);
	std::vector<Statement> operator()(If& _if);
	std::vector<Statement> operator()(Switch& _switch);
	std::vector<Statement> operator()(FunctionDefinition&);
	std::vector<Statement> operator()(ForLoop&);
	std::vector<Statement> operator()(Block& _block);

private:
	std::vector<Statement> tryInline(Statement& _statement);
	Statement replace(Statement const& _statement, std::map<std::string, Statement const*> const& _replacements);

	/// The functions we are inside of (we cannot inline them).
	std::set<std::string> m_functionScopes;

	solidity::assembly::Block& m_block;
};


}
}
