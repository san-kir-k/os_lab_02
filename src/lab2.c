// Киреев А.К., 206 группа
// 4 вариант

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 64
#define RF_EOF 1
#define RF_INVALID 2
#define RF_VALID 0

// проигнорировать невалидную строку
void skip_str() {
    char ch;
    while (read(0, &ch, 1) > 0 && ch != '\n') {}
}

// считать имя файла с клавиатуры при помощи read
void read_file_name(char file_name[BUF_SIZE]) {
    char ch;
    int count = 0;
    while (read(0, &ch, 1) > 0 && ch != '\n') {
        file_name[count++] = ch;
    }
    file_name[count] = '\0';
}

// проверка числа на валидность
bool is_valid(char* buf) {
    int i = 0;
    int count_of_dots = 0;
    while (buf[i] != '\0') {
        if (isdigit(buf[i]) || buf[i] == '.') {
            if (buf[i] == '.') {
                count_of_dots++;
                i++;
            } else {
                i++;
                continue;
            }
        } else {
            return false;
        }
    }
    return (count_of_dots <= 1 && i > 0) ? true : false;
}

// считать float с клавиатуры при помощи read
int read_float(float* n) {
    char ch;
    char buf[100];
    int count = 0;
    // проверка на еоф (read возвращает 0, если еоф)
    int is_eof = 1;
    float sign = 1.0;
    while ((is_eof = read(0, &ch, 1)) > 0 && ch != ' ' && ch != '\n') {
        if (ch == '-') {
            sign *= -1;
        } else {
            buf[count++] = ch;
        }
    }
    if (is_eof == 0) {
        return RF_EOF;
    }
    buf[count] = '\0';
    if (!is_valid(buf)) {
        if (ch != '\n')
            skip_str();
        return RF_INVALID; // строка невалидна
    }
    *n = atof(buf) * sign;
    return RF_VALID;
}

// считать три float в строке через разделитель
int read_3_float(float* num) {
    // проверка на еоф (если хоть одна функция считывания словила еоф)
    int from_read = 1;
    int i = 0;
    while (i < 3) {
        from_read = read_float(&num[i]);
        if (from_read == RF_INVALID || from_read == RF_EOF) {
            if (from_read == RF_EOF) {
                return RF_EOF;
            } else {
                return RF_INVALID;
            }
        }
        i++;
    }
    return RF_VALID;
}

// закрывает канал
void close_pipe(int pipe[2]) {
    close(pipe[0]);
    close(pipe[1]);
}

int parent_proc(int pipe1[2], int pipe2[2]) {
    close(pipe1[0]);
    close(pipe2[1]);
    float num[3];
    // значение об окончании процесса, которое подается по второму каналу от ребенка к родителю 
    bool kill_sig = false;
    int read_ans = RF_VALID;
    while ((read_ans = read_3_float(num)) != RF_EOF) {
        if (read_ans == RF_INVALID) {
            continue;
        }
        write(pipe1[1], &num, 3 * sizeof(float));
        read(pipe2[0], &kill_sig, sizeof(bool));
        if (kill_sig) {
            close(pipe1[1]);
            close(pipe2[0]);
            exit(2);
        }
    }
    close(pipe1[1]);
    close(pipe2[0]);
    exit(0);
} 

int child_proc(char* file_name, int pipe1[2], int pipe2[2]) {
    close(pipe1[1]);
    close(pipe2[0]);
    float num[3];
    bool kill_sig = false;
    int fd = open(file_name, O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
    if (fd < 0) {
        perror("Ubable to create file.");
        close(pipe1[0]);
        close(pipe2[1]);
        exit(1);
    }
    while (read(pipe1[0], &num, 3 * sizeof(float)) > 0 ) {
        // если делим на ноль, то оправляем родителю сообщение прекратить работу
        if (num[1] * num[2] == 0.0) {
            kill_sig = true;
            write(pipe2[1], &kill_sig, sizeof(bool));
            close(fd);
            close(pipe1[0]);
            close(pipe2[1]);
            exit(2);
        }
        write(pipe2[1], &kill_sig, sizeof(bool));
        float res = num[0] / (num[1] * num[2]);
        // конвертируем флоат в строку для записи в файл
        char float_str[100];
        sprintf(float_str, "%f", res);
        int count = 0;
        while (float_str[count] != '\0') {
            write(fd, &float_str[count++], 1);
        }
        char endl = '\n';
        write(fd, &endl, 1);
    }
    close(fd);
    close(pipe1[0]);
    close(pipe2[1]);
    exit(0);
}

int main() {
    char file_name[BUF_SIZE];
    read_file_name(file_name);
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) != 0) {
        perror("Unable to create a pipe1.");
        exit(1);
    }
    if (pipe(pipe2) != 0) {
        perror("Unable to create a pipe2.");
        close_pipe(pipe1);
        exit(1);
    }
    int id = fork();
    if (id < 0) {
        close_pipe(pipe1);
        close_pipe(pipe2);
        perror("Fork error.");
        exit(1);
    } else if (id == 0) {
        child_proc(file_name, pipe1, pipe2);
    } else {
        parent_proc(pipe1, pipe2);
    }
}
