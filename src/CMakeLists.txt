cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(cymo C)

set(CYMO_HEADER
        cymo_event.h
        cymo_loop.h
        cymo_queue.h
        event_pipe.h
        reminder_queue.h
        spsc_queue.h
)
set(CYMO_SRC
        cymo_datetime.c
        cymo_event.c
        cymo_loop.c
        cymo_queue.c
        cymo_version.c
        event_pipe.c
        reminder_queue.c
        spsc_queue.c
)

add_library(cymo STATIC  ${CYMO_HEADER} ${CYMO_SRC})
target_link_libraries(cymo ${LIBUV_LIBRARY})
