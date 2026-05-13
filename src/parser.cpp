#include "parser.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

typedef struct {
  char *current;
  Element *elements;
  int capacity;
  int count;
} Parser;

Parser parser;

#define INITIAL_CAPACITY 32

void push_element(Element elem) {
  if (parser.count >= parser.capacity) {
    parser.capacity *= 2;
    parser.elements =
        (Element *)realloc(parser.elements, parser.capacity * sizeof(Element));
  }
  parser.elements[parser.count++] = elem;
}

char advance() {
  parser.current++;
  return parser.current[-1];
}

char peek() { return *parser.current; }

char peek_next() { return *(parser.current + 1); }

bool match_range(const char *range, int len) {
  for (int i = 0; i < len; i++) {
    char c = advance();
    if (c != range[i]) {
      return false;
    }
  }
  return true;
}

void heading() {
  int level = 1;
  while (peek() == '#') {
    level++;
    advance();
  }
  // leading space
  advance();
  char *start = parser.current;
  int length = 0;
  while (advance() != '\n') {
    length++;
  }

  Element_Type type =
      static_cast<Element_Type>(level <= 5 ? level - 1 : HEADING_FIVE);

  Element elem = Element{
      .type = type,
      .data = {.text =
                   Text_Element{
                       .txt = start,
                       .length = length,
                   }},
  };
  push_element(elem);
}

void metadata() {
  if (!match_range("--", 2)) {
    throw std::runtime_error("Expected ---");
  }
  if (advance() != '\n') {
    throw std::runtime_error("Expected newline after \"---\"");
  }

  Meta_Data meta = {};

  while (peek() != '\0' && !(peek() == '-' && parser.current[1] == '-' &&
                             parser.current[2] == '-')) {
    char *key_start = parser.current;
    while (peek() != ':')
      advance();
    advance(); // ':'
    advance(); // ' '

    char *value_start = parser.current;
    int value_len = 0;
    while (peek() != '\n' && peek() != '\0') {
      advance();
      value_len++;
    }
    if (peek() == '\n')
      advance();

    switch (*key_start) {
    case 't':
      meta.title = value_start;
      meta.title_len = value_len;
      break;
    case 'd':
      meta.date = value_start;
      meta.date_len = value_len;
      break;
    case 'h':
      meta.header_img_src = value_start;
      meta.header_img_len = value_len;
      break;
    }
  }
  if (peek() == '\0') {
    throw std::runtime_error("Unexpected End of File");
  }

  match_range("---", 3);
  if (peek() == '\n')
    advance();

  Element elem = Element{
      .type = METADATA,
      .data = {.meta = meta},
  };
  push_element(elem);
}

void code() {
  if (!match_range("``", 2))
    throw std::runtime_error("Expected ```");

  const char *lang_start = parser.current;
  int lang_len = 0;
  while (peek() != '\n' && peek() != '\0') {
    advance();
    lang_len++;
  }
  if (peek() == '\n')
    advance();

  const char *code_start = parser.current;
  int code_len = 0;
  while (peek() != '\0') {
    if (peek() == '`' && parser.current[1] == '`' && parser.current[2] == '`')
      break;
    advance();
    code_len++;
  }
  if (peek() == '\0')
    throw std::runtime_error("Unterminated code block");

  match_range("```", 3);
  if (peek() == '\n')
    advance();

  Code_Block cb = {};
  cb.txt = code_start;
  cb.length = code_len;
  cb.language = lang_start;
  cb.language_len = lang_len;

  Element elem = Element{
      .type = CODE_BLOCK,
      .data = {.code = cb},
  };
  push_element(elem);
}

