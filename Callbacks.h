#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <string>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output);

#endif // CALLBACKS_H