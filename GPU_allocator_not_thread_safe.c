#include <string.h>
#include <assert.h>
struct gpu_info {
int allocated[NGPUS];
unsigned int nfree;
};
void gi_init(struct gpu_info *gi) {
memset(gi->allocated, 0, sizeof(gi->allocated));
gi->nfree = NGPUS;
}
void gi_alloc(struct gpu_info *gi,
unsigned int ngpus,
/* OUT */ unsigned int gpus[]) {
assert(ngpus <= gi->nfree);
gi->nfree -= ngpus;
unsigned int g = 0;
for (unsigned int i = 0; g < ngpus; i++) {
assert(i < NGPUS);
if (!gi->allocated[i]) {
gi->allocated[i] = 1;
gpus[g++] = i;
}
}
}
void gi_release(struct gpu_info *gi,
unsigned int ngpus,
/* IN */ unsigned int gpus[]) {
for (unsigned int g = 0; g < ngpus; g++) {
assert(gpus[g] < NGPUS);
assert(gi->allocated[gpus[g]]);
gi->allocated[gpus[g]] = 0;
}
gi->nfree += ngpus;
assert(gi->nfree <= NGPUS);
}
void gi_free(struct gpu_info *gi) {
}
