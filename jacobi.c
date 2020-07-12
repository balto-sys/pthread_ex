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

#include <math.h>
#include <stdint.h>     // uint32_t and friends
#include <inttypes.h>   // PRIu32 and friends
#include <stdio.h>      // printf and friends
#include <sys/time.h>   // gettimeofday()

#define N (2000ULL)
#define NUMGRIDS (2ULL) 
double grid[NUMGRIDS][N][N];

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

void calculate_avg(uint32_t base_grid, uint32_t result_grid){
    uint32_t x, y;
    // The interior squares are easy; no bounds checking needed.
    for( y=1; y<(N-1); y++ ){
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
    for( y=0, x=1; x<(N-1); x++ ){
        grid[result_grid][x][y] = (
                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] +

                grid[base_grid][x-1][y+1] +
                grid[base_grid][x  ][y+1] +
                grid[base_grid][x+1][y+1] ) / 6.0;
    }

    // Bottom row, leave out the corners.
    for( y=N-1, x=1; x<(N-1); x++ ){
        grid[result_grid][x][y] = (
                grid[base_grid][x-1][y-1] +
                grid[base_grid][x  ][y-1] +
                grid[base_grid][x+1][y-1] +

                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] ) /6.0;
    }

    // Leftmost column, leave out the corners.
    for( y=1, x=0; y<(N-1); y++ ){
        grid[result_grid][x][y] = (
                grid[base_grid][x  ][y-1] +
                grid[base_grid][x+1][y-1] +

                grid[base_grid][x  ][y  ] +
                grid[base_grid][x+1][y  ] +

                grid[base_grid][x  ][y+1] +
                grid[base_grid][x+1][y+1] ) / 6.0;
    }

    // Rightmost column, leave out the corners.
    for( y=1, x=N-1; y<(N-1); y++ ){
        grid[result_grid][x][y] = (
                grid[base_grid][x-1][y-1] +
                grid[base_grid][x  ][y-1] +

                grid[base_grid][x-1][y  ] +
                grid[base_grid][x  ][y  ] +

                grid[base_grid][x-1][y+1] +
                grid[base_grid][x  ][y+1] ) / 6.0;
    }

    // Corners!
    /* remains constant
    grid[result_grid][0][0] = (
            grid[base_grid][0  ][0  ] +
            grid[base_grid][0  ][1  ] +
            grid[base_grid][1  ][0  ] +
            grid[base_grid][1  ][1  ] ) / 4.0;
            */

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

    /* remains constant
    grid[result_grid][N-1][N-1] = (
            grid[base_grid][N-1][N-1] +
            grid[base_grid][N-1][N-2] +
            grid[base_grid][N-2][N-1] +
            grid[base_grid][N-2][N-2] ) / 4.0;
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

int main(){
    double target_delta=0.05, delta;
    uint32_t count=0;
    struct timeval start, stop;

    gettimeofday( &start, NULL );
    initialize_grid();
    gettimeofday( &stop, NULL );
    fprintf(stdout, "%lf ", 
            (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)/1000000.0 );

    gettimeofday( &start, NULL );
    while(1){
        count++;
        if( count%2 ){
            calculate_avg(!count,!!count);
        }else{
            calculate_avg(!!count,!count);
        }

        delta = calculate_delta();

        if( delta < target_delta ){
            break;
        }
    }
    gettimeofday( &stop, NULL );
    fprintf(stdout, "%lf ", 
            (stop.tv_sec - start.tv_sec) - (stop.tv_usec - start.tv_usec)/1000000.0 );
    //print_grid(0);
}


