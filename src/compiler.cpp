#include "compiler.h"
#include "html.h"
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>

Post compile(char *source) {
  clock_t begin = clock();
  int count;
  Element *elements = parse(source, &count);
  std::string out;

  Post post;
  int start = 0;
  if (count > 0 && elements[0].type == METADATA) {
    Meta_Data &meta = elements[0].data.meta;
    post.title = std::string(meta.title, meta.title_len);
    post.date = std::string(meta.date, meta.date_len);
    post.header_img = std::string(
        meta.header_img_src ? meta.header_img_src : "", meta.header_img_len);
    build_header(meta, out);
    start = 1;
  } else {
    throw std::runtime_error("Metadata required");
  }

  post.asset_count = 0;
  if (!post.header_img.empty())
    post.asset_filenames[post.asset_count++] = post.header_img;
  build_body(elements + start, count - start, out, post.asset_filenames,
             post.asset_count, post.title, post.date);
  clock_t end = clock();
  post.compile_time = (double)(end - begin) / CLOCKS_PER_SEC;
  long long compile_us = (long long)(end - begin) * 1000000LL / CLOCKS_PER_SEC;
  build_footer(compile_us, out);
  free_elements(elements, count);
  post.html = std::move(out);
  return post;
}
