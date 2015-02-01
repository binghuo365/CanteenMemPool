#ifndef _CMemPage_H_
#define _CMemPage_H_


#define _SYSTEM_MEMSIZE_		(4 * 1024 * 1024 * 1024)			//system mem size
#define _PAGE_INDEX_COUNT_		(20+7)								//the page index size max memory size 64M
#define _MAX_MEM_PTR_			(3 * 1024 * 1024 * 1024)			//the mem ptr max
#define _PAGE_MIN_SIZE_			(1024 * 1024)						//the min page size
#define _PAGE_MIN_BITS_			(20)								//the min mem block
#define _MIN_BLOCK_SIZE_		(8)								    //the min block size
#define _MAX_BLOCK_SIZE_		(1 << (_PAGE_INDEX_COUNT_  -  1 ) )	//the max block size
#define _PAGE_COUNT_			(4 * 1024)							//the totol page size

namespace CanTeen
{
	class CMemPage;
	class CPagePool
	{
	public:
		CPagePool();
		~CPagePool();
		void *malloc (int nbytes);
		void free (void *ptr);
		void *realloc( void * pTemp , int nSize );
	private:

		int _lastUseIndex[_PAGE_INDEX_COUNT_];

		int _usedCount[_PAGE_INDEX_COUNT_];

		CMemPage* _usedPages[_PAGE_INDEX_COUNT_][_PAGE_COUNT_];

		CMemPage* _memPages[_PAGE_COUNT_];

	};
}


#endif