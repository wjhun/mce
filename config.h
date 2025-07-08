/* maximum buckets that can fit within a PAGESIZE_2M mcache */
#define TABLE_MAX_BUCKETS 131072

/* locking */
#define MUTEX_ACQUIRE_SPIN_LIMIT (1ull << 20)

/* could probably find progammatically via cpuid... */
#define DEFAULT_CACHELINE_SIZE 64

#define SG_FRAG_BYTE_THRESHOLD (128*KB)
