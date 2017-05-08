/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/
#include <sys/time.h>
#include <cm.h>
#include "../src/adt_tree.h"
#include "../src/adt_queue.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct timer_node_s {
    RB_ENTRY(rb_node_s) rbentry;
    datetime_t timestamp;
    int index;
}timer_node_t;


static int rb_node_cmp(timer_node_t *lhs, timer_node_t *rhs) {
    if (lhs->timestamp < rhs->timestamp)
        return -1;
    if (lhs->timestamp > rhs->timestamp)
        return 1;
    if (lhs->index < rhs->index)
        return -1;
    if (lhs->index > rhs->index)
        return 1;
    return 0;
}

RB_HEAD(rbtree, rb_node_s) head = RB_INITIALIZER(&head);

RB_GENERATE_STATIC(rbtree, rb_node_s, rbentry, rb_node_cmp)

#define MAX_VALUE 10000000L 

timer_node_t nodes[MAX_VALUE];
int main()
{
    struct timeval istart, iend;
    gettimeofday(&istart, NULL);
    timer_node_t* node, *tmp;
    for (int i = 0; i < MAX_VALUE; i++){
        node = &nodes[i];//malloc(sizeof(rb_node_t));
        node->timestamp = datetime_now();
        node->index = i;
        timer_node_t *ret = RB_INSERT(rbtree, &head, node);
        if(ret)
            printf("Coll %d\n", ret->index);
    }

    gettimeofday(&iend, NULL);
    char dst[100];
    RB_FOREACH(node, rbtree, &head) {
        datetime_format(dst, sizeof(dst), node->timestamp, 0);
        printf("time = %s, index= %d\n", dst, node->index);
    }
    struct timeval dstart, dend;
    gettimeofday(&dstart, NULL);
    RB_FOREACH_SAFE(node, rbtree, &head, tmp){
        RB_REMOVE(rbtree, &head, node);
        //free(node);
    }
    gettimeofday(&dend, NULL);

    int idelta = (iend.tv_sec - istart.tv_sec) * 1000000 + (iend.tv_usec - istart.tv_usec);
    double itime = (double)idelta / 1000000.0;

    int ddelta = (dend.tv_sec - dstart.tv_sec) * 1000000 + (dend.tv_usec - dstart.tv_usec);
    double dtime = (double)ddelta / 1000000.0;
    printf("itime = %f dtime=%f\n", itime, dtime);
}
