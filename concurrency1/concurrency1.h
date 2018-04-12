

struct item {
  int value;
  int wait;
};

# ifndef TYPE
# define TYPE struct item
# define TYPE_SIZE sizeof(struct item)
# endif
