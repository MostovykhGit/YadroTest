#include "source.h"

#include <cstdio>

// ouput events id
const int ClientLeftID = 11;
const int ClientSitID = 12;
const int ErrorID = 13;

Time::operator std::string() const {
  std::stringstream sstream;
  sstream << std::setfill('0') << std::setw(2) << hour << ':';
  sstream << std::setfill('0') << std::setw(2) << minute;
  return sstream.str();
}

bool operator>=(const Time& time1, const Time& time2) {
  return ((time1.hour >= time2.hour) ||
          (time1.hour == time2.hour && time1.minute >= time2.minute));
}

Time operator+(const Time& time1, const Time& time2) {
  int new_hour = time1.hour + time2.hour + (time1.minute + time2.minute) / 60;
  int new_minute = (time1.minute + time2.minute) % 60;
  return Time(new_hour, new_minute);
}

Time difference(const Time& time1, const Time& time2) {
  Time before = (time1 >= time2 ? time2 : time1);
  Time after = (time1 >= time2 ? time1 : time2);

  return Time(
      (after.hour * 60 + after.minute - before.hour * 60 - before.minute) / 60,
      (after.hour * 60 + after.minute - before.hour * 60 - before.minute) % 60);
}

Session::Session(int hour_price, int tables_amt, Time start_time,
                 Time finish_time)
    : hour_price(hour_price),
      tables_amt(tables_amt),
      start_time(start_time),
      finish_time(finish_time) {
  std::cout << static_cast<std::string>(start_time) << '\n';
  for (int i = 1; i <= tables_amt; ++i) {
    money_from_table[i] = 0;
    time_from_table[i] = Time(0, 0);
  }
}

void Session::client_left_msg(Time time, std::string client) {
  std::cout << static_cast<std::string>(time) << ' ' << ClientLeftID << ' '
            << client << '\n';
}

void Session::client_sit_msg(Time time, std::string client, int table) {
  std::cout << static_cast<std::string>(time) << ' ' << ClientSitID << ' '
            << client << ' ' << table << '\n';
}

void Session::error_msg(Time time, std::string msg) {
  std::cout << static_cast<std::string>(time) << ' ' << ErrorID << ' ' << msg
            << '\n';
}

// обрабатываем конкретный входной запрос
void Session::process_event(Event cur_event) {
  if (cur_event.id == 1) {
    if (!(cur_event.time >= start_time)) {
      error_msg(cur_event.time, "NotOpenYet");
    } else if (clients_st.find(cur_event.client) != clients_st.end()) {
      error_msg(cur_event.time, "YouShallNotPass");
    } else {
      clients_st.insert(cur_event.client);
    }
  }
  if (cur_event.id == 2) {
    if (clients_st.find(cur_event.client) == clients_st.end()) {
      error_msg(cur_event.time, "ClientUnknown");
    } else {
      if (client_table_mp.find(cur_event.client) != client_table_mp.end()) {
        if (table_client_mp.find(cur_event.table) != table_client_mp.end()) {
          error_msg(cur_event.time, "PlaceIsBusy");
        } else {
          Time temp_difference =
              difference(cur_event.time, client_starttime_mp[cur_event.client]);
          time_from_table[client_table_mp[cur_event.client]] =
              time_from_table[client_table_mp[cur_event.client]] +
              temp_difference;
          money_from_table[client_table_mp[cur_event.client]] +=
              (temp_difference.minute == 0 ? temp_difference.hour
                                           : temp_difference.hour + 1) *
              hour_price;
          table_client_mp.erase(client_table_mp[cur_event.client]);
          client_starttime_mp[cur_event.client] = cur_event.time;
          client_table_mp[cur_event.client] = cur_event.table;
          table_client_mp[cur_event.table] = cur_event.client;
        }
      } else {
        if (table_client_mp.find(cur_event.table) != table_client_mp.end()) {
          error_msg(cur_event.time, "PlaceIsBusy");
        } else {
          client_starttime_mp[cur_event.client] = cur_event.time;
          client_table_mp[cur_event.client] = cur_event.table;
          table_client_mp[cur_event.table] = cur_event.client;
          tables_busy += 1;
        }
      }
    }
  }
  if (cur_event.id == 3) {
    if (tables_busy < tables_amt) {
      error_msg(cur_event.time, "ICanWaitNoLonger!");
    } else if (clients_st.find(cur_event.client) == clients_st.end()) {
      error_msg(cur_event.time, "ClientUnknown");
    } else if (static_cast<int>(clients_q.size()) > tables_amt) {
      client_left_msg(cur_event.time, cur_event.client);
      clients_st.erase(cur_event.client);
    } else {
      clients_q.push(cur_event.client);
    }
  }
  if (cur_event.id == 4) {
    if (clients_st.find(cur_event.client) == clients_st.end()) {
      error_msg(cur_event.time, "ClientUnknown");
    } else if (client_table_mp.find(cur_event.client) !=
               client_table_mp.end()) {
      Time temp_difference =
          difference(cur_event.time, client_starttime_mp[cur_event.client]);
      time_from_table[client_table_mp[cur_event.client]] =
          time_from_table[client_table_mp[cur_event.client]] + temp_difference;
      money_from_table[client_table_mp[cur_event.client]] +=
          (temp_difference.minute == 0 ? temp_difference.hour
                                       : temp_difference.hour + 1) *
          hour_price;
      clients_st.erase(cur_event.client);
      int prev_table = client_table_mp[cur_event.client];
      table_client_mp.erase(prev_table);
      client_table_mp.erase(cur_event.client);
      client_starttime_mp.erase(cur_event.client);

      if (clients_q.empty()) {
        tables_busy--;
      } else {
        std::string client_from_q = clients_q.front();
        client_sit_msg(cur_event.time, client_from_q, prev_table);
        clients_q.pop();
        client_starttime_mp[client_from_q] = cur_event.time;
        client_table_mp[client_from_q] = prev_table;
        table_client_mp[prev_table] = client_from_q;
      }
    }
  }
}

