# Evidence: SYZBOT-417aeb05fd190f3a  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __unmap_hugepage_range / alloc_surplus_hugetlb_folio
- mainline fix commit: `21cc2b5c5062a256ae9064442d37ebbc23f5aef7`
- candidate JSON: `A_SYZBOT-417aeb05fd190f3a.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__unmap_hugepage_range", "fn_b": "alloc_surplus_hugetlb_folio"}`

## Commit message

```
21cc2b5c5062a256ae9064442d37ebbc23f5aef7
mm/hugetlb: add missing hugetlb_lock in __unmap_hugepage_range()

When restoring a reservation for an anonymous page, we need to check to
freeing a surplus.  However, __unmap_hugepage_range() causes data race
because it reads h->surplus_huge_pages without the protection of
hugetlb_lock.

And adjust_reservation is a boolean variable that indicates whether
reservations for anonymous pages in each folio should be restored. 
Therefore, it should be initialized to false for each round of the loop. 
However, this variable is not initialized to false except when defining
the current adjust_reservation variable.

This means that once adjust_reservation is set to true even once within
the loop, reservations for anonymous pages will be restored
unconditionally in all subsequent rounds, regardless of the folio's state.

To fix this, we need to add the missing hugetlb_lock, unlock the
page_table_lock earlier so that we don't lock the hugetlb_lock inside the
page_table_lock lock, and initialize adjust_reservation to false on each
round within the loop.

Link: https://lkml.kernel.org/r/20250823182115.1193563-1-aha310510@gmail.com
Fixes: df7a6d1f6405 ("mm/hugetlb: restore the reservation if needed")
Signed-off-by: Jeongjun Park <aha310510@gmail.com>
Reported-by: syzbot+417aeb05fd190f3a6da9@syzkaller.appspotmail.com
Closes: https://syzkaller.appspot.com/bug?extid=417aeb05fd190f3a6da9
Reviewed-by: Sidhartha Kumar <sidhartha.kumar@oracle.com>
Cc: Breno Leitao <leitao@debian.org>
Cc: David Hildenbrand <david@redhat.com>
Cc: Muchun Song <muchun.song@linux.dev>
Cc: Oscar Salvador <osalvador@suse.de>
Cc: <stable@vger.kernel.org>
Signed-off-by: Andrew Morton <akpm@linux-foundation.org>
```

## CVE / bug description

```
KCSAN: data-race in __unmap_hugepage_range / alloc_surplus_hugetlb_folio
```

## Full mainline patch

