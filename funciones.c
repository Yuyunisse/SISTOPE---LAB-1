#include "funciones.h"

int sum(int a, int b){
    return a+b;
}

int resta(int a, int b){
    return a-b;
}


//Ejemplo de lectura y escritura de imagen BMP en C

#pragma pack(push, 1) 
//cuando se trabaja con uint se usa para leer directamente los bytes de la imagen, push hace que la estructura se lea byte por byte y pop hace que se lea de forma normal
typedef struct {
    // BMP Header | Tipo de dato por bits | Descripción
    uint16_t type; // Tipo de dato, tiene 2 bytes y es un número que indica si el archivo es BMP a través de las siglas BM
    uint32_t size; // Tamaño del archivo, tiene 4 bytes y es un número que indica el tamaño del archivo en bytes
    uint16_t reserved1; // Reservado, tiene 2 bytes y es un número que no se utiliza
    uint16_t reserved2; // Reservado, tiene 2 bytes y es un número que no se utiliza
    uint32_t offset; // Offset, tiene 4 bytes y es un número que indica la posición en bytes donde comienza la información de la imagen
} BMPHeader;

typedef struct {
    uint32_t size; // Tamaño de la información de la imagen, tiene 4 bytes y es un número que indica el tamaño de la información de la imagen en bytes
    int32_t width; // Ancho de la imagen, tiene 4 bytes y es un número que indica el ancho de la imagen en píxeles
    int32_t height; // Alto de la imagen, tiene 4 bytes y es un número que indica el alto de la imagen en píxeles
    uint16_t planes; // Planos, tiene 2 bytes y es un número que indica la cantidad de planos de color, debe valer 1
    uint16_t bit_count; // Bits por píxel, tiene 2 bytes y es un número que indica la cantidad de bits por píxel, debe valer 1, 4, 8, 16, 24, or 32
    uint32_t compression; // Compresión, tiene 4 bytes y es un número que indica el tipo de compresión, puede ser 0 (sin compresión), 1 (RLE 8-bit/píxel), 2 (RLE 4-bit/píxel), 3 (bitfields), 4 (JPEG), 5 (PNG)
    uint32_t size_image; // Tamaño de la imagen, tiene 4 bytes y es un número que indica el tamaño de la imagen en bytes
    int32_t x_pixels_per_meter; // Píxeles por metro en el eje X, tiene 4 bytes y es un número que indica la cantidad de píxeles por metro en el eje X
    int32_t y_pixels_per_meter; // Píxeles por metro en el eje Y, tiene 4 bytes y es un número que indica la cantidad de píxeles por metro en el eje Y
    uint32_t colors_used; // Colores usados, tiene 4 bytes y es un número que indica la cantidad de colores usados
    uint32_t colors_important; // Colores importantes, tiene 4 bytes y es un número que indica la cantidad de colores importantes
} BMPInfoHeader;
#pragma pack(pop)

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGBPixel;

typedef struct {
    int width;
    int height;
    RGBPixel *data; // Puntero a los píxeles de la imagen 
} BMPImage;

BMPImage* read_bmp(const char* filename) {
    FILE* file = fopen(filename, "rb"); //rb = read binary
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo.\n");
        return NULL;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, file);
    if (header.type != 0x4D42) { // 42 = 66 en decimal = B en ASCII y 4D = 77 en decimal = M en ASCII
        fprintf(stderr, "Error: El archivo no es un BMP válido.\n");
        fclose(file);
        return NULL;
    }

    BMPInfoHeader info_header;
    fread(&info_header, sizeof(BMPInfoHeader), 1, file); //fread(puntero a la estructura, tamaño de la estructura, cantidad de estructuras, archivo)

    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    image->width = info_header.width;
    image->height = info_header.height;
    image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * info_header.width * info_header.height);

    fseek(file, header.offset, SEEK_SET); // fseek(archivo, desplazamiento, origen desde donde se desplaza SEEK_SET = inicio del archivo, SEEK_CUR = posición actual del archivo, SEEK_END = final del archivo)

    //se hace padding para que la imagen tenga un tamaño múltiplo de 4, esto se hace para que la imagen sea más rápida de leer
    int padding = (4 - (info_header.width * sizeof(RGBPixel)) % 4) % 4; // primero se pasan a bytes los píxeles de la imagen y se calcula el residuo de la división entre 4, si el residuo es 0 no hay padding, si el residuo es 1, 2 o 3 se calcula el padding
    for (int y = info_header.height - 1; y >= 0; y--) {
        for (int x = 0; x < info_header.width; x++) {
            RGBPixel pixel;
            fread(&pixel, sizeof(RGBPixel), 1, file);
            image->data[y * info_header.width + x] = pixel;
        }
        fseek(file, padding, SEEK_CUR);
    }

    fclose(file);
    return image;
}

void free_bmp(BMPImage* image) {
    if (image) {
        free(image->data);
        free(image);
    }
}


