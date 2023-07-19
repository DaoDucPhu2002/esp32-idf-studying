#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *get_password(char *str)
{
    char *password_ptr = strstr(str, "password=");
    if (password_ptr != NULL)
    {
        password_ptr += strlen("password=");

        // Đếm độ dài của password
        int password_length = 0;
        char *password_end = password_ptr;
        while (*password_end != '\0' && *password_end != '&')
        {
            password_end++;
            password_length++;
        }

        // Cấp phát bộ nhớ động cho chuỗi password và sao chép giá trị
        char *password = malloc((password_length + 1) * sizeof(char));
        strncpy(password, password_ptr, password_length);
        password[password_length] = '\0';

        return password;
    }

    return NULL; // Trường hợp không tìm thấy password
}

int main()
{
    char input[] = "ssid=Ba+Anh+Em&password=11111111";

    // In kết quả

    printf("Password: %s\n", get_password(input));

    return 0;
}
