#include <cassert>

#include "mmex.h"

MMEX::MMEX( quint64 capacity , QObject *parent)
	: QThread(parent)
{
	assert( capacity > 0 ) ; 
	this->mMemCapacity = 0 ;

	//
	this->mMemCapacity = capacity ;
	mMaxBlock= 30 ;
	MPT mpt ;
	mpt.busy = false ;
	mpt.SP = 0;
	mpt.CP = 0;
}

MMEX::~MMEX()
{

}

void MMEX::run() 
{

}

quint64 MMEX::malloc( /*quint64 size*/ )
{
	quint64 malloced_size = 0;
	this->mAtomMutex.lock();

	int p1 = -1 , p2 = -1 ;

	int at = -1;
	for( int i = 0 ; i < this->mPointerMap.size() ; i ++ )
	{
		MPT mpt = this->mPointerMap.at( i ) ;
		if( mpt.busy == false )
		{
			at = i ;
			break ;
		}
	}
	//找第一个点
	

	this->mAtomMutex.unlock();
	return malloced_size ;
}

quint64 MMEX::free( quint64 SP ,  quint64 size  )
{
	quint64 freed_size = 0;
	this->mAtomMutex.lock();



	this->mAtomMutex.unlock();
	return freed_size ;
}

