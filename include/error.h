#ifndef ERROR_H
#define ERROR_H

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY (x)
#define LOCATION __FILE__ ":" TOSTRING (__LINE__)

void error (const char *location, const char *msg);

#endif // ERROR_H