#define NGPUS 10
struct gpu_info {
...
};
void gi_init(struct gpu_info *gi);
void gi_alloc(struct gpu_info *gi,
unsigned int ngpus,
/* OUT */ unsigned int gpus[]);
void gi_release(struct gpu_info *gi,
unsigned int ngpus,
/* IN */ unsigned int gpus[]);
void gi_free(struct gpu_info *gi);
