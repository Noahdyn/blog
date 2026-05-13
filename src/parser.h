#ifndef PARSER_H
#define PARSER_H

enum Element_Type {
  HEADING_ONE,
  HEADING_TWO,
  HEADING_THREE,
  HEADING_FOUR,
  HEADING_FIVE,
  PARAGRAPH,
  METADATA,
  CODE_BLOCK,
  IMAGE,
  VIDEO,
  LINK,
  BOLD,
  ITALIC,
  TEXT,
};

struct Meta_Data {
  const char *title;
  int title_len;
  const char *date;
  int date_len;
  const char *header_img_src;
  int header_img_len;
};

struct Text_Element {
  const char *txt;
  int length;
};

struct Code_Block {
  const char *txt;
  int length;
  const char *language;
  int language_len;
};

struct Image {
  const char *src;
  int src_len;
  const char *alt_txt;
  int alt_len;
  const char *title;
  int title_len;
};

struct Link {
  const char *href;
  int href_len;
  const char *text;
  int text_len;
  const char *title;
  int title_len;
};

struct Element;

struct Paragraph {
  Element *children;
  int count;
};

struct Element {
  Element_Type type;
  union {
    Meta_Data meta;
    Text_Element text;
    Code_Block code;
    Image image;
    Link link;
    Paragraph paragraph;
  } data;
};

Element *parse(char *source, int *out_count);
void free_elements(Element *elements, int count);
void debug_elements(Element *elements, int count);

#endif