```diff
commit 21cc2b5c5062a256ae9064442d37ebbc23f5aef7
Author: Jeongjun Park <aha310510@gmail.com>
Date:   Sun Aug 24 03:21:15 2025 +0900

    mm/hugetlb: add missing hugetlb_lock in __unmap_hugepage_range()
    
    When restoring a reservation for an anonymous page, we need to check to
    freeing a surplus.  However, __unmap_hugepage_range() causes data race
    because it reads h->surplus_huge_pages without the protection of
    hugetlb_lock.
    
    And adjust_reservation is a boolean variable that indicates whether
    reservations for anonymous pages in each folio should be restored.
    Therefore, it should be initialized to false for each round of the loop.
    However, this variable is not initialized to false except when defining
    the current adjust_reservation variable.
    
    This means that once adjust_reservation is set to true even once within
    the loop, reservations for anonymous pages will be restored
    unconditionally in all subsequent rounds, regardless of the folio's state.
    
    To fix this, we need to add the missing hugetlb_lock, unlock the
    page_table_lock earlier so that we don't lock the hugetlb_lock inside the
    page_table_lock lock, and initialize adjust_reservation to false on each
    round within the loop.
    
    Link: https://lkml.kernel.org/r/20250823182115.1193563-1-aha310510@gmail.com
    Fixes: df7a6d1f6405 ("mm/hugetlb: restore the reservation if needed")
    Signed-off-by: Jeongjun Park <aha310510@gmail.com>
    Reported-by: syzbot+417aeb05fd190f3a6da9@syzkaller.appspotmail.com
    Closes: https://syzkaller.appspot.com/bug?extid=417aeb05fd190f3a6da9
    Reviewed-by: Sidhartha Kumar <sidhartha.kumar@oracle.com>
    Cc: Breno Leitao <leitao@debian.org>
    Cc: David Hildenbrand <david@redhat.com>
    Cc: Muchun Song <muchun.song@linux.dev>
    Cc: Oscar Salvador <osalvador@suse.de>
    Cc: <stable@vger.kernel.org>
    Signed-off-by: Andrew Morton <akpm@linux-foundation.org>

diff --git a/mm/hugetlb.c b/mm/hugetlb.c
index 753f99b4c718..eed59cfb5d21 100644
--- a/mm/hugetlb.c
+++ b/mm/hugetlb.c
@@ -5851,7 +5851,7 @@ void __unmap_hugepage_range(struct mmu_gather *tlb, struct vm_area_struct *vma,
 	spinlock_t *ptl;
 	struct hstate *h = hstate_vma(vma);
 	unsigned long sz = huge_page_size(h);
-	bool adjust_reservation = false;
+	bool adjust_reservation;
 	unsigned long last_addr_mask;
 	bool force_flush = false;
 
@@ -5944,6 +5944,7 @@ void __unmap_hugepage_range(struct mmu_gather *tlb, struct vm_area_struct *vma,
 					sz);
 		hugetlb_count_sub(pages_per_huge_page(h), mm);
 		hugetlb_remove_rmap(folio);
+		spin_unlock(ptl);
 
 		/*
 		 * Restore the reservation for anonymous page, otherwise the
@@ -5951,14 +5952,16 @@ void __unmap_hugepage_range(struct mmu_gather *tlb, struct vm_area_struct *vma,
 		 * If there we are freeing a surplus, do not set the restore
 		 * reservation bit.
 		 */
+		adjust_reservation = false;
+
+		spin_lock_irq(&hugetlb_lock);
 		if (!h->surplus_huge_pages && __vma_private_lock(vma) &&
 		    folio_test_anon(folio)) {
 			folio_set_hugetlb_restore_reserve(folio);
 			/* Reservation to be adjusted after the spin lock */
 			adjust_reservation = true;
 		}
-
-		spin_unlock(ptl);
+		spin_unlock_irq(&hugetlb_lock);
 
 		/*
 		 * Adjust the reservation for the region that will have the

```

## File: `mm/hugetlb.c`

- changed old-lines: [5854, 5960, 5961]

### Function `__unmap_hugepage_range` (L5842–L6010, 169 lines)

