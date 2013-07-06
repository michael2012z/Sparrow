#ifndef _DATE_H_
#define _DATE_H_

struct _date_t {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

typedef struct _date_t date_t;

date_t date_parse_format_iso(char *str);

#endif /* _DATE_H_ */
