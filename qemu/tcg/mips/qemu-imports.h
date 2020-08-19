#ifndef __QEMU_IMPORTS_H__
#define __QEMU_IMPORTS_H__
typedef uint32_t TCGMemOpIdx;
typedef TCGMemOp MemOp;
#   define  MO_ASHIFT   4
#   define  MO_AMASK    (7 << MO_ASHIFT)

#ifdef NEED_CPU_H
#ifdef TARGET_ALIGNED_ONLY
#   define  MO_ALIGN    0
#   define  MO_UNALN    MO_AMASK
#else
#   define  MO_ALIGN    MO_AMASK
#   define  MO_UNALN    0
#endif
#endif

/* This will be used by TCG backends to compute offsets.  */
#define TLB_MASK_TABLE_OFS(IDX) \
    ((int)offsetof(ArchCPU, neg.tlb.f[IDX]) - (int)offsetof(ArchCPU, env))

#define TARGET_PAGE_BITS_MIN TARGET_PAGE_BITS
#define TLB_WATCHPOINT      (1 << (TARGET_PAGE_BITS_MIN - 4))
#define TLB_BSWAP           (1 << (TARGET_PAGE_BITS_MIN - 5))
#define TLB_DISCARD_WRITE   (1 << (TARGET_PAGE_BITS_MIN - 6))

#define TLB_FLAGS_MASK \
    (TLB_INVALID_MASK | TLB_NOTDIRTY | TLB_MMIO \
    | TLB_WATCHPOINT | TLB_BSWAP | TLB_DISCARD_WRITE)

/**
 *  * arg_label
 *   * @i: value
 *    *
 *     * The opposite of label_arg.  Retrieve a label from the
 *      * encoding of the TCG opcode stream.
 *       */

static inline TCGLabel *arg_label(TCGContext *s, TCGArg i)
{
    TCGLabel *l = &s->labels[i];
    l->label_index = i;
    return l;
}

/**
 *  * make_memop_idx
 *   * @op: memory operation
 *    * @idx: mmu index
 *     *
 *      * Encode these values into a single parameter.
 *       */
static inline TCGMemOpIdx make_memop_idx(MemOp op, unsigned idx)
{
    tcg_debug_assert(idx <= 15);
    return (op << 4) | idx;
}

/**
 *  * get_memop
 *   * @oi: combined op/idx parameter
 *    *
 *     * Extract the memory operation from the combined value.
 *      */
static inline MemOp get_memop(TCGMemOpIdx oi)
{
    return oi >> 4;
}

/**
 *  * get_mmuidx
 *   * @oi: combined op/idx parameter
 *    *
 *     * Extract the mmu index from the combined value.
 *      */
static inline unsigned get_mmuidx(TCGMemOpIdx oi)
{
    return oi & 15;
}

/**
 *  * get_alignment_bits
 *   * @memop: MemOp value
 *    *
 *     * Extract the alignment size from the memop.
 *      */
static inline unsigned get_alignment_bits(MemOp memop)
{
    unsigned a = memop & MO_AMASK;

    if (a == MO_UNALN) {
        /* No alignment required.  */
        a = 0;
    } else if (a == MO_ALIGN) {
        /* A natural alignment requirement.  */
        a = memop & MO_SIZE;
    } else {
        /* A specific alignment requirement.  */
        a = a >> MO_ASHIFT;
    }
#if defined(CONFIG_SOFTMMU)
    /* The requested alignment cannot overlap the TLB flags.  */
    tcg_debug_assert((TLB_FLAGS_MASK & ((1 << a) - 1)) == 0);
#endif
    return a;
}
#endif
