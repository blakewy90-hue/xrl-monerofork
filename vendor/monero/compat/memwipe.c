#include <windows.h>
#include <stddef.h>

void memwipe(void *data, size_t size)
{
    if (data != NULL && size != 0) {
        SecureZeroMemory(data, size);
    }
}