void image() {
  Image img = {};
  if (peek() != '[') {
    throw std::runtime_error("Expected '[' in image");
  }
  advance();
  char *alt_start = parser.current;
  int alt_len = 0;
  while (peek() != ']') {
    if (peek() == '\0')
      throw std::runtime_error("Unexpected End of File");
    advance();
    alt_len++;
  }
  advance(); // ']'
  img.alt_txt = alt_start;
  img.alt_len = alt_len;

  if (peek() != '(')
    throw std::runtime_error("Expected '(' after alt in image");
  advance(); // '('

  char *src_start = parser.current;
  int src_len = 0;
  while (peek() != ')' && !(peek() == ' ' && peek_next() == '"')) {
    if (peek() == '\0')
      throw std::runtime_error("Unexpected End of File");
    advance();
    src_len++;
  }
  img.src = src_start;
  img.src_len = src_len;

  img.title = nullptr;
  img.title_len = 0;
  if (peek() == ' ' && peek_next() == '"') {
    advance(); // ' '
    advance(); // '"'
    char *title_start = parser.current;
    int title_len = 0;
    while (peek() != '"') {
      if (peek() == '\0')
        throw std::runtime_error("Unexpected End of File");
      advance();
      title_len++;
    }
    advance(); // closing '"'
    img.title = title_start;
    img.title_len = title_len;
  }

  if (peek() != ')')
    throw std::runtime_error("Expected ')' to close image");
  advance(); // ')'

  bool is_video = (img.src_len >= 4 &&
                   (strncmp(img.src + img.src_len - 4, ".mp4", 4) == 0 ||
                    (img.src_len >= 5 &&
                     strncmp(img.src + img.src_len - 5, ".webm", 5) == 0)));

  Element elem = Element{
      .type = is_video ? VIDEO : IMAGE,
      .data = {.image = img},
  };
  push_element(elem);
}

bool is_element_start(char c) { return c == '#' || c == '`' || c == '-'; }

void skip_whitespace() {
  while (peek() == '\n' || peek() == '\r' || peek() == ' ' || peek() == '\t')
    advance();
}

static Element *parse_inline(const char *start, int length, int *out_count) {
  int capacity = 8, count = 0;
  Element *children = (Element *)malloc(capacity * sizeof(Element));

  auto push = [&](Element e) {
    if (count >= capacity) {
      capacity *= 2;
      children = (Element *)realloc(children, capacity * sizeof(Element));
    }
    children[count++] = e;
  };

  auto push_text = [&](const char *txt, int len) {
    if (len <= 0)
      return;
    push(Element{TEXT, {.text = Text_Element{txt, len}}});
  };

  int i = 0, run_start = 0;

  while (i < length) {
    // Bold: **text**
    if (i + 1 < length && start[i] == '*' && start[i + 1] == '*') {
      int j = i + 2;
      while (j + 1 < length && !(start[j] == '*' && start[j + 1] == '*'))
        j++;
      if (j + 1 < length) {
        push_text(start + run_start, i - run_start);
        push(Element{BOLD, {.text = Text_Element{start + i + 2, j - (i + 2)}}});
        i = j + 2;
        run_start = i;
        continue;
      }
    }
    // Italic: *text*
    if (start[i] == '*') {
      int j = i + 1;
      while (j < length && start[j] != '*')
        j++;
      if (j < length) {
        push_text(start + run_start, i - run_start);
        push(Element{ITALIC,
                     {.text = Text_Element{start + i + 1, j - (i + 1)}}});
        i = j + 1;
        run_start = i;
        continue;
      }
    }
    // Link: [text](href "optional title")
    if (start[i] == '[') {
      int j = i + 1;
      while (j < length && start[j] != ']')
        j++;
      if (j < length && j + 1 < length && start[j + 1] == '(') {
        int href_start = j + 2, k = href_start;
        while (k < length && start[k] != ')' &&
               !(start[k] == ' ' && k + 1 < length && start[k + 1] == '"'))
          k++;
        int href_end = k;
        const char *title = nullptr;
        int title_len = 0;
        if (k < length && start[k] == ' ' && k + 1 < length &&
            start[k + 1] == '"') {
          k += 2;
          int ts = k;
          while (k < length && start[k] != '"')
            k++;
          title = start + ts;
          title_len = k - ts;
          if (k < length)
            k++;
        }
        if (k < length && start[k] == ')') {
          push_text(start + run_start, i - run_start);
          Link lnk{};
          lnk.text = start + i + 1;
          lnk.text_len = j - (i + 1);
          lnk.href = start + href_start;
          lnk.href_len = href_end - href_start;
          lnk.title = title;
          lnk.title_len = title_len;
          push(Element{LINK, {.link = lnk}});
          i = k + 1;
          run_start = i;
          continue;
        }
      }
    }
    i++;
  }
  push_text(start + run_start, i - run_start);

  *out_count = count;
  return children;
}

