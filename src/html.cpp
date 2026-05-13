#include "html.h"

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
  out += "    <p>All posts are written by myself and not llm-generated.</p>\n";
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
