#include "html.h"
#include <cstring>
#include <string>

static std::string nav(const std::string &active, const std::string &prefix) {
  std::string out;
  out += "  <header class=\"post-header\">\n";
  out += "    <nav class=\"post-nav\">\n";
  const struct {
    std::string label;
    std::string href;
  } links[] = {
      {"About", prefix + "index.html"},
      {"Blog", prefix + "blog.html"},
  };
  for (const auto &l : links) {
    if (l.label == active)
      out += "      <a href=\"" + l.href + "\" aria-current=\"page\">" +
             l.label + "</a>\n";
    else
      out += "      <a href=\"" + l.href + "\">" + l.label + "</a>\n";
  }
  out += "    </nav>\n";
  out += "  </header>\n";
  return out;
}

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

void build_footer(long long compile_us, std::string &out) {
  out += "  <footer class=\"post-footer\"><p>Page generated in " +
         std::to_string(compile_us) + " \xce\xbcs</p></footer>\n";
  out += "</main>\n";
  out += "</body>\n";
  out += "</html>\n";
}

void build_header(Meta_Data meta, std::string &out) {
  std::string title(meta.title, meta.title_len);
  std::string date(meta.date, meta.date_len);

  out += "<!DOCTYPE html>\n";
  out += "<html lang=\"en\">\n";
  out += "<head>\n";
  out += "  <meta charset=\"UTF-8\">\n";
  out += "  <meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0, viewport-fit=cover\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\" "
         "media=\"(prefers-color-scheme: dark)\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\" "
         "media=\"(prefers-color-scheme: light)\">\n";
  out += "  <meta name=\"date\" content=\"" + date + "\">\n";
  out += "  <title>" + title + "</title>\n";
  out += "  <link rel=\"stylesheet\" href=\"../style.css\">\n";
  out += "  <link href=\"../prism.css\" rel=\"stylesheet\">\n";
  out += "</head>\n";
  out += "<body>\n";
  out += "  <script src=\"../prism.js\"></script>";
  out += "<main>\n";
  out += nav("Blog", "../");
}

std::string generate_index() {
  std::string out;
  out += "<!DOCTYPE html>\n";
  out += "<html lang=\"en\">\n";
  out += "<head>\n";
  out += "  <meta charset=\"UTF-8\">\n";
  out += "  <meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0, viewport-fit=cover\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\" "
         "media=\"(prefers-color-scheme: dark)\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\" "
         "media=\"(prefers-color-scheme: light)\">\n";
  out += "  <title>About</title>\n";
  out += "  <link rel=\"stylesheet\" href=\"style.css\">\n";
  out += "</head>\n";
  out += "<body>\n";
  out += "<main>\n";
  out += nav("About", "");
  out += "  <section class=\"post-body\">\n";
  out += "    <p>This is my personal blog.</p>\n";
  out += "    <p>I use this to write up about some personal projects I find "
         "noteworthy aswell as my understandings of various concepts in "
         "tech.</p>\n";
  out += "    <p>This Blog is generated by my own SSG, you can check it out "
         "<a href=\"https://github.com/Noahdyn/blog\">here</a>.</p>\n";
  out +=
      "    <p>All posts are written by myself and not produce of an llm.</p>\n";
  out += "    <p>Thanks for visiting!</p>\n";
  out += "  </section>\n";
  out += "</main>\n";
  out += "</body>\n";
  out += "</html>\n";
  return out;
}

std::string generate_blog_index(const std::vector<Post> &posts) {
  std::string out;
  out += "<!DOCTYPE html>\n";
  out += "<html lang=\"en\">\n";
  out += "<head>\n";
  out += "  <meta charset=\"UTF-8\">\n";
  out += "  <meta name=\"viewport\" content=\"width=device-width, "
         "initial-scale=1.0, viewport-fit=cover\">\n";
  out += "  <meta name=\"theme-color\" content=\"#e9dac5\">\n";
  out += "  <title>Blog</title>\n";
  out += "  <link rel=\"stylesheet\" href=\"style.css\">\n";
  out += "</head>\n";
  out += "<body>\n";
  out += "<main>\n";
  out += nav("Blog", "");
  out += "  <section class=\"post-body\">\n";
  for (const auto &p : posts) {
    out += "    <a href=\"blog/" + p.filename + "\" class=\"post-link\">\n";
    out += "      <span class=\"post-link-title\">" + p.title + "</span>\n";
    if (!p.date.empty())
      out += "      <time datetime=\"" + p.date + "\">" + p.date + "</time>\n";
    out += "    </a>\n";
  }
  out += "  </section>\n";
  out += "</main>\n";
  out += "</body>\n";
  out += "</html>\n";
  return out;
}
