/* You are given a N x N array where the value in each
 * array represents an initial temperature.  There is
 * one heat source and one heat sink where the temperatures
 * remain constant.  The temperature of any location in the
 * 2d array at time t+1 is the average of the temperatures
 * of the surrounding locations (and the location itself)
 * at time t.
 *
 * Your job is to evalute this for sufficient timesteps that
 * the system reaches equilibrium (where the max change
 * between time t and t+1 for any cell is less than some delta.
 *
 * The standard way of doing this is to have two grids, one
 * for time t and one for time t+1.  After you've solved for
 * time t+1, swap the grids and continue.
 */

#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>     // uint32_t and friends
#include <inttypes.h>   // PRIu32 and friends
#include <stdio.h>      // printf and friends
#include <sys/time.h>   // gettimeofday()

#define N (2000ULL)
#define NUM_THREADS (N + 3) // One per row + 2 for first and last column + 1 for corners.
#define NUMGRIDS (2ULL) 

double grid[NUMGRIDS][N][N];

enum{
    BARRIER_INIT        =0,
    BARRIER_DELTA       =1,
    NUM_BARRIERS        =2
};
static pthread_barrier_t barrier[NUM_BARRIERS];

void initialize_grid(){
    uint32_t grid_idx, x, y;
    for( grid_idx=0; grid_idx < NUMGRIDS; grid_idx++ ){
        for( x=0; x<N; x++ ){
            for( y=0; y<N; y++ ){
                grid[grid_idx][x][y] = 0.0; // Not strictly necessary.
            }
        }
    }
    grid[0][0][0] = -100.0;     // heat sink
    grid[0][N-1][N-1] = 100.0;  // heat source

    grid[1][0][0] = -100.0;     // heat sink
    grid[1][N-1][N-1] = 100.0;  // heat source

    // No additional initialization for the delta grid.
    return;
}

void print_grid(uint32_t grid_idx){
    uint32_t x, y;
    for( y=0; y<N; y++ ){
        for( x=0; x<N; x++ ){
            fprintf( stdout, "%05.1lf ", grid[grid_idx][x][y] );
        }
        fprintf( stdout, "\n" );
    }
    fprintf( stdout, "\n" );
}

void calculate_avg(uint32_t base_grid, uint32_t result_grid, uint64_t y){
    uint64_t x;
    // The interior squares are easy; no bounds checking needed.
    //for( y=1; y<(N-1); y++ ){
    if( ( y>0 ) && ( y < N-1 ) ){
        for( x=1; x<(N-1); x++ ){
            grid[result_grid][x][y] =(
                grid[base_grid][x-1][y-1] +
                grid[base_grid][x  ][y-1] +
                grid[base_grid][x+1][y-1] +

                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] +

                grid[base_grid][x-1][y+1] +
                grid[base_grid][x  ][y+1] +
                grid[base_grid][x+1][y+1] ) / 9.0;
        }
    }
    // Top row, leave out the corners.
    //for( y=0, x=1; x<(N-1); x++ ){
    else if( y==0 ){
        for( x=1; x<(N-1); x++ ){
            grid[result_grid][x][y] = (
                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] +

                grid[base_grid][x-1][y+1] +
                grid[base_grid][x  ][y+1] +
                grid[base_grid][x+1][y+1] ) / 6.0;
        }
    }

    // Bottom row, leave out the corners.
    //for( y=N-1, x=1; x<(N-1); x++ ){
    else if( y==N-1 ){
        for( x=1; x<(N-1); x++ ){
            grid[result_grid][x][y] = (
                grid[base_grid][x-1][y-1] +
                grid[base_grid][x  ][y-1] +
                grid[base_grid][x+1][y-1] +

                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] ) /6.0;
            }
    }

    // Leftmost column, leave out the corners.
    else if( y==N ){ //special case #1
        for( y=1, x=0; y<(N-1); y++ ){
            grid[result_grid][x][y] = (
                    grid[base_grid][x  ][y-1] +
                    grid[base_grid][x+1][y-1] +

                    grid[base_grid][x  ][y  ] +
                    grid[base_grid][x+1][y  ] +

                    grid[base_grid][x  ][y+1] +
                    grid[base_grid][x+1][y+1] ) / 6.0;
        }
        y=N;    // Reset y value to its thread id.
    }

    // Rightmost column, leave out the corners.
    else if( y==N+1 ){ //special case #2
        for( y=1, x=N-1; y<(N-1); y++ ){
            grid[result_grid][x][y] = (
                    grid[base_grid][x-1][y-1] +
                    grid[base_grid][x  ][y-1] +

                    grid[base_grid][x-1][y  ] +
                    grid[base_grid][x  ][y  ] +

                    grid[base_grid][x-1][y+1] +
                    grid[base_grid][x  ][y+1] ) / 6.0;
        }
        y=N+1;
    }

    // Corners!
    else if( y==N+2 ){ // special case #3
        grid[result_grid][0][N-1] = (
                grid[base_grid][0  ][N-1] +
                grid[base_grid][0  ][N-2] +
                grid[base_grid][1  ][N-1] +
                grid[base_grid][1  ][N-2] ) / 4.0;

        grid[result_grid][N-1][0] = (
                grid[base_grid][N-1][0  ] +
                grid[base_grid][N-2][0  ] +
                grid[base_grid][N-1][1  ] +
                grid[base_grid][N-2][1  ] ) / 4.0;
    }

    /* remains constant
    grid[result_grid][N-1][N-1] = (
            grid[base_grid][N-1][N-1] +
            grid[base_grid][N-1][N-2] +
            grid[base_grid][N-2][N-1] +
            grid[base_grid][N-2][N-2] ) / 4.0;
            */
    /* remains constant, as these are our source and sink.
    grid[result_grid][0][0] = (
            grid[base_grid][0  ][0  ] +
            grid[base_grid][0  ][1  ] +
            grid[base_grid][1  ][0  ] +
            grid[base_grid][1  ][1  ] ) / 4.0;
            */

    return;
}

