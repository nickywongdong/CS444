

struct item {
  int32_t value;
  int32_t wait;
};

# ifndef TYPE
# define TYPE struct item
# define TYPE_SIZE sizeof(struct item)
# endif
