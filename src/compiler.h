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

Post compile(char *source);
#endif
