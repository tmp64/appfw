#include <cstdio>
#include <appfw/appfw.h>
#include <appfw/init.h>

int main(int argc, char **argv) {
    appfw::InitComponent appfwInit(appfw::InitOptions().setArgs(argc, argv));
    printf("Hello, world!\n");
}
