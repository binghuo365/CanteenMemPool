#include "CMemPage.h"

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

using namespace CanTeen;

namespace CanTeen
{
	bool num_is_powerof_2(int blockSize)
	{
		return !(blockSize & (blockSize - 1));
	}
	int get_power_of_2(int blockSize)
	{
		int power = 1;
		while(blockSize >> 1)
		{
			power++;
		}
		if(!num_is_powerof_2(blockSize))
		{
			power++;
		}
		return 1 >> power;
	}

	int get_power_of_2_index(int blockSize)
	{
		int power = 1;
		while(blockSize >> 1)
		{
			power++;
		}
		if(!num_is_powerof_2(blockSize))
		{
			power++;
		}
		return power;
	}
	
	class CMemPage
	{
		friend class CPagePool;
	public:
		inline bool isEmpty()
		{
			return _freeIndex == _nodesCount;
		}
		inline bool isFree()
		{
			return _freeIndex > 0;
		}
		inline static CMemPage* mallocPage(int blockSize)
		{
			int size = get_power_of_2(blockSize);
			int pageSize = 0;
			if( size > _PAGE_MIN_SIZE_ )
			{
				pageSize = size + sizeof( CMemPage );
			}
			else
			{
				pageSize = _PAGE_MIN_SIZE_ + sizeof( CMemPage );
			}
			void *buf = ::malloc( pageSize);
			if( NULL == buf )
				return NULL;
			CMemPage memchunk( size , pageSize , buf );
			memcpy( buf , &memchunk , sizeof(memchunk) );
			return (CMemPage*)buf;
		}

		inline static void freePage( CMemPage* page )
		{
			if( page->_freeNodes )
			{
				::free(page->_freeNodes);
				page->_freeNodes = NULL;
			}
			::free( page );
		}

		inline void * mallocNode( int size )
		{
			assert( _blockSize >=  size );
			assert( _freeIndex != 0 );

			return (void*)_freeNodes[ --_freeIndex ];
		}

		inline bool freeNode( void * node )
		{
#ifdef _DEBUG
			int offset = ((char *)node - (char *)this);
			offset -= sizeof( CMemPage );
			if( offset % _blockSize )
			{
				return false;
			}
			else
			{
				_freeNodes[ _freeIndex++ ] = (int)node;
				return true;
			}
#else
			_freeNodes[ _freeIndex++ ] = (int)node;
			return true;
#endif
		}

		inline CMemPage(int blockSize , int pageSize , void * const start)
			:_pageSize(0),_blockSize(0),_freeIndex(0)
		{
			memset( this , 0 , sizeof( CMemPage ) );

			_pageSize = pageSize;

			if( blockSize <= _MIN_BLOCK_SIZE_ )
			{
				_blockSize = _MIN_BLOCK_SIZE_ ;
			}
			else
			{
				if( pageSize - sizeof(CMemPage) > _PAGE_MIN_SIZE_ )
				{
					_blockSize = pageSize - sizeof(CMemPage);
				}
				else
				{
					_blockSize = get_power_of_2(blockSize) ;
				}
			}

			_nodesCount = ( _pageSize - sizeof( CMemPage ) ) / blockSize;
			_freeNodes = (int*)::malloc( _nodesCount * sizeof( int ) );

			for( int i = 0 ; i < _nodesCount ; i++ )
			{
				_freeNodes[i] = (int)((char*)start +
					sizeof( CMemPage ) + i * _blockSize );
			}
			_freeIndex = _nodesCount;
		}
		~CMemPage()
		{
		}
	private:
		int _pageSize;		//page size
		int _blockSize;	    //block size
		int _nodesCount;	    //blocks size
		int _freeIndex;	    //free index
		int *_freeNodes;	    //free nodes
	};
}

CPagePool::CPagePool()
{
	memset( _lastUseIndex , 0 , sizeof( _lastUseIndex ) );
	memset( _usedCount , 0 , sizeof( _usedCount ) );
	memset( _usedPages , 0 , sizeof( _usedPages ) );
	memset( _memPages , 0 , sizeof( _memPages ) );
}
CPagePool::~CPagePool()
{
	for( int i = 0 ; i < _PAGE_COUNT_; i++ )
	{
		if( _memPages[i] )
		{
			CMemPage::freePage( _memPages[i] );
		}
	}
}

void *CPagePool::malloc (int bytes )
{
	if( bytes > _MAX_BLOCK_SIZE_ )
	{
		return NULL;
	}
	if( bytes < _MIN_BLOCK_SIZE_ )
	{
		bytes = _MIN_BLOCK_SIZE_;
	}
	int bit = get_power_of_2_index( bytes );
	int i = 0;
	int lastUseIndex = _lastUseIndex[bit];
	int usedCount = _usedCount[bit];
	for( i = lastUseIndex ; i < usedCount ; i++ )
	{
		if( _usedPages[bit][i]->isFree() )
		{
			_lastUseIndex[bit] = i;
			return _usedPages[bit][i]->mallocNode( bytes );
		}
	}
	for( i = 0 ; i < lastUseIndex ; i++ )
	{
		if( _usedPages[bit][i]->isFree() )
		{
			_lastUseIndex[bit] = i;
			return _usedPages[bit][i]->mallocNode(bytes);
		}
	}
	CMemPage *memPage = CMemPage::mallocPage( bytes );
	if( NULL != memPage )
	{
		_usedPages[bit][ _usedCount[bit] ++ ] = memPage;
		int index = (int)memPage >> _PAGE_MIN_BITS_;
		_memPages[ index ] = memPage;
		return memPage->mallocNode( bytes );
	}
	return NULL;
}
void CPagePool::free (void *ptr)
{
	if( !ptr )
	{
		return;
	}

	int index = (int)ptr >> (_PAGE_MIN_BITS_);
	CMemPage* memPage = _memPages[index];
	if( !memPage || memPage > ptr )
	{
		index -- ;
	}
	memPage = _memPages[index];
	assert( memPage );
	
	if( memPage->freeNode( ptr ) )
	{
		return;
	}
	return;
}
void * CPagePool::realloc( void * ptr, int size )
{
	if( !ptr )
	{
		return NULL;
	}

	int index = (int)ptr >> _PAGE_MIN_BITS_;
	CMemPage* memPage = _memPages[index];
	if( !memPage || memPage > ptr )
	{
		index -- ;
	}
	memPage = _memPages[index];
	assert( memPage );
	if( memPage->_blockSize > (int)size )
	{
		return ptr;
	}

	void * buf = CPagePool::malloc(size);
	if( NULL != buf )
	{
		::memcpy( buf , ptr , memPage->_blockSize );
		CPagePool::free(ptr);
	}
	return buf;
}
