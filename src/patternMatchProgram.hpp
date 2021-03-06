/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief StrusStream program implementation for loading pattern definitions from source
/// \file "patternMatchProgram.hpp"
#ifndef _STRUS_STREAM_PATTERN_MATCH_PROGRAM_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_STREAM_PATTERN_MATCH_PROGRAM_IMPLEMENTATION_HPP_INCLUDED
#include "strus/reference.hpp"
#include "strus/patternMatchProgramInterface.hpp"
#include "strus/patternMatchProgramInstanceInterface.hpp"
#include "strus/tokenPatternMatchInstanceInterface.hpp"
#include "strus/tokenPatternMatchInterface.hpp"
#include "strus/stream/charRegexMatchOptions.hpp"
#include "strus/stream/tokenPatternMatchOptions.hpp"
#include "strus/charRegexMatchInstanceInterface.hpp"
#include "strus/charRegexMatchInterface.hpp"
#include "symbolTable.hpp"
#include <string>
#include <vector>
#include <set>
#include <cstddef>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief StrusStream program loader for loading pattern definitions from source
class PatternMatchProgramInstance
	:public PatternMatchProgramInstanceInterface
{
public:
	/// \brief Constructor
	PatternMatchProgramInstance(
			const TokenPatternMatchInterface* tpm,
			const CharRegexMatchInterface* crm,
			ErrorBufferInterface* errorhnd_);

	/// \brief Destructor
	virtual ~PatternMatchProgramInstance(){}

	virtual bool load( const std::string& source);
	virtual bool compile();

	virtual const CharRegexMatchInstanceInterface* getCharRegexMatchInstance() const;
	virtual const TokenPatternMatchInstanceInterface* getTokenPatternMatchInstance() const;

	virtual const char* tokenName( unsigned int id) const
	{
		return m_regexNameSymbolTab.key( id);
	}

private:
	void loadOption( char const*& si);
	void loadExpression( char const*& si);
	void loadExpressionNode( const std::string& name, char const*& si);

private:
	ErrorBufferInterface* m_errorhnd;
	std::vector<std::string> m_tokenPatternMatchOptionNames;
	std::vector<std::string> m_charRegexMatchOptionNames;
	stream::TokenPatternMatchOptions m_tokenPatternMatchOptions;
	stream::CharRegexMatchOptions m_charRegexMatchOptions;
	Reference<TokenPatternMatchInstanceInterface> m_tokenPatternMatch;
	Reference<CharRegexMatchInstanceInterface> m_charRegexMatch;
	SymbolTable m_regexNameSymbolTab;
	SymbolTable m_patternNameSymbolTab;
	std::set<uint32_t> m_unresolvedPatternNameSet;
};


/// \brief StrusStream interface to load pattern definitions from source
class PatternMatchProgram
	:public PatternMatchProgramInterface
{
public:
	PatternMatchProgram( const TokenPatternMatchInterface* tpm_, const CharRegexMatchInterface* crm_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_tpm(tpm_),m_crm(crm_){}

	virtual ~PatternMatchProgram(){}

	virtual PatternMatchProgramInstanceInterface* createInstance() const;

private:
	ErrorBufferInterface* m_errorhnd;
	const TokenPatternMatchInterface* m_tpm;
	const CharRegexMatchInterface* m_crm;
};




} //namespace
#endif

