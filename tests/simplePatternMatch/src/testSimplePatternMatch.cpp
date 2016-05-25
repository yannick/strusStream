/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/base/stdint.h"
#include "strus/lib/stream.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/streamPatternMatchInterface.hpp"
#include "strus/streamPatternMatchInstanceInterface.hpp"
#include "strus/streamPatternMatchContextInterface.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <cmath>
#include <cstring>
#include <iomanip>

#undef STRUS_LOWLEVEL_DEBUG
#define RANDINT(MIN,MAX) ((std::rand()%(MAX-MIN))+MIN)

strus::ErrorBufferInterface* g_errorBuffer = 0;

struct DocumentItem
{
	unsigned int pos;
	std::string type;
	std::string value;

	DocumentItem( unsigned int pos_, std::string type_, std::string value_)
		:pos(pos_),type(type_),value(value_){}
	DocumentItem( const DocumentItem& o)
		:pos(o.pos),type(o.type),value(o.value){}
};

struct Document
{
	std::string id;
	std::vector<DocumentItem> itemar;

	explicit Document( const std::string& id_)
		:id(id_){}
	Document( const Document& o)
		:id(o.id),itemar(o.itemar){}
};

static std::string termValue( unsigned int val)
{
	char buf[ 32];
	snprintf( buf, 32, "%u", val);
	return std::string( buf);
}

class ZipfDistribution
{
public:
	explicit ZipfDistribution( std::size_t size)
	{
		std::size_t ii=1, ie=size;
		m_ar.push_back( 1.0);
		for (; ii < ie; ++ii)
		{
			m_ar.push_back( m_ar[ ii-1] + 1.0 / (double)(ii+1));
		}
	}

	unsigned int random() const
	{
		double val = m_ar.back() * (double)std::rand() / (double)RAND_MAX;
		std::size_t ii=0;
		for (; ii<m_ar.size(); ++ii)
		{
			if (val < m_ar[ii])
			{
				break;
			}
			while (m_ar.size() - ii > 100 && val > m_ar[ ii+100])
			{
				ii+=100;
			}
		}
		return ii+1;
	}

private:
	std::vector<double> m_ar;
};


static Document createRandomDocument( unsigned int no, unsigned int size, unsigned int mod)
{
	ZipfDistribution zipfdist( mod);
	Document rt( std::string("doc_") + termValue(no));
	unsigned int ii = 0, ie = size;
	for (; ii < ie; ++ii)
	{
		unsigned int tok = zipfdist.random();
		rt.itemar.push_back( DocumentItem( ii+1, "num", termValue( tok)));
		if (RANDINT(0,12) == 0)
		{
			rt.itemar.push_back( DocumentItem( ii+1, "sent", ""));
		}
	}
	return rt;
}

static void createTermOpRule( strus::StreamPatternMatchInstanceInterface* ptinst, const char* operation, unsigned int range, unsigned int cardinality, unsigned int* param, std::size_t paramsize)
{
	std::string type( "num");
	std::size_t pi = 0, pe = paramsize;
	bool with_delim = false;
	if (std::strcmp( operation, "sequence_struct") == 0
	|| std::strcmp( operation, "inrange_struct") == 0)
	{
		with_delim = true;
	}
	if (with_delim)
	{
		unsigned int delim_termid = ptinst->getTermId( "sent", "");
		ptinst->pushTerm( delim_termid);
		++paramsize;
	}
	for (; pi != pe; ++pi)
	{
		unsigned int termid = ptinst->getTermId( type, termValue(param[pi]));
		ptinst->pushTerm( termid);
		ptinst->attachVariable( std::string("A") + termValue(pi));
	}
	ptinst->pushExpression( operation, paramsize, range, cardinality);
}

static void createTermOpPattern( strus::StreamPatternMatchInstanceInterface* ptinst, const char* operation, unsigned int range, unsigned int cardinality, unsigned int* param, std::size_t paramsize)
{
	std::string rulename = operation;
	std::size_t pi = 0, pe = paramsize;
	for (; pi != pe; ++pi)
	{
		rulename.push_back( '_');
		rulename.append( termValue( param[pi]));
	}
	createTermOpRule( ptinst, operation, range, cardinality, param, paramsize);
	ptinst->closePattern( rulename, true);
}

