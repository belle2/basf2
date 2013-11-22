#include <stdio.h>
#include <svd/online/SVDOnlineToOfflineMap.h>

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Hello! Hello!\n");
    printf("Usage: %s filename.xml\n read filename.xml\n", argv[0]);
    return 0;
  }

  Belle2::SVDOnlineToOfflineMap test(argv[1]);
}
