/*Написать программу, находящую в заданном каталоге и всех его подкаталогах все файлы,
 * заданного размера. Имя каталога задаётся пользователем в качестве первого аргумента командной строки.
 * Диапазон от N1 до N2 задается в аргументах командной строки. Программа выводит результаты поиска в файл
 * (четвертый аргумент командной строки) в виде полный путь, имя файла, его размер. На консоль выводится
 * общее число просмотренных файлов.*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

void find_files(char *root_path, int from_size, int to_size);

char *get_full_path(char *parent, char *name);

void print_error(const char *s_name, const char *msg, const char *desc, const char *path);

int isNumber(char *str);

int counter = 0;
char *program_name;
FILE *to_write;
int isNumber(char *str);

int main(int argc, char *argv[]) {
    program_name = basename(argv[0]);
    if (argc < 5) {
        print_error(program_name, "Incorrect amount of arguments.", 0, NULL);
        return 1;
    }
    char *dir_name = realpath(argv[1], NULL);
    if (dir_name == NULL) {
        print_error(program_name, "Error opening directory:", argv[1], NULL);
        return 1;
    }
    if (!isNumber(argv[2]) || !isNumber(argv[3])) {
        print_error(program_name, "Incorrect sizes.", NULL, NULL);
        return 1;
    }
    if ((to_write = fopen(argv[4], "w")) == NULL) {
        print_error(program_name, "Can't open file:", argv[4], NULL);
        return 1;
    }
    find_files(dir_name, atoi(argv[2]), atoi(argv[3]));
    if (fclose(to_write) == -1)
        print_error(program_name, "Error closing file:", realpath(argv[2], NULL), NULL);
    printf("Amount of checked files:%d\n", counter);
    return 0;	

}

int isNumber(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9')
            return 0;
    }
    return 1;
}

void find_files(char *root_path, int from_size, int to_size) {
    struct dirent *structdirent;
    char *path;
    errno = 0;
    DIR *curr_dir;
    if ((curr_dir = opendir(root_path)) == NULL) {
        path = get_full_path(root_path, structdirent->d_name);
        print_error(program_name, strerror(errno), NULL, path);
        return;
    }
    while ((structdirent = readdir(curr_dir))) {
        path = get_full_path(root_path, structdirent->d_name);
        if (structdirent->d_type == DT_DIR &&
            strcmp(".", structdirent->d_name) && strcmp("..", structdirent->d_name)) {
            find_files(path, from_size, to_size);
        } else if (structdirent->d_type == DT_REG) {
            struct stat *buf = (struct stat *) calloc(1, sizeof(struct stat));
            if (stat(path, buf) == -1) {
                print_error(program_name, strerror(errno), NULL, path);
            }
            if (buf->st_size > from_size && buf->st_size < to_size) {
                fprintf(to_write, "%s %s %ld\n", path, structdirent->d_name, buf->st_size);
            }
            counter++;
        }
        errno = 0;
    }
    if (errno != 0)
        print_error(program_name, strerror(errno), NULL, path);
    if (closedir(curr_dir) == -1) {
        print_error(program_name, strerror(errno), NULL, path);
    }
}

char *get_full_path(char *parent, char *name) {
    char *full_path = (char *) calloc
            (strlen(parent) + strlen(name) + 2, sizeof(char));
    strcpy(full_path, parent);
    strcat(full_path, "/");
    strcat(full_path, name);
    return full_path;
}

void print_error(const char *s_name, const char *msg, const char *desc, const char *path) {
    fprintf(stderr, "%s: %s %s in file %s\n", s_name, msg, (desc) ? desc :"", path);
}