static void createRules( strus::StreamPatternMatchInstanceInterface* ptinst, const char* joinop, unsigned int nofFeatures, unsigned int nofRules)
{
	ZipfDistribution zipfdist( nofFeatures);
	unsigned int ni=0, ne=nofFeatures;
	for (; ni < ne; ++ni)
	{
		unsigned int range = 5;
		unsigned int cardinality = 0;
		unsigned int param[2];
		param[0] = zipfdist.random();
		param[1] = zipfdist.random();

		if (joinop)
		{
			if (nofRules-- == 0) return;
			createTermOpPattern( ptinst, joinop, range, cardinality, param, 2);
		}
		else
		{
			unsigned int selectOp = RANDINT( 0, 5);
			static const char* opar[] =
			{
				"sequence_struct",
				"sequence",
				"inrange_struct",
				"inrange",
				"any"
			};
			const char* op = opar[ selectOp];
			createTermOpPattern( ptinst, op, range, cardinality, param, 2);
		}
	}
}

static std::vector<Document> createRandomDocuments( unsigned int collSize, unsigned int docSize, unsigned int nofFeatures)
{
	std::vector<Document> rt;
	unsigned int di=0, de=collSize;

	for (di=0; di < de; ++di)
	{
		Document doc( createRandomDocument( di+1, docSize, nofFeatures));
		rt.push_back( doc);
	}
	return rt;
}

static unsigned int getUintValue( const char* arg)
{
	unsigned int rt = 0, prev = 0;
	char const* cc = arg;
	for (; *cc; ++cc)
	{
		if (*cc < '0' || *cc > '9') throw std::runtime_error( std::string( "parameter is not a non negative integer number: ") + arg);
		rt = (rt * 10) + (*cc - '0');
		if (rt < prev) throw std::runtime_error( std::string( "parameter out of range: ") + arg);
	}
	return rt;
}

static std::string doubleToString( double val_)
{
	unsigned int val = (unsigned int)::floor( val_ * 1000);
	unsigned int val_sec = val / 1000;
	unsigned int val_ms = val & 1000;
	std::ostringstream val_str;
	val_str << val_sec << "." << std::setfill('0') << std::setw(3) << val_ms;
	return val_str.str();
}

static unsigned int matchRules( strus::StreamPatternMatchInstanceInterface* ptinst, const Document& doc, strus::StreamPatternMatchContextInterface::Statistics& globalstats)
{
	std::auto_ptr<strus::StreamPatternMatchContextInterface> mt( ptinst->createContext());
	std::vector<DocumentItem>::const_iterator di = doc.itemar.begin(), de = doc.itemar.end();
	unsigned int didx = 0;
	for (; di != de; ++di,++didx)
	{
		unsigned int termid = mt->termId( di->type, di->value);
		if (termid)
		{
			mt->putInput( termid, di->pos, didx, 1);
		}
	}
	std::vector<strus::stream::PatternMatchResult> results = mt->fetchResults();
	unsigned int nofMatches = results.size();

	strus::StreamPatternMatchContextInterface::Statistics stats;
	mt->getStatistics( stats);
	globalstats.nofPatternsTriggered += stats.nofPatternsTriggered;
	globalstats.nofOpenPatterns += stats.nofOpenPatterns;

#ifdef STRUS_LOWLEVEL_DEBUG
	std::vector<strus::stream::PatternMatchResult>::const_iterator
		ri = results.begin(), re = results.end();
	for (; ri != re; ++ri)
	{
		std::cout << "match '" << ri->name() << "':";
		std::vector<strus::stream::PatternMatchResultItem>::const_iterator
			ei = ri->itemlist().begin(), ee = ri->itemlist().end();
	
		for (; ei != ee; ++ei)
		{
			std::cout << " " << ei->name() << " [" << ei->ordpos()
					<< ", " << ei->origpos() << ", " << ei->origsize() << "]";
		}
		std::cout << std::endl;
	}
	std::cout << "nof matches " << nofMatches << ", nof rules triggered " << stats.nofPatternsTriggered << " nof open rules " << doubleToString( stats.nofOpenPatterns) << std::endl;
#endif
	return nofMatches;
}

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <features> <nofdocs> <docsize> [<joinop>]" << std::endl;
	std::cerr << "<features>= number of distinct features" << std::endl;
	std::cerr << "<nofdocs> = number of documents to insert" << std::endl;
	std::cerr << "<docsize> = size of a document" << std::endl;
	std::cerr << "<nofpatterns> = number of patterns to use" << std::endl;
	std::cerr << "<joinop> = operator to use for patterns (default all)" << std::endl;
}

