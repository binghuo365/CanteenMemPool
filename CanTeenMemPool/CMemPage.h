#define _PAGE_INDEX_COUNT_		(20+7)												//page�������� �����ɷ���1 << 27 block ��page
#define _PAGE_MIN_BITS_			(20)														//the min mem block
#define _PAGE_MIN_SIZE_			(1 << _PAGE_MIN_BITS_)						//page�����size
#define _MIN_BLOCK_SIZE_		(8)																//the min block size
#define _MAX_BLOCK_SIZE_		(1 << (_PAGE_INDEX_COUNT_  -  1 ) )		//the max block size
#define _PAGE_COUNT_			(4 * 1024)													//�ڴ�����page��

#include <memory.h>
#include <stdlib.h>
#include <assert.h>

size_t get_power_of_2(size_t i)
{
	if (is_power_of_2(i))
		return i;
	size_t bit = 0, out = 0, tmp = i;
	do
	{
		bit++;
	} while ((tmp >>= 1));
	out = (1 << bit);
	return out;
}

size_t get_power_of_2_index(const size_t i)
{
	size_t bit = 0, tmp = i;
	do
	{
		bit++;
	} while ((tmp >>= 1));
	if (is_power_of_2(i))
	{
		return --bit;
	}
	return bit;
}

bool is_power_of_2(size_t i)
{
	return !(i & (i - 1));
}

class CMemPage
{
public:
	CMemPage(size_t blockSize, size_t pageSize, void * const start)
	{
		memset(this, 0, sizeof(CMemPage));
		_pageSize = pageSize;
		if (blockSize <= _MIN_BLOCK_SIZE_)
		{
			blockSize = _MIN_BLOCK_SIZE_;
		}
		else
		{
			if (pageSize - sizeof(CMemPage) > _PAGE_MIN_SIZE_)
			{
				_blockSize = pageSize - sizeof(CMemPage);
			}
			else
			{
				_blockSize = get_power_of_2(blockSize);
			}
		}
		_nodesCount = (_pageSize - sizeof(CMemPage)) / _blockSize;
		_freeNodes = (size_t *)malloc(_nodesCount * _blockSize);
		for (int i = 0; i < _nodesCount; ++i)
		{
			_freeNodes[i] = (size_t)((char*)start + sizeof(CMemPage)+i * _blockSize);
		}
		_freeIndex = _nodesCount;
	}
	~CMemPage(){}
public:
	void* mallocNode(size_t t)
	{
		assert(_blockSize >= t);
		assert(_freeIndex >= 0);
		return (void*)_freeNodes[_freeIndex--];
	}
	bool freeNode(void * node)
	{
		size_t offset = (char*)node - (char*)this - sizeof(CMemPage);
		if (offset % _blockSize)
		{
			return false;
		}
		_freeNodes[_freeIndex++] = (size_t)node;
	}

	void* mallocPage(size_t blockSize)
	{
		size_t  power2Size = get_power_of_2(blockSize);
		size_t pageSize = 0;
		if (power2Size > _PAGE_MIN_SIZE_)
		{
			pageSize = power2Size + sizeof(CMemPage);
		}
		else
		{
			pageSize = _PAGE_MIN_SIZE_ + sizeof(CMemPage);
		}
		void* buf = ::malloc(pageSize);
		if (NULL == buf)
		{
			return NULL; 
		}
		CMemPage page(power2Size, pageSize, buf);
		memcpy(buf, &page, sizeof(page));
		return (CMemPage*)buf; 
	}

	bool freePage(void *)
	{

	}

private:
	size_t _blockSize;				//block�Ĵ�С
	size_t _pageSize;				//page��С
	size_t _nodesCount;			//���ٸ�block��Nodes��
	size_t _freeIndex;				//�����λ��ʼ���䣬�ڼ���node�ǿ��е�
	size_t *_freeNodes;			 //����node��Ӧ�ĵ�ַ
};

