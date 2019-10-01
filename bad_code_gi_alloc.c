void gi_alloc(struct gpu_info *gi,
unsigned int ngpus,
/* OUT */ unsigned int gpus[]) {
for (int i = 0; i < ngpus; i++)
rthread_sema_procure(&gi->sema); //need to add locks in here, make sure procure is in a critical section 
...
}