double calculate_delta(){
    double delta=0.0, max_delta=0.0;
    uint32_t x,y;
    for(x=0; x<N; x++){
        for(y=0; y<N; y++){
            delta = fabs( grid[0][x][y] - grid[1][x][y] );
            if( delta > max_delta ){
                max_delta = delta;
            }
        }
    }
    return max_delta;
}

static double delta = 0.0;  // Only thread 0 is allowed to write to this.

void* thread_loop(void *threadid){

    uint64_t t = (uint64_t)(threadid);
    double target_delta=0.05;
    uint32_t count=0;
    struct timeval init_start, init_stop, delta_start, delta_stop, calc_start, calc_stop;
    double elapsed_delta=0.0, elapsed_calc=0.0;


    // Initialization (Have thread 0 do this for now as it doesn't take long.)
    if( t == 0 ){
        gettimeofday( &init_start, NULL );
        initialize_grid();
        gettimeofday( &init_stop, NULL );
        fprintf(stdout, "%lf ", (init_stop.tv_sec - init_start.tv_sec) + (init_stop.tv_usec - init_start.tv_usec)/1000000.0 );
    }

    // Hold up all threads until after thread 0 is done initializing.
    pthread_barrier_wait( &barrier[BARRIER_INIT] );

    // Combined calculation and stopping condition
    while(1){
        count++;
        gettimeofday( &calc_start, NULL );
        calculate_avg(!(count%2),!!(count%2), t);
        gettimeofday( &calc_stop, NULL );
        elapsed_calc += (calc_stop.tv_sec - calc_start.tv_sec) + (calc_stop.tv_usec - calc_start.tv_usec)/1000000.0;

        gettimeofday( &delta_start, NULL );
        if( t==0 ){ // Just have thread 0 do this.
            delta = calculate_delta();
        }
        gettimeofday( &delta_stop, NULL );
        elapsed_delta += (delta_stop.tv_sec - delta_start.tv_sec) + (delta_stop.tv_usec - delta_start.tv_usec)/1000000.0;

        // Force everyone to wait until thread 0 has written to the global delta variable.
        pthread_barrier_wait( &barrier[BARRIER_DELTA] );

        if( delta < target_delta ){
            break;
        }
    }

    if( t==0 ){ // Only want one thread doing this.
        fprintf(stdout, "%lf %lf ", elapsed_delta, elapsed_calc);
        //print_grid(0);
    }
    pthread_exit(NULL);
}

int main(){
    // The only thing we want main() to do is create, launch and join threads.
    pthread_t threads[NUM_THREADS];
    uint64_t t;

    for( t=0; t<NUM_BARRIERS; t++ ){
        assert( ! pthread_barrier_init( &barrier[t], NULL, NUM_THREADS ) );
    }

    for( t=0; t<NUM_THREADS; t++ ){
        assert( ! pthread_create(&threads[t], NULL, thread_loop, (void*)t ) );
    }

    for( t=0; t<NUM_THREADS; t++ ){
        pthread_join( threads[t], NULL );
    }

    for( t=0; t<NUM_BARRIERS; t++ ){
        assert( ! pthread_barrier_destroy( &barrier[t] ) );
    }
    pthread_exit(NULL);

}
