#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

typedef const char *(*fstring_callback_t)(void *data, const char *name);

#define fstr_vt_null    0
#define fstr_vt_str     1
#define fstr_vt_int     2
#define fstr_vt_long    3

typedef struct {
    const char *name;
    char type;
    union {
        const char *s;
        int i;
        long l;
        float f;
        double d;
        fstring_callback_t cb;
    } value;
} fstr_value;

typedef struct{ int count; int size; fstr_value **values; } _fstr_header; 

#define fstr_list_init(VAR, SIZE)       _fstr_header VAR = {.count=0, .size=SIZE, .values = (fstr_value **)calloc(SIZE + 2, sizeof(fstr_value*))}
#define fstr_list_destroy(VAR)       free(VAR.values)


#define fstr_nstr(N, V)   &((fstr_value){.name=N, .type=fstr_vt_str, .value.s=V})
#define fstr_nint(N, V)   &((fstr_value){.name=N, .type=fstr_vt_int, .value.i=V})

#define fstr_str(X)       &((fstr_value){.name=#X, .type=fstr_vt_str, .value.s=X})
#define fstr_int(X)       &((fstr_value){.name=#X, .type=fstr_vt_int, .value.i=X})

#define fstr_end        NULL

#define fstr_list_add(p, v)      do {\
     if (p.count == p.size) {\
                p.size *= 2; \
                p.values = realloc(p.values, sizeof(fstr_value) * p.size);\
            }\
            p.values[p.count++] = v;\
  } while(0);


#define fstr_values   (fstr_value[])



int blfstring(char *buffer, size_t buffer_len, const char *format, fstr_value *values[]);
int bfstring(char *buffer, size_t buffer_len, const char *format, ...);
char *lfstring(const char *format, fstr_value *values[]);
char *fstring(const char *format, ...);

int main(int argc, char **argv)
{
    char *thing = "blah";
    int i=5;
    fstr_value *parameters[] = {
        fstr_nstr("thingy", thing),
        fstr_str(thing),
        fstr_end
    };

    struct st { int x; };

    struct st *x;

    x = (&(struct st){ .x = 1});

    fstr_list_init(l,10);
    //fstr_list_add(l, fstr_str(thing));
    //fstr_list_add(l, fstr_int(i));
    char *result = lfstring("blah {thing} {i}", parameters);

    result = lfstring("blah {thing} {i}", (fstr_value *[]){
        fstr_str(thing), 
        fstr_int(i)
    });
    fstr_list_destroy(l);
    puts(result);
}