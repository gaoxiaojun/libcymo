#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <cm.h>

char dst[100];

void print_datetime2(datetime_t dt)
{
	datetime_format(dst, sizeof(dst), dt, 0);
	printf("%s\n", dst);
}

void print_timezone()
{
        struct  timeval    tv;
        struct  timezone   tz;
        gettimeofday(&tv,&tz);

        printf("tv_sec:%ld\n",tv.tv_sec);
        printf("tv_usec:%d\n",tv.tv_usec);
        printf("tz_minuteswest:%d\n",tz.tz_minuteswest);
        printf("tz_dsttime:%d\n",tz.tz_dsttime);
}

void test_datetime_now_performance(void)
{
    const int MAX_VALUE = 100000;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for(int i = 0; i < MAX_VALUE; i++) {
        datetime_now();
    }
    gettimeofday(&end, NULL);
    int delta = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("1s op = %f\n", MAX_VALUE / (double)delta * 1000000);
}

int main()
{
	char timestr[] = "2013-12-31T00:00:00Z";
 	datetime_t dt;	
       	datetime_from_iso8601(timestr, strlen(timestr), &dt);
	print_datetime2(dt);
	datetime_t dt2;
	datetime_from_iso8601(dst, strlen(dst), &dt2);
	print_datetime2(dt2);
	assert(dt == dt2);	
	datetime_t now = datetime_now();
	print_datetime2(now);
	print_timezone();
    test_datetime_now_performance();
}
