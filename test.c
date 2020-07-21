#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "/home/martin/msr-safe/msr_safe.h" 
#if 0
int main() 
{
	int fd;	// this is the file descriptor
	uint64_t energy[1000], mperf[1000];
	ssize_t rc;
	int i;

	fd = open("/dev/cpu/0/msr_safe", O_RDWR);
	//assert(fd > -1);

	for( i=0; i<1000; i++) {
		//rc = pread(fd, &energy[i], 8, 0x611);
		//assert(rc);
		
		rc = pread(fd, &mperf[i], 8, 0xE7);	
		//assert(rc);

	}
	for( i=0; i<1000; i++) {
		printf("%" PRIu64 " %" PRIu64 "\n", energy[i], mperf[i]);
	}
	exit(0);
}
#endif

struct msr_batch_array batch;
struct msr_batch_op op[1000*2];

int main(){
	int i;
	batch.numops = 1000*2;
	batch.ops = op;	
	int rc;
	int fd = open( "/dev/cpu/msr_batch", O_RDWR );
	assert( fd != -1 );

	for( i=0; i<1000*2; i=i+2 ){
		batch.ops[i].cpu = 0;	
		batch.ops[i].isrdmsr = 1;	
		batch.ops[i].err = 0;	
		batch.ops[i].msr = 0x611;	
		batch.ops[i].msrdata = 0;	
		batch.ops[i].wmask = 0;	

		batch.ops[i+1].cpu = 0;	
		batch.ops[i+1].isrdmsr = 1;	
		batch.ops[i+1].err = 0;	
		batch.ops[i+1].msr = 0xE7;	
		batch.ops[i+1].msrdata = 0;	
		batch.ops[i+1].wmask = 0;	
	}
	rc = ioctl( fd, X86_IOC_MSR_BATCH, &batch );
	assert( rc != -1 );

	for( i=0; i<1000*2; i=i+2 ){
		fprintf(stdout, "%" PRIu64 " %" PRIu64 "\n", 
			(uint64_t)batch.ops[i].msrdata,
			(uint64_t)batch.ops[i+1].msrdata);
	}
	return 0;
}

/*
struct msr_batch_op
{
    __u16 cpu;     // In: CPU to execute {rd/wr}msr instruction
    __u16 isrdmsr; // In: 0=wrmsr, non-zero=rdmsr
    __s32 err;     // Out: set if error occurred with this operation
    __u32 msr;     // In: MSR Address to perform operation
    __u64 msrdata; // In/Out: Input/Result to/from operation
    __u64 wmask;   // Out: Write mask applied to wrmsr
};

struct msr_batch_array
{
    __u32 numops;             // In: # of operations in operations array
    struct msr_batch_op *ops; // In: Array[numops] of operations
};

#define X86_IOC_MSR_BATCH   _IOWR('c', 0xA2, struct msr_batch_array)

*/
