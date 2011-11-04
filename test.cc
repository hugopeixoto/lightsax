
#include "light_sax.h"

class printer_handler : public light_sax::handler_t
{
  public:
  printer_handler ();
  virtual void start_element (char* a_name, char* a_attributes[][2]);
  virtual void end_element (char* a_name);
  virtual void characters (char* ch, int start, int length);

  protected:
  unsigned int depth_;
};

printer_handler::printer_handler ()
{
  depth_ = 0;
}

void printer_handler::start_element (char* a_name, char* a_attributes[][2])
{
  printf("%*sOpening tag: %s\n", 2*depth_, "", a_name);
  ++depth_;
}

void printer_handler::end_element (char* a_name)
{
  --depth_;
  printf("%*sEnding tag: %s\n", 2*depth_, "  ", a_name);
}

void printer_handler::characters (char* a_characters, int start, int length)
{
}

/**
 * @brief Reads an XML document from stdin 
 */
int main ()
{
  char msg[] = "<pokemon>hello <img/></pokemon>";
  printer_handler handler;
  light_sax::parser_t parser(&handler);
  parser.parse(stdin);
  parser.parse(msg);
  return 0;
}

