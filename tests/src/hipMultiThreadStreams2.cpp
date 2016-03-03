#include<iostream>
#include"test_common.h"
#include<thread>
#define N 1000

template<typename T>
__global__ void Inc(hipLaunchParm lp, T *Array){
int tx = hipThreadIdx_x + hipBlockIdx_x * hipBlockDim_x;
Array[tx] = Array[tx] + T(1);
}

void run1(size_t size, hipStream_t stream){
	float *Ah, *Bh, *Cd, *Dd, *Eh;

	hipMallocHost(&Ah, size);
	hipMallocHost(&Bh, size);
	hipMalloc(&Cd, size);
	hipMalloc(&Dd, size);
	hipMallocHost(&Eh, size);

	for(int i=0;i<N;i++){
		Ah[i] = 1.0f;
	}

	hipMemcpyAsync(Bh, Ah, size, hipMemcpyHostToHost, stream);
	hipMemcpyAsync(Cd, Bh, size, hipMemcpyHostToDevice, stream);
	hipLaunchKernel(HIP_KERNEL_NAME(Inc), dim3(N/500), dim3(500), 0, stream, Cd);
	hipMemcpyAsync(Dd, Cd, size, hipMemcpyDeviceToDevice, stream);
	hipMemcpyAsync(Eh, Dd, size, hipMemcpyDeviceToHost, stream);
	HIPCHECK(hipDeviceSynchronize());
	HIPASSERT(Eh[10] == Ah[10] + 1.0f);
}


void run(size_t size, hipStream_t stream1, hipStream_t stream2){
	float *Ah, *Bh, *Cd, *Dd, *Eh;
	float *Ahh, *Bhh, *Cdd, *Ddd, *Ehh;

	hipMallocHost(&Ah, size);
	hipMallocHost(&Bh, size);
	hipMalloc(&Cd, size);
	hipMalloc(&Dd, size);
	hipMallocHost(&Eh, size);
	hipMallocHost(&Ahh, size);
	hipMallocHost(&Bhh, size);
	hipMalloc(&Cdd, size);
	hipMalloc(&Ddd, size);
	hipMallocHost(&Ehh, size);

	hipMemcpyAsync(Bh, Ah, size, hipMemcpyHostToHost, stream1);
	hipMemcpyAsync(Bhh, Ahh, size, hipMemcpyHostToHost, stream2);
	hipMemcpyAsync(Cd, Bh, size, hipMemcpyHostToDevice, stream1);
	hipMemcpyAsync(Cdd, Bhh, size, hipMemcpyHostToDevice, stream2);
	hipLaunchKernel(HIP_KERNEL_NAME(Inc), dim3(N/500), dim3(500), 0, stream1, Cd);
	hipLaunchKernel(HIP_KERNEL_NAME(Inc), dim3(N/500), dim3(500), 0, stream2, Cdd);
	hipMemcpyAsync(Dd, Cd, size, hipMemcpyDeviceToDevice, stream1);
	hipMemcpyAsync(Ddd, Cdd, size, hipMemcpyDeviceToDevice, stream2);
	hipMemcpyAsync(Eh, Dd, size, hipMemcpyDeviceToHost, stream1);
	hipMemcpyAsync(Ehh, Ddd, size, hipMemcpyDeviceToHost, stream2);
	HIPCHECK(hipDeviceSynchronize());
	HIPASSERT(Eh[10] = Ah[10] + 1.0f);
	HIPASSERT(Ehh[10] = Ahh[10] + 1.0f);
}

int main(int argc, char **argv){
	HipTest::parseStandardArguments(argc, argv, true);


	hipStream_t stream[3];
	for(int i=0;i<3;i++){
		HIPCHECK(hipStreamCreate(&stream[i]));
	}

	const size_t size = N * sizeof(float);

	std::thread t1(run1, size, stream[0]);
	std::thread t2(run1, size, stream[0]);
	std::thread t3(run, size, stream[1], stream[2]);
	t1.join();
//	std::cout<<"T1"<<std::endl;
	t2.join();
//	std::cout<<"T2"<<std::endl;
	t3.join(); 
	passed();
}
