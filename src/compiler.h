#ifndef LEXER_H
#define LEXER_H
#include "parser.h"
#include <string>

struct Post {
  std::string html;
  std::string title;
  std::string date;
  std::string header_img;
  std::string filename;
  std::string asset_filenames[20];
  double compile_time;
  int asset_count;
};

void build_body(Element *elements, int count, std::string &out,
                std::string *assets, int &asset_count, const std::string &title,
                const std::string &date);
Post compile(char *source);
#endif
