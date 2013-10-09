#ifndef _LIGHTSAX_H_
#define _LIGHTSAX_H_

#include <cstdio>
#include <cstring>
#include <cctype>

namespace lightsax
{

class handler_t
{
  public:
  virtual void start_element(char * name, char * attrs[][2]) = 0;
  virtual void end_element(char * name) = 0;
  virtual void characters(char * ch, int start, int length) = 0;
};

class parser_t
{
  public:
  parser_t(handler_t * a_handler)
  {
    handler = a_handler;
  }

  bool parse (FILE* a_fp)
  {
    int nmemb = 0;

    offset = 0;
    size = 0;
    buffer = new char[4 * (1 << 20)];

    do
    {
      memset(buffer + size, 0, (4096*256)*2);
      nmemb = fread(buffer + size, 4096, 256, a_fp);

      while (buffer[size])
        size++;

      while (process_data());

      memmove(buffer, buffer + offset, size + 1);
      offset = 0;
    }
    while (nmemb);

    delete[] buffer;
    return true;
  }

  bool parse (char* a_buffer)
  {
    return parse(a_buffer, strlen(a_buffer));
  }

  bool parse (char* a_buffer, size_t a_size)
  {
    offset = 0;
    size = a_size;
    buffer = a_buffer;

    while (process_data());

    return true;
  }

  protected:
  char*  buffer;
  size_t offset;
  size_t size;
  handler_t * handler;

  char * attributes[1024][2];

  //virtual void debug(int idx, const char * str)
  //{
  //  //putc('[', stdout);
  //  //for (int i = 0; i < 200; i++) 
  //  //  if (isspace(buffer[i]))
  //  //    putc(' ', stdout);
  //  //  else
  //  //    if (!buffer[i])
  //  //      putc('#', stdout);
  //  //    else
  //  //      putc(buffer[i], stdout);

  //  //printf("]\n %*c %s\n", idx+1, '^', str);
  //}

  virtual bool process_data()
  {
    int start = offset;

    //debug(start, "processing");

    // Processing character data 
    while (buffer[offset] != '<' && offset < start + size)
      offset++;

    if (offset - start)
    {
      handler->characters(buffer, start, offset - start);
      size -= offset - start;
      return true;
    }

    int gt = find('>', offset + size);
    if (gt == (int)(offset + size))
      return false;

    if (buffer[offset + 1] == '/')
    { // End tagname
      int eot = find(' ', gt);

      char c = buffer[eot];
      buffer[eot] = 0;
      handler->end_element(buffer + offset + 2);
      buffer[eot] = c;

      offset = gt + 1;
      size -= offset - start;
      return true;
    }
    else
    { // Start tagname
      int eot = find(' ', gt);
      if (eot == gt) eot = find('/', gt);

      //char c = buffer[eot];
      bool self_closing = (buffer[gt - 1] == '/');

      //debug(eot, "setting up");
      setup_attributes(eot, gt);
      //debug(gt, "set up");

      buffer[eot] = 0;
      handler->start_element(buffer + offset + 1, attributes);

      if (self_closing)
        handler->end_element(buffer + offset + 1);

      //buffer[eot] = c;
      offset = gt + 1;
      size -= offset - start;
      return true;
    }

    return false;
  }

  void setup_attributes(int start, int end)
  {
    int nattrs = 0;
    while (start < (int)(offset + size))
    {
      int skey, ekey;
      int sval, eval;

      //debug(start, "next attribute");
      while (isspace(buffer[start])) start++;
      skey = start;

      if (buffer[start] == '/' || buffer[start] == '>' || buffer[start] == '?')
      {
        attributes[nattrs][0] = attributes[nattrs][1] = 0;
        return;
      }

      while (!isspace(buffer[start]) && buffer[start] != '=') start++;
      ekey = start;

      while (isspace(buffer[start]) || buffer[start] == '=') start++;
      sval = ++start;

      for (char del = buffer[start - 1]; buffer[start] != del; start++)
        if (buffer[start] == '\\')
          start++;

      eval = start++;

      buffer[ekey] = 0;
      buffer[eval] = 0;

      attributes[nattrs][0] = buffer + skey;
      attributes[nattrs][1] = buffer + sval;
      nattrs++;
    }
  }

  int find(char ch, size_t limit) const
  {
    for (size_t i = offset; i < limit; i++)
      if (buffer[i] == ch)
        return i;

    return limit;
  }

};

}

#endif // _LIGHTSAX_H_