```c
void __unmap_hugepage_range(struct mmu_gather *tlb, struct vm_area_struct *vma,
			    unsigned long start, unsigned long end,
			    struct folio *folio, zap_flags_t zap_flags)
{
	struct mm_struct *mm = vma->vm_mm;
	const bool folio_provided = !!folio;
	unsigned long address;
	pte_t *ptep;
	pte_t pte;
	spinlock_t *ptl;
	struct hstate *h = hstate_vma(vma);
	unsigned long sz = huge_page_size(h);
	bool adjust_reservation = false;
	unsigned long last_addr_mask;
	bool force_flush = false;

	WARN_ON(!is_vm_hugetlb_page(vma));
	BUG_ON(start & ~huge_page_mask(h));
	BUG_ON(end & ~huge_page_mask(h));

	/*
	 * This is a hugetlb vma, all the pte entries should point
	 * to huge page.
	 */
	tlb_change_page_size(tlb, sz);
	tlb_start_vma(tlb, vma);

	last_addr_mask = hugetlb_mask_last_page(h);
	address = start;
	for (; address < end; address += sz) {
		ptep = hugetlb_walk(vma, address, sz);
		if (!ptep) {
			address |= last_addr_mask;
			continue;
		}

		ptl = huge_pte_lock(h, mm, ptep);
		if (huge_pmd_unshare(mm, vma, address, ptep)) {
			spin_unlock(ptl);
			tlb_flush_pmd_range(tlb, address & PUD_MASK, PUD_SIZE);
			force_flush = true;
			address |= last_addr_mask;
			continue;
		}

		pte = huge_ptep_get(mm, address, ptep);
		if (huge_pte_none(pte)) {
			spin_unlock(ptl);
			continue;
		}

		/*
		 * Migrating hugepage or HWPoisoned hugepage is already
		 * unmapped and its refcount is dropped, so just clear pte here.
		 */
		if (unlikely(!pte_present(pte))) {
			/*
			 * If the pte was wr-protected by uffd-wp in any of the
			 * swap forms, meanwhile the caller does not want to
			 * drop the uffd-wp bit in this zap, then replace the
			 * pte with a marker.
			 */
			if (pte_swp_uffd_wp_any(pte) &&
			    !(zap_flags & ZAP_FLAG_DROP_MARKER))
				set_huge_pte_at(mm, address, ptep,
						make_pte_marker(PTE_MARKER_UFFD_WP),
						sz);
			else
				huge_pte_clear(mm, address, ptep, sz);
			spin_unlock(ptl);
			continue;
		}

		/*
		 * If a folio is supplied, it is because a specific
		 * folio is being unmapped, not a range. Ensure the folio we
		 * are about to unmap is the actual folio of interest.
		 */
		if (folio_provided) {
			if (folio != page_folio(pte_page(pte))) {
				spin_unlock(ptl);
				continue;
			}
			/*
			 * Mark the VMA as having unmapped its page so that
			 * future faults in this VMA will fail rather than
			 * looking like data was lost
			 */
			set_vma_resv_flags(vma, HPAGE_RESV_UNMAPPED);
		} else {
			folio = page_folio(pte_page(pte));
		}

		pte = huge_ptep_get_and_clear(mm, address, ptep, sz);
		tlb_remove_huge_tlb_entry(h, tlb, ptep, address);
		if (huge_pte_dirty(pte))
			folio_mark_dirty(folio);
		/* Leave a uffd-wp pte marker if needed */
		if (huge_pte_uffd_wp(pte) &&
		    !(zap_flags & ZAP_FLAG_DROP_MARKER))
			set_huge_pte_at(mm, address, ptep,
					make_pte_marker(PTE_MARKER_UFFD_WP),
					sz);
		hugetlb_count_sub(pages_per_huge_page(h), mm);
		hugetlb_remove_rmap(folio);

		/*
		 * Restore the reservation for anonymous page, otherwise the
		 * backing page could be stolen by someone.
		 * If there we are freeing a surplus, do not set the restore
		 * reservation bit.
		 */
		if (!h->surplus_huge_pages && __vma_private_lock(vma) &&
		    folio_test_anon(folio)) {
			folio_set_hugetlb_restore_reserve(folio);
			/* Reservation to be adjusted after the spin lock */
			adjust_reservation = true;
		}

		spin_unlock(ptl);

		/*
		 * Adjust the reservation for the region that will have the
		 * reserve restored. Keep in mind that vma_needs_reservation() changes
		 * resv->adds_in_progress if it succeeds. If this is not done,
		 * do_exit() will not see it, and will keep the reservation
		 * forever.
		 */
		if (adjust_reservation) {
			int rc = vma_needs_reservation(h, vma, address);

			if (rc < 0)
				/* Pressumably allocate_file_region_entries failed
				 * to allocate a file_region struct. Clear
				 * hugetlb_restore_reserve so that global reserve
				 * count will not be incremented by free_huge_folio.
				 * Act as if we consumed the reservation.
				 */
				folio_clear_hugetlb_restore_reserve(folio);
			else if (rc)
				vma_add_reservation(h, vma, address);
		}

		tlb_remove_page_size(tlb, folio_page(folio, 0),
				     folio_size(folio));
		/*
		 * If we were instructed to unmap a specific folio, we're done.
		 */
		if (folio_provided)
			break;
	}
	tlb_end_vma(tlb, vma);

	/*
	 * If we unshared PMDs, the TLB flush was not recorded in mmu_gather. We
	 * could defer the flush until now, since by holding i_mmap_rwsem we
	 * guaranteed that the last refernece would not be dropped. But we must
	 * do the flushing before we return, as otherwise i_mmap_rwsem will be
	 * dropped and the last reference to the shared PMDs page might be
	 * dropped as well.
	 *
	 * In theory we could defer the freeing of the PMD pages as well, but
	 * huge_pmd_unshare() relies on the exact page_count for the PMD page to
	 * detect sharing, so we cannot defer the release of the page either.
	 * Instead, do flush now.
	 */
	if (force_flush)
		tlb_flush_mmu_tlbonly(tlb);
}
```

**External callers of `__unmap_hugepage_range` at parent**

- `include/linux/hugetlb.h:134` — `void __unmap_hugepage_range(struct mmu_gather *tlb,`
- `include/linux/hugetlb.h:445` — `static inline void __unmap_hugepage_range(struct mmu_gather *tlb,`
- `mm/memory.c:1928` — `				__unmap_hugepage_range(tlb, vma, start, end,`
