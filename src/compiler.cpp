#include "compiler.h"
#include "html.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>

void build_body(Element *elements, int count, std::string &out,
                std::string *assets, int &asset_count, const std::string &title,
                const std::string &date) {

  out += "  <article class=\"post-body\">\n";
  out += "    <h1 class=\"post-title\">" + title + "</h1>\n";
  out += "    <time class=\"post-date\" datetime=\"" + date + "\">" + date +
         "</time>\n";
  for (int i = 0; i < count; i++) {
    switch (elements[i].type) {
    case HEADING_ONE:
      out += "    <h1>";
      out.append(elements[i].data.text.txt, elements[i].data.text.length);
      out += "</h1>\n";
      break;
    case HEADING_TWO:
      out += "    <h2>";
      out.append(elements[i].data.text.txt, elements[i].data.text.length);
      out += "</h2>\n";
      break;
    case HEADING_THREE:
      out += "    <h3>";
      out.append(elements[i].data.text.txt, elements[i].data.text.length);
      out += "</h3>\n";
      break;
    case HEADING_FOUR:
      out += "    <h4>";
      out.append(elements[i].data.text.txt, elements[i].data.text.length);
      out += "</h4>\n";
      break;
    case HEADING_FIVE:
      out += "    <h5>";
      out.append(elements[i].data.text.txt, elements[i].data.text.length);
      out += "</h5>\n";
      break;
    case PARAGRAPH: {
      const Paragraph &para = elements[i].data.paragraph;
      out += "    <p>";
      for (int j = 0; j < para.count; j++) {
        const Element &c = para.children[j];
        switch (c.type) {
        case TEXT:
          out.append(c.data.text.txt, c.data.text.length);
          break;
        case BOLD:
          out += "<strong>";
          out.append(c.data.text.txt, c.data.text.length);
          out += "</strong>";
          break;
        case ITALIC:
          out += "<em>";
          out.append(c.data.text.txt, c.data.text.length);
          out += "</em>";
          break;
        case LINK:
          out += "<a href=\"";
          out.append(c.data.link.href, c.data.link.href_len);
          out += "\"";
          if (c.data.link.title_len > 0) {
            out += " title=\"";
            out.append(c.data.link.title, c.data.link.title_len);
            out += "\"";
          }
          out += ">";
          out.append(c.data.link.text, c.data.link.text_len);
          out += "</a>";
          break;
        default:
          break;
        }
      }
      out += "</p>\n";
      break;
    }
    case CODE_BLOCK: {
      const Code_Block &cb = elements[i].data.code;
      out += "    <pre><code";
      if (cb.language_len > 0) {
        out += " class=\"language-";
        out.append(cb.language, cb.language_len);
        out += "\"";
      }
      out += ">";
      for (int j = 0; j < cb.length; j++) {
        switch (cb.txt[j]) {
        case '&':
          out += "&amp;";
          break;
        case '<':
          out += "&lt;";
          break;
        case '>':
          out += "&gt;";
          break;
        default:
          out += cb.txt[j];
          break;
        }
      }
      out += "</code></pre>\n";
      break;
    }
    case IMAGE: {
      const Image &img = elements[i].data.image;
      out += "    <img src=\"../assets/";
      out.append(img.src, img.src_len);
      out += "\" alt=\"";
      out.append(img.alt_txt, img.alt_len);
      out += "\"";
      if (img.title_len > 0) {
        out += " title=\"";
        out.append(img.title, img.title_len);
        out += "\"";
      }
      out += " />\n";
      assets[asset_count++] = std::string(img.src, img.src_len);
      break;
    }
    case VIDEO: {
      const Image &v = elements[i].data.image;
      out += "    <video autoplay loop muted playsinline";
      if (v.title_len > 0) {
        out += " poster=\"../assets/";
        out.append(v.title, v.title_len);
        out += "\"";
      }
      out += ">\n      <source src=\"../assets/";
      out.append(v.src, v.src_len);
      out += "\" type=\"video/";
      out += (v.src_len >= 5 && strncmp(v.src + v.src_len - 5, ".webm", 5) == 0)
                 ? "webm"
                 : "mp4";
      out += "\">\n    </video>\n";
      assets[asset_count++] = std::string(v.src, v.src_len);
      break;
    }
    default:
      break;
    }
  }
  out += "  </article>\n";
}

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
  out += "  <footer class=\"post-footer\"><p>Generated in " +
         std::to_string(compile_us) + " \xce\xbcs</p></footer>\n";
  out += "</main>\n";
  out += "</body>\n";
  out += "</html>\n";
  free_elements(elements, count);
  post.html = std::move(out);
  return post;
}