int main( int argc, const char** argv)
{
	try
	{
		g_errorBuffer = strus::createErrorBuffer_standard( 0, 1);
		if (!g_errorBuffer)
		{
			std::cerr << "construction of error buffer failed" << std::endl;
			return -1;
		}
		if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
		{
			printUsage( argc, argv);
			return 0;
		}
		else if (argc < 5)
		{
			std::cerr << "ERROR too few arguments" << std::endl;
			printUsage( argc, argv);
			return 1;
		}
		else if (argc > 6)
		{
			std::cerr << "ERROR too many arguments" << std::endl;
			printUsage( argc, argv);
			return 1;
		}
		unsigned int nofFeatures = getUintValue( argv[1]);
		unsigned int nofDocuments = getUintValue( argv[2]);
		unsigned int documentSize = getUintValue( argv[3]);
		unsigned int nofPatterns = getUintValue( argv[4]);
		const char* joinop = (argc > 5)?argv[5]:0;

		std::auto_ptr<strus::StreamPatternMatchInterface> pt( strus::createStreamPatternMatch_standard( g_errorBuffer));
		if (!pt.get()) throw std::runtime_error("failed to create pattern matcher");
		std::auto_ptr<strus::StreamPatternMatchInstanceInterface> ptinst( pt->createInstance());
		if (!ptinst.get()) throw std::runtime_error("failed to create pattern matcher instance");
		createRules( ptinst.get(), joinop, nofFeatures, nofPatterns);

		if (g_errorBuffer->hasError())
		{
			throw std::runtime_error( "error creating automaton for evaluating rules");
		}
		std::vector<Document> docs = createRandomDocuments( nofDocuments, documentSize, nofFeatures);
		unsigned int totalNofmatches = 0;
		std::cerr << "starting rule evaluation ..." << std::endl;

		std::clock_t start = std::clock();
		strus::StreamPatternMatchContextInterface::Statistics stats;
		std::vector<Document>::const_iterator di = docs.begin(), de = docs.end();
		for (; di != de; ++di)
		{
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "document " << di->id << ":" << std::endl;
#endif
			unsigned int nofmatches = matchRules( ptinst.get(), *di, stats);
			totalNofmatches += nofmatches;
			if (g_errorBuffer->hasError())
			{
				throw std::runtime_error("error matching rule");
			}
		}
		double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
		if (g_errorBuffer->hasError())
		{
			throw std::runtime_error("uncaugth exception");
		}
		std::cerr << "OK" << std::endl;
		std::cerr << "processed " << docs.size() << " documents with " << nofPatterns << " patterns, total " << totalNofmatches << " matches, " << stats.nofPatternsTriggered << " patterns triggered " << (unsigned int)(stats.nofOpenPatterns/docs.size()+0.5) << " open patterns in average in " << doubleToString(duration) << " seconds" << std::endl;
		delete g_errorBuffer;
		return 0;
	}
	catch (const std::runtime_error& err)
	{
		if (g_errorBuffer->hasError())
		{
			std::cerr << "error processing pattern matching: "
					<< g_errorBuffer->fetchError() << " (" << err.what()
					<< ")" << std::endl;
		}
		else
		{
			std::cerr << "error processing pattern matching: "
					<< err.what() << std::endl;
		}
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << "out of memory processing pattern matching" << std::endl;
	}
	delete g_errorBuffer;
	return -1;
}

