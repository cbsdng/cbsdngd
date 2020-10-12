#pragma once

#include <iostream>
#include <sstream>
#include <string>

class Message
{
public:
  Message()
  {
    id = 0;
    type = 0;
    payload = "";
  }
  Message(const int &_id, const int &_type, const std::string &_payload)
  {
    data(_id, _type, _payload);
  }

  friend std::ostream &operator<<(std::ostream &os, const Message &m)
  {
    os << m.id << ' ' << m.type << ' ';
    os << m.payload.size() << ' ' << m.payload;
    return os;
  }
  friend std::istream &operator>>(std::istream &is, Message &m)
  {
    is >> m.id;
    if (is.fail())
    {
      std::cerr << "Failed to load id\n";
      return is;
    }
    is >> m.type;
    if (is.fail())
    {
      std::cerr << "Failed to load type\n";
      return is;
    }
    int size;
    is >> size;
    if (is.fail())
    {
      std::cerr << "Failed to load size\n";
    }
    ++size;
    char c;
    is.get(c);
    if (is.fail())
    {
      std::cerr << "Failed to load space after size\n";
      return is;
    }
    char buffer[size];
    is.get(buffer, size, '\0');
    if (is.fail())
    {
      std::cerr << "Failed to load payload\n";
      return is;
    }
    m.payload = buffer;
    return is;
  }

  std::string data() const
  {
    std::stringstream s;
    s << id << ' ' << type << ' ' << payload;
    return s.str();
  }

  void data(const int &_id, const int &_type, const std::string &_payload)
  {
    id = _id;
    type = _type;
    payload = _payload;
  }

  int getid() const { return id; };
  int gettype() const { return type; };
  std::string getpayload() const { return payload; };

protected:
  int id;
  int type;
  std::string payload;
};
