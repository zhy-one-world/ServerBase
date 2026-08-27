/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:18
	file base:	mem_pool
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _COMMON_MEM_POOL_H_
#define _COMMON_MEM_POOL_H_

#include "singleton.hpp"

namespace faith
{
	class mem_pool : public singleton<mem_pool>
	{
		friend class singleton<mem_pool>;
	public:
		//分配函数返回的最小内存块的长度,如果传入的长度小于该数值,要分配一个MIN_MEM_BLOCK_SIZE的内存块
		static const int MIN_MEM_BLOCK_SIZE = 8;
		//分配函数管理的内存块的最大长度。如果要分配的块大小该数值,则直接用new 与 delete 进行管理
		static const int MAX_MEM_BLOCK_SIZE = 4*1024*1024;

		//分配内存
		void *	alloc(std::size_t size);
		//释放内存
		void	free(void *data,std::size_t size);
	private:
		mem_pool();
	};
}
#endif