void paragraph() {
  char *start = parser.current - 1;
  int length = 1;
  while (peek() != '\0') {
    if (peek() == '\n') {
      if (parser.current[1] == '\n' || parser.current[1] == '\0' ||
          is_element_start(parser.current[1])) {
        advance();
        break;
      }
    }
    advance();
    length++;
  }

  int inline_count = 0;
  Element *children = parse_inline(start, length, &inline_count);
  Element elem = Element{
      .type = PARAGRAPH,
      .data = {.paragraph =
                   Paragraph{.children = children, .count = inline_count}},
  };
  push_element(elem);
}

void free_elements(Element *elements, int count) {
  for (int i = 0; i < count; i++) {
    if (elements[i].type == PARAGRAPH)
      free(elements[i].data.paragraph.children);
  }
  free(elements);
}

void debug_elements(Element *elements, int count) {
  for (int i = 0; i < count; i++) {
    Element *e = &elements[i];
    switch (e->type) {
    case HEADING_ONE:
    case HEADING_TWO:
    case HEADING_THREE:
    case HEADING_FOUR:
    case HEADING_FIVE:
      printf("[H%d] %.*s\n", e->type + 1, e->data.text.length,
             e->data.text.txt);
      break;
    case PARAGRAPH: {
      const Paragraph &p = e->data.paragraph;
      printf("[PARAGRAPH] %d children\n", p.count);
      for (int j = 0; j < p.count; j++) {
        const Element &c = p.children[j];
        if (c.type == LINK) {
          printf("  [LINK] text=%.*s href=%.*s\n", c.data.link.text_len,
                 c.data.link.text, c.data.link.href_len, c.data.link.href);
        } else {
          const char *t = c.type == TEXT   ? "TEXT"
                          : c.type == BOLD ? "BOLD"
                                           : "ITALIC";
          printf("  [%s] %.*s\n", t, c.data.text.length, c.data.text.txt);
        }
      }
      break;
    }
    case METADATA:
      printf("[METADATA] title=%.*s date=%.*s url_title=%.*s\n",
             e->data.meta.title_len, e->data.meta.title, e->data.meta.date_len,
             e->data.meta.date, e->data.meta.header_img_len,
             e->data.meta.header_img_src);
      break;
    case CODE_BLOCK:
      printf("[CODE_BLOCK lang=%.*s] %.*s\n", e->data.code.length,
             e->data.code.language, e->data.code.length, e->data.code.txt);
      break;
    case IMAGE:
      printf("[IMAGE] src=%.*s alt=%.*s title=%.*s\n", e->data.image.src_len,
             e->data.image.src, e->data.image.alt_len, e->data.image.alt_txt,
             e->data.image.title_len, e->data.image.title);
      break;
    case VIDEO:
      printf("[VIDEO] src=%.*s alt=%.*s poster=%.*s\n", e->data.image.src_len,
             e->data.image.src, e->data.image.alt_len, e->data.image.alt_txt,
             e->data.image.title_len, e->data.image.title);
      break;
    default:
      break;
    }
  }
}

Element *parse(char *source, int *out_count) {
  parser = Parser{
      .current = source,
      .elements = (Element *)malloc(INITIAL_CAPACITY * sizeof(Element)),
      .capacity = INITIAL_CAPACITY,
      .count = 0,
  };
  while (*parser.current != '\0') {
    skip_whitespace();
    if (*parser.current == '\0')
      break;
    char c = advance();
    switch (c) {
    case '#':
      heading();
      break;
    case '`':
      code();
      break;
    case '-':
      metadata();
      break;
    case '!':
      image();
      break;
    default:
      paragraph();
      break;
    }
  }
  *out_count = parser.count;
  return parser.elements;
}
