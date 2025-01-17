#ifndef SOURCE_H
#define SOURCE_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>

// структура для времени
struct Time {
  int hour = 0;
  int minute = 0;

  Time(int hour, int minute) : hour(hour), minute(minute) {}
  Time() = default;

  operator std::string() const;
};

// переопределение некоторых операторов
bool operator>=(const Time& time1, const Time& time2);
Time operator+(const Time& time1, const Time& time2);
Time difference(const Time& time1, const Time& time2);

// структура для входящего события
struct Event {
  Time time;
  int id = 0;
  std::string client;
  int table = 0;
  Event(Time time, int id, std::string client, int table)
      : time(time), id(id), client(client), table(table) {}
};

// класс для сессии - обработка входных, выходных событий дня
class Session {
 public:
  Session(int hour_price, int tables_amt, Time start_time, Time finish_time);
  void client_left_msg(Time time, std::string client);
  void client_sit_msg(Time time, std::string client, int table);
  void error_msg(Time time, std::string msg);
  void process_event(Event cur_event);
  void end_of_day();

 private:
  int hour_price;
  int tables_amt;
  int tables_busy = 0;
  Time start_time;
  Time finish_time;
  std::map<int, int> money_from_table;
  std::map<int, Time> time_from_table;
  std::map<std::string, int> client_table_mp;
  std::map<int, std::string> table_client_mp;
  std::map<std::string, Time> client_starttime_mp;
  std::set<std::string> clients_st;
  std::queue<std::string> clients_q;
};

void input_handling(std::string file_name);

#endif  // SOURCE_H
