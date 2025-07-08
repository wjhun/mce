#include <runtime.h>

int main(int argc, char *argv[]) {
	heap h = init_process_runtime();
	rprintf("smoke test\n");
	void *p = allocate(h, 4096);
	rprintf("alloc at %p\n", p);
	return 0;
}
