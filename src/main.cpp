#include "compiler.h"
#include "html.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

static char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';
  fclose(file);
  return buffer;
}

static void write_file(const char *path, const std::string &contents) {
  FILE *out = fopen(path, "wb");
  if (out == NULL) {
    fprintf(stderr, "Could not open output file \"%s\".\n", path);
    exit(74);
  }
  fwrite(contents.data(), 1, contents.size(), out);
  fclose(out);
}

static void copy_file(const char *src, const char *dst) {
  char *contents = read_file(src);
  FILE *out = fopen(dst, "wb");
  if (out == NULL) {
    fprintf(stderr, "Could not open output file \"%s\".\n", dst);
    exit(74);
  }
  fputs(contents, out);
  fclose(out);
  free(contents);
}

static const char *static_assets[] = {"style.css"};

int main() {
  fs::remove_all("docs");
  fs::create_directories("docs/blog");
  fs::create_directories("docs/assets");

  std::vector<Post> posts;

  for (const auto &entry : fs::directory_iterator("blog")) {
    if (entry.path().extension() != ".md")
      continue;

    char *source = read_file(entry.path().c_str());
    Post post;
    try {
      post = compile(source);
    } catch (const std::exception &e) {
      fprintf(stderr, "skipping %s: %s\n", entry.path().c_str(), e.what());
      free(source);
      continue;
    }
    free(source);

    std::string filename = entry.path().stem().string() + ".html";
    std::string out_path = "docs/blog/" + filename;
    write_file(out_path.c_str(), post.html);
    printf("compiled %s -> %s\n", entry.path().c_str(), out_path.c_str());

    for (int j = 0; j < post.asset_count; j++) {
      fs::path src = fs::path("assets") / post.asset_filenames[j];
      std::string dst = "docs/assets/" + src.filename().string();
      fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
      printf("copied asset %s -> %s\n", src.c_str(), dst.c_str());
    }

    post.filename = filename;
    posts.push_back(post);
  }

  std::sort(posts.begin(), posts.end(),
            [](const Post &a, const Post &b) { return a.date > b.date; });

  write_file("docs/index.html", generate_index());
  printf("generated docs/index.html (%zu posts)\n", posts.size());

  write_file("docs/blog.html", generate_blog_index(posts));
  printf("generated docs/blog.html (%zu posts)\n", posts.size());

  for (const char *asset : static_assets) {
    char dst[256];
    snprintf(dst, sizeof(dst), "docs/%s", asset);
    copy_file(asset, dst);
  }
}
