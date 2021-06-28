# libfstring
A templated string function written for C (Think python's fstring)

## Introduction

Have you been coding away in ANSI C and suddenly thought, "gee I wish i could use Python's f strings here? 
Well wonder no longer -- you can!

## Examples

The best way to start is by example:

```
int total_money = 529;
float pi = 3.14159
char *boss_description = "lazy";

char *output;

output = fstring("We have ${total_money}, because the boss is {boss_description}. Also, pi is {pi}. Don't forget to {that thing}",
              fstr_str(boss_description), 
              fstr_int(total_money),
              fstr_float(pi),
              fstr_nstr("that thing", "write code"), 
              fstr_end);
              
puts(output);
/* Outputs:
   We have $529, because the boss is lazy. Also, pi is 3.14159
 */
free(output);
```

Well how about that? 

The key points to note: You have to specify which variables to make available, and their type. You can also name a variable
by using the fstr_nstr (and other types preceded with "n"). So fstr_nlong("thing", 123456) creates a variable named {thing}.

The output of fstring is a malloc()'d string. But you can also call bfstring, which is like snprintf in that it takes a buffer.

```
static char buffer[1024];
bfstring(buffer, sizeof(buffer), "{what} things are {doing}", fstr_nstr("what", "Magic"), fstr_nstr("doing", "happening");
/* Buffer contains: "Magic things are happening */
```

This library wouldn't be that useful if you always had to know exactly what variables you want, so it also supports passing
an array in (both the malloc()'d and buffer versions.

```
fstr_value *info[] = {
    fstr_str(thing),
    fstr_int(counter)
}
output = fstring("{thing} is {counter}", fstr_list(info));
```

