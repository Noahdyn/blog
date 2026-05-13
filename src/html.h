#ifndef HTML_H
#define HTML_H
#include "parser.h"
#include "compiler.h"
#include <string>
#include <vector>

void build_header(Meta_Data meta, std::string &out);
std::string generate_index();
std::string generate_blog_index(const std::vector<Post> &posts);
#endif
