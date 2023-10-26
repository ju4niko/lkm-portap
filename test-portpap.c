#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/lkm_portap"

int main() {
    int fd;
    char write_buffer[] = "HOLA";
    char read_buffer[1024]; // Tamaño suficiente para leer datos

    // Abre el dispositivo
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("No se pudo abrir el dispositivo");
        return -1;
    }

    // Escribe "HOLA" en el dispositivo
    ssize_t bytes_written = write(fd, write_buffer, strlen(write_buffer));
    if (bytes_written < 0) {
        perror("Error al escribir en el dispositivo");
        close(fd);
        return -1;
    }
    printf("Se escribieron %zd bytes en el dispositivo\n", bytes_written);

    // Lee datos del dispositivo
    ssize_t bytes_read = read(fd, read_buffer, sizeof(read_buffer));
    if (bytes_read < 0) {
        perror("Error al leer del dispositivo");
        close(fd);
        return -1;
    }
    read_buffer[bytes_read] = '\0'; // Agrega el carácter nulo para imprimir como cadena
    printf("Se leyeron %zd bytes del dispositivo: %s\n", bytes_read, read_buffer);

    // Cierra el dispositivo
    close(fd);

    return 0;
}