void Session::end_of_day() {
  std::set<std::string> people_last;
  for (const auto& my_pair : client_table_mp) {
    people_last.insert(my_pair.first);
  }

  for (const auto& el : people_last) {
    client_left_msg(finish_time, el);
    Time temp_difference = difference(finish_time, client_starttime_mp[el]);
    time_from_table[client_table_mp[el]] =
        time_from_table[client_table_mp[el]] + temp_difference;
    money_from_table[client_table_mp[el]] +=
        (temp_difference.minute == 0 ? temp_difference.hour
                                     : temp_difference.hour + 1) *
        hour_price;
    clients_st.erase(el);
  }

  std::cout << static_cast<std::string>(finish_time) << '\n';

  for (int i = 1; i <= tables_amt; i++) {
    std::cout << i << ' ' << money_from_table[i] << ' '
              << static_cast<std::string>(time_from_table[i]) << '\n';
  }
}

// обрабатываем поток входных данных
void input_handling(std::string file_name) {
  std::ifstream in_stream;
  in_stream.open(file_name);
  if (!in_stream.is_open()) {
    std::cout << "Not correct filename";
    exit(0);
  }

  std::string input_line;
  int assigned_arguments = 0;

  int tables_amt = 0;
  getline(in_stream, input_line);
  assigned_arguments = sscanf(input_line.c_str(), "%d", &tables_amt);
  if (assigned_arguments != 1) {
    std::cout << input_line << '\n';
    std::cout << "Not correct input";
    exit(0);
  }

  Time start_time;
  Time finish_time;
  getline(in_stream, input_line);
  assigned_arguments =
      sscanf(input_line.c_str(), "%d:%d %d:%d", &start_time.hour,
             &start_time.minute, &finish_time.hour, &finish_time.minute);
  if (assigned_arguments != 4) {
    std::cout << input_line << '\n';
    std::cout << "Not correct input";
    exit(0);
  }

  int hour_price = 0;
  getline(in_stream, input_line);
  assigned_arguments = sscanf(input_line.c_str(), "%d", &hour_price);
  if (assigned_arguments != 1) {
    std::cout << input_line << '\n';
    std::cout << "Not correct input";
    exit(0);
  }

  Session cur_session(hour_price, tables_amt, start_time, finish_time);

  Time prev_time(0, 0);
  Time cur_time;
  int event_id;
  std::string cur_client;
  char colon;
  int cur_table = 0;

  while (getline(in_stream, input_line)) {
    // getline(in_stream, input_line);
    std::stringstream ss(input_line);

    ss >> cur_time.hour >> colon >> cur_time.minute >> event_id >> cur_client;
    if (colon != ':') {
      std::cout << input_line << '\n';
      std::cout << "Not correct input";
      exit(0);
    }
    if (event_id == 2) {
      ss >> cur_table;
      if (cur_table > tables_amt) {
        std::cout << input_line;
        std::cout << "Not correct input";
        exit(0);
      }
    }
    if (!(cur_time >= prev_time)) {
      std::cout << input_line;
      std::cout << "Not correct input";
      exit(0);
    }
    if (!(event_id >= 1 && event_id <= 4)) {
      std::cout << input_line;
      std::cout << "Not correct input";
      exit(0);
    }
    prev_time = cur_time;

    std::cout << input_line << '\n';
    Event cur_event(cur_time, event_id, cur_client, cur_table);
    cur_session.process_event(cur_event);
  }
  cur_session.end_of_day();

  in_stream.close();
}

// проверка входных параметров
int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Not correct input parametrs";
    exit(0);
  }

  input_handling(argv[1]);
}
