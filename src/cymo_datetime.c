/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include <assert.h>
#include <cymo.h>
#include <sys/time.h>
#include <time.h>

#define TICKS_PER_SECOND UINT64_C(1000000)
#define TICKS_PER_DAY UINT64_C(86400000000)

static const int days_preceding_month[2][13] = {
    { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
    { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};

/*static int days_preceding_month[2][13] ={
    { 0, 31,  59,  90,  120, 151, 181, 212, 243, 273, 304, 334, 365},
    { 0, 31,  60,  91,  121, 152, 182, 213, 244, 274, 305, 335, 366}
};*/

static const uint16_t DayOffset[13] = { 0, 306, 337, 0, 31, 61, 92,
    122, 153, 184, 214, 245, 275 };

static const uint32_t Pow10[10] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};

static int leap_year(uint16_t y)
{
    return ((y & 3) == 0 && (y % 100 != 0 || y % 400 == 0));
}

static unsigned char month_days(uint16_t y, uint16_t m)
{
    static const unsigned char days[2][13] = {
        { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
        { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
    };
    return days[m == 2 && leap_year(y)][m];
}

/* Rata Die algorithm by Peter Baum */

static void rdn_to_ymd(uint32_t rdn, uint16_t* yp, uint16_t* mp, uint16_t* dp,
    uint16_t* yday)
{
    uint32_t Z, H, A, B;
    uint16_t y, m, d, doy /* day of year 1..366*/;

    rdn++;
    Z = rdn + 306;
    H = 100 * Z - 25;
    A = H / 3652425;
    B = A - (A >> 2);
    y = (100 * B + H) / 36525;
    d = B + Z - (1461 * y >> 2);
    m = (535 * d + 48950) >> 14;
    if (m > 12)
        y++, m -= 12, doy = d - 306;
    else
        doy = d + 59 + ((y & 3) == 0 && (y % 100 != 0 || y % 400 == 0));

    *yp = y;
    *mp = m;
    *dp = d - DayOffset[m];
    if (yday)
        *yday = doy;
}

static inline uint32_t yd_to_rdn(uint16_t y, uint16_t d)
{
    y--;
    return 365 * y + y / 4 - y / 100 + y / 400 + d;
}

static inline uint32_t ymd_to_rdn(uint16_t y, uint16_t m, uint16_t d)
{
    if (m < 1 || m > 12) {
        y += m / 12;
        m %= 12;
        if (m < 1)
            y--, m += 12;
    }
    assert(m >= 1);
    assert(m <= 12);
    return yd_to_rdn(y, days_preceding_month[leap_year(y)][m] + d - 1);
}

datetime_t datetime_from_ymd(uint16_t year, uint16_t month, uint16_t day)
{
    uint64_t rdn = ymd_to_rdn(year, month, day);
    return rdn * TICKS_PER_DAY;
}

void datetime_to_ymd(datetime_t time, uint16_t* year, uint16_t* month,
    uint16_t* day)
{
    uint32_t rdn = time / TICKS_PER_DAY;
    rdn_to_ymd(rdn, year, month, day, NULL);
}

datetime_t datetime_from_hmsu(uint16_t hour, uint16_t minute, uint16_t second,
    uint32_t usec)
{
    // totalSeconds is bounded by 2^31 * 2^12 + 2^31 * 2^8 + 2^31,
    // which is less than 2^44, meaning we won't overflow totalSeconds.
    uint64_t totalSeconds = (uint64_t)hour * 3600 + (uint64_t)minute * 60 + (uint64_t)second;
    return totalSeconds * TICKS_PER_SECOND + usec;
}

void datetime_decode(datetime_t dt, uint16_t* year, uint16_t* month,
    uint16_t* day, uint16_t* hour, uint16_t* minute,
    uint16_t* second, uint32_t* usec)
{
    uint32_t rdn = dt / TICKS_PER_DAY;
    rdn_to_ymd(rdn, year, month, day, NULL);
    uint64_t perday = dt % TICKS_PER_DAY;
    *usec = perday % TICKS_PER_SECOND;
    uint32_t sod = perday / TICKS_PER_SECOND;
    *second = sod % 60;
    sod /= 60;
    *minute = sod % 60;
    sod /= 60;
    *hour = sod;
}

datetime_t datetime_from_timeval(struct timeval* t)
{
    uint64_t epoch_ticks = t->tv_sec * TICKS_PER_SECOND + t->tv_usec;
    return epoch_ticks + EPOCH_DATE_TIME;
}

static int parse_2d(const unsigned char* const p, size_t i, uint16_t* vp)
{
    unsigned char d0, d1;
    if (((d0 = p[i + 0] - '0') > 9) || ((d1 = p[i + 1] - '0') > 9))
        return 1;
    *vp = d0 * 10 + d1;
    return 0;
}

static int parse_4d(const unsigned char* const p, size_t i, uint16_t* vp)
{
    unsigned char d0, d1, d2, d3;
    if (((d0 = p[i + 0] - '0') > 9) || ((d1 = p[i + 1] - '0') > 9) || ((d2 = p[i + 2] - '0') > 9) || ((d3 = p[i + 3] - '0') > 9))
        return 1;
    *vp = d0 * 1000 + d1 * 100 + d2 * 10 + d3;
    return 0;
}

int datetime_from_iso8601(const char* str, size_t len, datetime_t* dt)
{
    const unsigned char *cur, *end;
    unsigned char ch;
    uint16_t year, month, day, hour, min, sec;
    uint16_t zone_hour, zone_min;
    uint32_t nsec;
    timespan_t offset;

    /*
     *           1
     * 01234567890123456789
     * 2013-12-31T23:59:59Z
     */
    cur = (const unsigned char*)str;
    if (len < 20 || cur[4] != '-' || cur[7] != '-' || cur[13] != ':' || cur[16] != ':')
        return 1;

    ch = cur[10];
    if (!(ch == 'T' || ch == ' ' || ch == 't'))
        return 1;

    if (parse_4d(cur, 0, &year) || year < 1 || parse_2d(cur, 5, &month) || month < 1 || month > 12 || parse_2d(cur, 8, &day) || day < 1 || day > 31 || parse_2d(cur, 11, &hour) || hour > 23 || parse_2d(cur, 14, &min) || min > 59 || parse_2d(cur, 17, &sec) || sec > 59)
        return 1;

    if (day > 28 && day > month_days(year, month))
        return 1;

    end = cur + len;
    cur = cur + 19;
    offset = nsec = 0;

    ch = *cur++;
    if (ch == '.') {
        const unsigned char* start;
        size_t ndigits;

        start = cur;
        for (; cur < end; cur++) {
            const unsigned char digit = *cur - '0';
            if (digit > 9)
                break;
            nsec = nsec * 10 + digit;
        }

        ndigits = cur - start;
        if (ndigits < 1 || ndigits > 9)
            return 1;

        nsec *= Pow10[9 - ndigits];

        if (cur == end)
            return 1;

        ch = *cur++;
    }

    if (!(ch == 'Z' || ch == 'z')) {
        /*
	 *  01234
	 * ±00:00
	 */
        if (cur + 5 < end || !(ch == '+' || ch == '-') || cur[2] != ':')
            return 1;

        if (parse_2d(cur, 0, &zone_hour) || zone_hour > 23 || parse_2d(cur, 3, &zone_min) || zone_min > 59)
            return 1;

        offset = (zone_hour * 60 + zone_min) * 60 * TICKS_PER_SECOND;
        if (ch == '-')
            offset *= -1;

        cur += 5;
    }

    if (cur != end)
        return 1;

    *dt = datetime_from_ymd(year, month, day);
    *dt += datetime_from_hmsu(hour, min, sec, nsec / 1000);
    *dt -= offset;
    return 0;
}

int datetime_to_tm(const datetime_t dt, struct tm* tmp)
{
    uint32_t rdn, sod;

    if (dt < dt - EPOCH_DATE_TIME)
        return 1;
    rdn = dt / TICKS_PER_DAY;
    sod = dt % TICKS_PER_DAY / TICKS_PER_SECOND;
    uint16_t y, m, d, doy;

    rdn_to_ymd(rdn, &y, &m, &d, &doy);
    tmp->tm_mday = d; /* Day of month [1,31]           */
    tmp->tm_yday = doy - 1; /* Day of year [0,365]           */
    tmp->tm_mon = m - 1; /* Month of year [0,11]          */
    tmp->tm_year = y - 1900; /* Years since 1900              */
    tmp->tm_wday = rdn % 7; /* Day of week [0,6] (Sunday =0) */

    tmp->tm_sec = sod % 60;
    sod /= 60;
    tmp->tm_min = sod % 60;
    sod /= 60;
    tmp->tm_hour = sod;
    return 0;
}

/* zone_offset [-1439,1439] minutes */
static size_t timestamp_format_internal(char* dst, size_t len,
    const datetime_t dt,
    int32_t zone_offset,
    const int precision)
{
    unsigned char* p;
    uint64_t sec;
    uint32_t rdn, v, nsec;
    uint16_t y, m, d;
    size_t dlen;

    dlen = sizeof("YYYY-MM-DDThh:mm:ssZ") - 1;

    if (precision)
        dlen += 1 + precision;

    if (dlen >= len)
        return 0;

    sec = dt % TICKS_PER_DAY / TICKS_PER_SECOND;
    rdn = dt / TICKS_PER_DAY;
    nsec = dt % TICKS_PER_DAY % TICKS_PER_SECOND;

    rdn_to_ymd(rdn, &y, &m, &d, NULL);

    /*
     *           1
     * 0123456789012345678
     * YYYY-MM-DDThh:mm:ss
     */
    p = (unsigned char*)dst;
    v = sec % TICKS_PER_SECOND;
    p[18] = '0' + (v % 10);
    v /= 10;
    p[17] = '0' + (v % 6);
    v /= 6;
    p[16] = ':';
    p[15] = '0' + (v % 10);
    v /= 10;
    p[14] = '0' + (v % 6);
    v /= 6;
    p[13] = ':';
    p[12] = '0' + (v % 10);
    v /= 10;
    p[11] = '0' + (v % 10);
    p[10] = 'T';
    p[9] = '0' + (d % 10);
    d /= 10;
    p[8] = '0' + (d % 10);
    p[7] = '-';
    p[6] = '0' + (m % 10);
    m /= 10;
    p[5] = '0' + (m % 10);
    p[4] = '-';
    p[3] = '0' + (y % 10);
    y /= 10;
    p[2] = '0' + (y % 10);
    y /= 10;
    p[1] = '0' + (y % 10);
    y /= 10;
    p[0] = '0' + (y % 10);
    p += 19;

    if (precision) {
        v = nsec / Pow10[9 - precision];
        switch (precision) {
        case 9:
            p[9] = '0' + (v % 10);
            v /= 10;
        case 8:
            p[8] = '0' + (v % 10);
            v /= 10;
        case 7:
            p[7] = '0' + (v % 10);
            v /= 10;
        case 6:
            p[6] = '0' + (v % 10);
            v /= 10;
        case 5:
            p[5] = '0' + (v % 10);
            v /= 10;
        case 4:
            p[4] = '0' + (v % 10);
            v /= 10;
        case 3:
            p[3] = '0' + (v % 10);
            v /= 10;
        case 2:
            p[2] = '0' + (v % 10);
            v /= 10;
        case 1:
            p[1] = '0' + (v % 10);
        }
        p[0] = '.';
        p += 1 + precision;
    }

    if (!zone_offset)
        *p++ = 'Z';
    else {
        if (zone_offset < 0)
            p[0] = '-', v = -zone_offset;
        else
            p[0] = '+', v = zone_offset;

        p[5] = '0' + (v % 10);
        v /= 10;
        p[4] = '0' + (v % 6);
        v /= 6;
        p[3] = ':';
        p[2] = '0' + (v % 10);
        v /= 10;
        p[1] = '0' + (v % 10);
        p += 6;
    }
    *p = 0;
    return dlen;
}

/*
 *          1         2         3
 * 12345678901234567890123456789012345 (+ null-terminator)
 * YYYY-MM-DDThh:mm:ssZ
 * YYYY-MM-DDThh:mm:ss±hh:mm
 * YYYY-MM-DDThh:mm:ss.123Z
 * YYYY-MM-DDThh:mm:ss.123±hh:mm
 * YYYY-MM-DDThh:mm:ss.123456Z
 * YYYY-MM-DDThh:mm:ss.123456±hh:mm
 * YYYY-MM-DDThh:mm:ss.123456789Z
 * YYYY-MM-DDThh:mm:ss.123456789±hh:mm
 */

size_t datetime_format(char* dst, size_t len, datetime_t dt, long offset)
{
    uint32_t f;
    int precision;

    f = dt % TICKS_PER_DAY % TICKS_PER_SECOND;
    if (!f)
        precision = 0;
    else {
        if ((f % 1000000) == 0)
            precision = 3;
        else if ((f % 1000) == 0)
            precision = 6;
        else
            precision = 9;
    }
    return timestamp_format_internal(dst, len, dt, offset, precision);
}

#ifdef _WIN32
void gettimeofday(struct timeval* tp, void*)
{
    uint64_t intervals;
    FILETIME ft;

    GetSystemTimeAsFileTime(&ft);

    /*
     * A file time is a 64-bit value that represents the number
     * of 100-nanosecond intervals that have elapsed since
     * January 1, 1601 12:00 A.M. UTC.
     *
     * Between January 1, 1970 (Epoch) and January 1, 1601 there were
     * 134774 days,
     * 11644473600 seconds or
     * 11644473600,000,000,0 100-nanosecond intervals.
     *
     * See also MSKB Q167296.
     */

    intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    intervals -= 116444736000000000;

    tp->tv_sec = (long)(intervals / 10000000);
    tp->tv_usec = (long)((intervals % 10000000) / 10);
}
#endif // _WIN32

#ifdef _WIN32
long _gettimezone(void)
{
    uin32_t n;
    TIME_ZONE_INFORMATION tz;

    n = GetTimeZoneInfomation(&tz);

    switch (n) {
    case TIME_ZONE_ID_UNKNOWN:
        return -tz.Bias;

    case TIME_ZONE_ID_STANDARD:
        return -(tz.Bias + tz.StandardBias);

    case TIME_ZONE_ID_DAYLIGHT:
        return -(tz.Bias + tz.DaylightBias);

    default: /* TIME_ZONE_ID_INVALID */
        return 0;
    }
}
int32_t get_timezone_offset(void) { return _gettimezone() * 60; }

void cm_timezone_update(void) {}

#else
void cm_timezone_update(void)
{
#if (__FreeBSD__)

    if (getenv("TZ")) {
        return;
    }

    putenv("TZ=UTC");

    tzset();

    unsetenv("TZ");

    tzset();

#elif (__linux__)
    time_t s;
    struct tm* t;
    char buf[4];

    s = time(0);

    t = localtime(&s);

    strftime(buf, 4, "%H", t);

#endif
}

long get_timezone_offset(void)
{
    time_t sec = time(NULL);
    struct tm tm;
    localtime_r(&sec, &tm);
    return tm.tm_gmtoff / 60;
}
#endif

datetime_t datetime_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t intervals = tv.tv_sec * TICKS_PER_SECOND + tv.tv_usec + EPOCH_DATE_TIME;
    return intervals;
}
