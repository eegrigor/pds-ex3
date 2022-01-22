#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int *unroll(int **ising, int n){
    int *ising1d = (int *)malloc(n * n * sizeof(int));
    for(int i = 0 ; i < n ; i++){
        for(int j = 0 ; j < n ; j++){
            ising1d[i*n + j] = ising[i][j];
        }
    }

    return ising1d;
}
void swap(int  **a, int  **b) {
  int  *tmp = *a;
  *a = *b;
  *b = tmp;
}

__global__ void moment(int *ising, int *newising, int n, int mpt){
    if(threadIdx.x == (n*n)/mpt - 1 ){
        for(int i = 0 ; i < mpt ; i++){
            if(threadIdx.x * mpt) + i < n*n){
                int sum = ising[(((threadIdx.x * mpt) + i)+n)%(n*n)] + ising[(((threadIdx.x * mpt) + i)-n)%(n*n)]
                + ising[((threadIdx.x * mpt) + i) + 1 - (n - 1)*(((threadIdx.x * mpt) + i)%n == n - 1)]
                + ising[((threadIdx.x * mpt) + i) - 1 + (n - 1)*(((threadIdx.x * mpt) + i)%n == 0)]
                + ising[((threadIdx.x * mpt) + i)];
                if(sum > 0)
                    newising[((threadIdx.x * mpt) + i)] = 1 ;
                else
                    newising[((threadIdx.x * mpt) + i)] = -1 ;
            }
            else break;
        }
    }
    else{
        for(int i = 0 ; i < mpt ; i++){
            int sum = ising[(((threadIdx.x * mpt) + i)+n)%(n*n)] + ising[(((threadIdx.x * mpt) + i)-n)%(n*n)]
            + ising[((threadIdx.x * mpt) + i) + 1 - (n - 1)*(((threadIdx.x * mpt) + i)%n == n - 1)]
            + ising[((threadIdx.x * mpt) + i) - 1 + (n - 1)*(((threadIdx.x * mpt) + i)%n == 0)]
            + ising[((threadIdx.x * mpt) + i)];
            if(sum > 0)
                newising[((threadIdx.x * mpt) + i)] = 1 ;
            else
                newising[((threadIdx.x * mpt) + i)] = -1 ;
        }
    }
}

int main(int argc, char **argv){

    //size of Ising model
    int n = 2 ;
    // number of iterations
    int k = 2 ;

    srand(time(NULL));

    int *ising = (int *) malloc(n * n * sizeof(int));
    for(int i = 0 ; i < n ; i++){
        for(int j = 0 ; j < n ; j ++){
            ising[i*n + j] = rand() % 2 ;
            if(ising[i*n + j] == 0){
                ising[i*n + j] = -1;
            }
        }
    }

    for(int i = 0 ; i < n ; i++){
        for(int j = 0 ; j < n ; j++){
            printf("%d " , ising[i*n + j]);
        }
        printf("\n");
    }
    printf("\n");

    int *newising = (int *)malloc(n * n * sizeof(int));
    
    int *d_ising;
    int *d_newising;
    int size = n * n * sizeof(int);
    
    //allocate on gpu
    cudaMalloc((void**)&d_ising, size);
    cudaMalloc((void**)&d_newising, size);
    
    //moments per thread 
    int mpt = 2 ; 
    

    for(int l = 0 ; l < k ; l++){
        //copy data to gpu
        cudaMemcpy(d_ising, ising, size, cudaMemcpyHostToDevice);
        //call function on gpu with n*n threads
        moment<<<1,(n*n)/mpt>>>(d_ising, d_newising, n , mpt);
        //copy result from gpu
        cudaMemcpy(newising, d_newising, size, cudaMemcpyDeviceToHost);

        swap(&ising,&newising);

        for(int i = 0 ; i < n ; i++){
            for(int j = 0 ; j < n ; j++){
                printf("%d " , ising[i*n + j]);
            }
            printf("\n");
        }
        printf("\n");
        
    }
    //free pointers
    free(ising);
    free(newising);
    cudaFree(d_ising);
    cudaFree(d_newising);
    return 0 ;
}

