/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_MEMORY_MODEL_H
#define __ASM_MEMORY_MODEL_H

#include <linux/pfn.h>

#ifndef __ASSEMBLY__

/*
 * supports 3 memory models.
 * mkk mem:
 * flat=> 平坦, 从任何 CPU 角度看, 物理地址空间是连续的, 只有一个 Node.
 * discontiguous=> 可以理解为 Node 的平坦模型, 与 NUMA 强相关, 有多个 Node, 每个 Node 的 node_mem_map 对应一个 struct page 的数组, 之后对物理空间个映射.
 * sparse=> 从 Node 为中心转换成了以 section 为中心, 以 struct mem_section 来管理每个 section 的 mem_map, 每个 section 支持内存热拔插.
 */
#if defined(CONFIG_FLATMEM)

#ifndef ARCH_PFN_OFFSET
#define ARCH_PFN_OFFSET		(0UL)
#endif
/* mkk mem: 可以看到这里 page 到 pfn 的转换很容易, 只是一个偏移量的区别, 通过一个 struct page 的数组 mem_map 索引来映射 */
#define __pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))
#define __page_to_pfn(page)	((unsigned long)((page) - mem_map) + \
				 ARCH_PFN_OFFSET)

#ifndef pfn_valid
static inline int pfn_valid(unsigned long pfn)
{
	/* avoid <linux/mm.h> include hell */
	extern unsigned long max_mapnr;
	unsigned long pfn_offset = ARCH_PFN_OFFSET;

	return pfn >= pfn_offset && (pfn - pfn_offset) < max_mapnr;
}
#define pfn_valid pfn_valid
#endif
/* mkk mem: 可以看到稀疏内存模型存在两种实现方式 */
#elif defined(CONFIG_SPARSEMEM_VMEMMAP)

/* memmap is virtually contiguous.  */
#define __pfn_to_page(pfn)	(vmemmap + (pfn))
#define __page_to_pfn(page)	(unsigned long)((page) - vmemmap)

#elif defined(CONFIG_SPARSEMEM)
/*
 * Note: section's mem_map is encoded to reflect its start_pfn.
 * section[i].section_mem_map == mem_map's address - start_pfn;
 */
#define __page_to_pfn(pg)					\
({	const struct page *__pg = (pg);				\
	int __sec = page_to_section(__pg);			\
	(unsigned long)(__pg - __section_mem_map_addr(__nr_to_section(__sec)));	\
})

#define __pfn_to_page(pfn)				\
({	unsigned long __pfn = (pfn);			\
	struct mem_section *__sec = __pfn_to_section(__pfn);	\
	__section_mem_map_addr(__sec) + __pfn;		\
})
#endif /* CONFIG_FLATMEM/SPARSEMEM */

/*
 * Convert a physical address to a Page Frame Number and back
 */
#define	__phys_to_pfn(paddr)	PHYS_PFN(paddr)
#define	__pfn_to_phys(pfn)	PFN_PHYS(pfn)

#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page

#endif /* __ASSEMBLY__ */

#endif