//funcion para saturar los colores de la imagen, solo se multiplica el rgb por un factor
BMPImage* saturate_bmp(BMPImage* image, float factor) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];

            // Saturar cada componente de color multiplicándolo por el factor de saturación
            pixel.r = (unsigned char)(pixel.r * factor);
            pixel.g = (unsigned char)(pixel.g * factor);
            pixel.b = (unsigned char)(pixel.b * factor);

            // Asegurarse de que los valores no excedan 255
            pixel.r = (pixel.r > 255) ? 255 : pixel.r;
            pixel.g = (pixel.g > 255) ? 255 : pixel.g;
            pixel.b = (pixel.b > 255) ? 255 : pixel.b;

            new_image->data[y * image->width + x] = pixel;
        }
    }

    return new_image;
}


//funcion para escalar a grises los colores de la imagen
BMPImage* grises_bmp(BMPImage* image) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++)
        {
            RGBPixel pixel = image->data[y * image->width + x];

            // Aplicar la ecuación de luminiscencia para convertir a escala de grises
            unsigned char grayscale = (unsigned char)(pixel.r * 0.3 + pixel.g * 0.59 + pixel.b * 0.11);
            // Asignar el mismo valor a los componentes de rojo, verde y azul para obtener una escala de grises

            pixel.r = grayscale;
            pixel.g = grayscale;
            pixel.b = grayscale;

            new_image->data[y * image->width + x] = pixel;
        }
    }

    return new_image;
}


//funcion para binarizar los colores de la imagen
BMPImage* binarize_bmp(BMPImage* image, float threshold) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            // Calcular el valor promedio de los componentes de color y compararlo con el umbral
            float average = (pixel.r + pixel.g + pixel.b) / 3.0f;
            if (average > threshold) {
                // Si el valor promedio es mayor que el umbral, asignar 1
                pixel.r = pixel.g = pixel.b = 255; // Blanco
            } else {
                // Si el valor promedio es menor o igual al umbral, asignar 0
                pixel.r = pixel.g = pixel.b = 0; // Negro
            }
            new_image->data[y * image->width + x] = pixel;
        }
    }

    return new_image;
}


// Función para determinar si una imagen es "casi negra"
int is_nearly_black(BMPImage* image, float threshold) {
    int total_pixels = image->width * image->height;
    int black_pixels = 0;

    // Contar el número de píxeles negros en la imagen
    for (int i = 0; i < total_pixels; i++) {
        RGBPixel pixel = image->data[i];
        if (pixel.r == 0 && pixel.g == 0 && pixel.b == 0) {
            black_pixels++;
        }
    }

    // Calcular el porcentaje de píxeles negros
    float percentage = (float)black_pixels / total_pixels;

    // Determinar si el porcentaje de píxeles negros es mayor o igual al umbral
    if (percentage >= threshold) {
        return 1; // La imagen es "casi negra"
    } else {
        return 0; // La imagen no es "casi negra"
    }
}



//funcion para guardar la imagen en un archivo
void write_bmp(const char* filename, BMPImage* image) {
    FILE* file = fopen(filename, "wb"); //wb = write binary
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo.\n");
        return;
    }

    BMPHeader header;
    header.type = 0x4D42;
    header.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image->width * image->height * sizeof(RGBPixel);
    header.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader info_header;
    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = image->width;
    info_header.height = image->height;
    info_header.planes = 1;
    info_header.bit_count = 24; // está fijado en 24 en este ejemplo pero puede ser 1, 4, 8, 16, 24 o 32
    info_header.size_image = image->width * image->height * sizeof(RGBPixel);

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info_header, sizeof(BMPInfoHeader), 1, file);

    int padding = (4 - (image->width * sizeof(RGBPixel)) % 4) % 4;
    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            fwrite(&pixel, sizeof(RGBPixel), 1, file);
        }

        RGBPixel padding_pixel = {0};
        fwrite(&padding_pixel, sizeof(RGBPixel), padding, file);
    }

    fclose(file);
}

int main() {
    const char* filename = "rb.bmp";
    BMPImage* image = read_bmp(filename);
    if (!image) {
        return 1;
    }

    printf("Ancho de la imagen: %d\n", image->width);
    printf("Alto de la imagen: %d\n", image->height);

    // Acceder a los píxeles de la imagen
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            printf("Pixel (%d, %d): R=%d, G=%d, B=%d\n", x, y, pixel.r, pixel.g, pixel.b);
        }
    }

    BMPImage* new_image = saturate_bmp(image, 1.3f);
    write_bmp("saturated.bmp", new_image);

    BMPImage* new_image_gris = grises_bmp(image);
    write_bmp("gris.bmp", new_image_gris);

    BMPImage* new_image_binarize = binarize_bmp(image, 0.5f*250.0f);
    write_bmp("binarize.bmp", new_image_binarize);

    int clasificacion = is_nearly_black(image, 0.5f);
    printf("clasificacion: %d\n", clasificacion); 

    free_bmp(image);
    free_bmp(new_image);
    free_bmp(new_image_gris);
    free_bmp(new_image_binarize);
    return 0;
}