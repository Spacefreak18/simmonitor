#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtar.h>
#include <sys/stat.h>
#include <unistd.h>

void extract_tarball(const char *tarball_path, const char *extract_dir) {
    TAR *tar;
    int result;

    // Open the tarball
    result = tar_open(&tar, tarball_path, NULL, 0, 0, TAR_GNU);
    if (result != 0) {
        perror("tar_open");
        exit(EXIT_FAILURE);
    }

    // Change directory to the extraction directory
    if (chdir(extract_dir) != 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    // Extract the tarball
    result = tar_extract_all(tar, NULL);
    if (result != 0) {
        perror("tar_extract_all");
        exit(EXIT_FAILURE);
    }

    // Close the tarball
    result = tar_close(tar);
    if (result != 0) {
        perror("tar_close");
        exit(EXIT_FAILURE);
    }

    printf("Extraction complete.\n");
}
