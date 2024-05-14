


#ifndef __ARRAYDEEP_H__
#define __ARRAYDEEP_H__

#include <arraydeep.h>


/*
static 
int find(
    int array_length, 
    global int3 *array ,
    int3 elem 
    ) {
    for(int i = 0; i < array_length; ++i) {
        if ( array[i][0] == elem[0] && array[i][1] == elem[1] && array[i][2] == elem[2] ) {
            return i;
        }
    }
    return -array_length - 1;
}

static 
int find(
    int array_length, 
    global int *array ,
    int3 elem 
    ) {
    for ( int i = 0; i < array_length; ++i ) {
        int threei = i+i+i;//3*i
        if ( array[3*i] == elem[0] && array[threei+1] == elem[1] && array[threei+2] == elem[2] ) {
            return i;
        }
    }
    return -array_length - 1;
}

static 
int find(
    int array_length, 
    global int *array ,
    int elem0, 
    int elem1, 
    int elem2 
    ) {
    for ( int i = 0; i < array_length; ++i ) {
        int threei = i+i+i;//3*i
        if ( array[threei] == elem0 && array[threei+1] == elem1 && array[threei+2] == elem2 ) {
            return i;
        }
    }
    return -array_length - 1;
}




static 
int find(
    int array_start, 
    int array_end, 
    global int *array ,
    int elem 
    ) {
    for(int i = array_start; i < array_end; ++i) {
        if ( array[i] == elem ) {
            return i;
        }
    }
    return - array_end - array_start - 1;
}
*/

static 
int find(
    int array_length, 
    global int *array ,
    int elem 
    ) {
    for(int i = 0; i < array_length; ++i) {
        if ( array[i] == elem ) {
            return i;
        }
    }
    return -array_length - 1;
}

static 
int minidx(
    int array_len, 
    private float *array 
    ) {
    int min_idx;
    float minval = 1e10;
    for (int i = 0; i < array_len; ++i) {
        if ( array[i] > minval ) continue;
        minval = array[i];
        min_idx = i;
    }
    return min_idx;
}


/*

static 
void preservevalue(
    //int outarr_length, 
    global int *outarr ,
    int arr_length, 
    global int *arr ,
    int arr0_length, 
    global const int *arr0
    ) {
    int len = 0;
    for ( int i = 0; i < arr_length; ++i ) {
        if ( find(arr0_length, arr0, arr[i]) > -1 ) continue;
        outarr[len] = arr[i];
        len += 1;
    }
}

*/



#endif


