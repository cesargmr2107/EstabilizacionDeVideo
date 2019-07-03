/* 
 * File:   SISD.c
 * Authors: Cesar Marquez, Matias Garcia.
 *
 * Created on June 1, 2019, 5:51 PM
 */

#define MARGENBUSQUEDA 100


#include <stdint.h> /* for uint64 definition */

#include <stdio.h>
#include <stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <time.h>  

#include <Windows.h>

unsigned int diferenciaBloquesIndicePixel(IplImage* img1, int img1f, int img1c, IplImage* img2, int img2f, int img2c, int alto, int ancho) {

    unsigned int diferencia = 0;

    int fila, columna;

    for (fila = 0; fila < alto; fila++) {

        unsigned char* pImg1 = (unsigned char*) (img1->imageData + (img1f + fila) * img1->widthStep + img1c * img1->nChannels);
        unsigned char* pImg2 = (unsigned char*) (img2->imageData + (img2f + fila) * img2->widthStep + img2c * img2->nChannels);

        for (columna = 0; columna < ancho; columna++) {
            diferencia += abs(*pImg1++ - *pImg2++);
            diferencia += abs(*pImg1++ - *pImg2++);
            diferencia += abs(*pImg1++ - *pImg2++);


        }
    }
    return (diferencia);
}

void desplazarImagen(IplImage *image, int altoDespl, int anchoDespl) {

    int fila, columna;

    if (altoDespl > 0) {
        for (fila = image->height - 1 - altoDespl; fila >= 0; fila--) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep);
            unsigned char* pImgDestino = (unsigned char*) (image->imageData + (fila + altoDespl) * image->widthStep);
            for (columna = 0; columna < image->width; columna++) {
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
            }
        }
    }

    if (altoDespl < 0) {
        for (fila = -altoDespl; fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep);
            unsigned char* pImgDestino = (unsigned char*) (image->imageData + (fila + altoDespl) * image->widthStep);
            for (columna = 0; columna < image->width; columna++) {
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
            }
        }
    }

    if (anchoDespl > 0) {
        for (fila = 0; fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep + (image->width - anchoDespl) * image->nChannels - 1);
            unsigned char* pImgDestino = (unsigned char*) (image->imageData + fila * image->widthStep + image->width * image->nChannels - 1);
            for (columna = image->width - anchoDespl; columna >= 0; columna--) {
                *pImgDestino-- = *pImgOrigen--;
                *pImgDestino-- = *pImgOrigen--;
                *pImgDestino-- = *pImgOrigen--;
            }
        }
    }

    if (anchoDespl < 0) {
        for (fila = 0; fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep + (-anchoDespl) * image->nChannels);
            unsigned char* pImgDestino = (unsigned char*) (image->imageData + fila * image->widthStep);
            for (columna = -anchoDespl; columna < image->width; columna++) {
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
                *pImgDestino++ = *pImgOrigen++;
            }
        }
    }

    if (altoDespl > 0) {
        for (fila = 0; fila < altoDespl; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep);
            for (columna = 0; columna < image->width; columna++) {
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
            }
        }
    }
    if (altoDespl < 0) {
        for (fila = image->height - abs(altoDespl); fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep);
            for (columna = 0; columna < image->width; columna++) {
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
            }
        }
    }

    if (anchoDespl > 0) {
        for (fila = 0; fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep);
            for (columna = 0; columna < anchoDespl; columna++) {
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
            }
        }
    }

    if (anchoDespl < 0) {
        for (fila = 0; fila < image->height; fila++) {
            unsigned char* pImgOrigen = (unsigned char*) (image->imageData + fila * image->widthStep + (image->width - abs(anchoDespl)) * image->nChannels);
            for (columna = 0; columna < abs(anchoDespl); columna++) {
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
                *pImgOrigen++ = 0;
            }
        }
    }
}

int main(int argc, char** argv) {

    // Seleccionamos y cargamos el vídeo mediante una ventana de diálogo

    OPENFILENAME ofn;
    char szFile[100];


    MessageBox(NULL, "Selecciona el Fichero de Video a Estabilizar", "Video File Name", MB_OK);

    ZeroMemory(&ofn, sizeof ( ofn));
    ofn.lStructSize = sizeof ( ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof ( szFile);
    ofn.lpstrFilter = "All\0*.*\0Video AVI\0*.avi;*.mpeg\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    GetOpenFileName(&ofn);

    // Creamos las imagenes a mostrar
    CvCapture* capture = cvCaptureFromAVI(ofn.lpstrFile);

    // Comprobamos que se ha podido crear el archivo
    if (!capture) {
        printf("Error: fichero %s no leido\n", ofn.lpstrFile);
        return EXIT_FAILURE;
    }

    printf("Pulsa una tecla para empezar...\n");
    cvWaitKey(0);

     CvSize size =
            cvSize(
            (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
            (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT)
            );
    
    printf("\nAncho: %d", size.width);
    printf("\nAlto: %d", size.height);

    double fps = cvGetCaptureProperty(
            capture,
            CV_CAP_PROP_FPS
            );
    
    printf("\nFrames por segundo: %f", fps);

    CvVideoWriter *writer = cvCreateVideoWriter("videoSISD.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, size, 1);
    if (!writer) {
        fprintf(stderr, "Cannot create video file!\n");
        return (1);
    }
    
    int filaErrorMinimo, colErrorMinimo;

    IplImage *InFrameFirst = cvCloneImage(cvQueryFrame(capture)),
            *InFrameOld = cvCloneImage(InFrameFirst),
            *InFrameNew;

    /*
        cvShowImage("Original", InFrameFirst);

        InFrameOld = cvCloneImage(InFrameFirst);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, 50, 0);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, -50, 0);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, 0, 50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, 0, -50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);

        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, 50, 50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, 50, -50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, -50, 50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
        InFrameOld = cvCloneImage(InFrameFirst);
        desplazarImagen(InFrameOld, -50, -50);
        cvShowImage("TEST", InFrameOld);
        cvWaitKey(0);
     */

    // Definimos el tamaño de los bloques
    int anchoObjeto = 64, altoObjeto = 64;

    // Establecemos las coordenadas del bloque de referencia.
    int filaBusqueda, colBusqueda;
    filaBusqueda = InFrameFirst->height / 2 - altoObjeto / 2;
    colBusqueda = InFrameFirst->width / 2 - anchoObjeto / 2;

    // Iniciamos el cronómetro para medir el tiempo
    struct timespec start, finish;
    float elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while ((InFrameNew = cvQueryFrame(capture)) != NULL) {

        int fila, col;
        unsigned int diferencia = UINT_MAX;

        for (fila = filaBusqueda - MARGENBUSQUEDA; fila <= filaBusqueda + MARGENBUSQUEDA; fila++) {

            for (col = colBusqueda - MARGENBUSQUEDA; col <= colBusqueda + MARGENBUSQUEDA; col++) {

                unsigned int diferenciaProv;
                diferenciaProv = diferenciaBloquesIndicePixel(InFrameFirst, filaBusqueda, colBusqueda, InFrameNew, fila, col, altoObjeto, anchoObjeto);

                if (diferenciaProv < diferencia) {
                    diferencia = diferenciaProv;
                    filaErrorMinimo = fila;
                    colErrorMinimo = col;
                }

            }
        }
        
/*
        printf("\n Fila %d Col %d comienzo busqueda", filaBusqueda, colBusqueda);
        printf("\n El error en el frame X es en fila %d, col %d, error %d", filaErrorMinimo, colErrorMinimo, diferencia);
        printf("\n Correccion %d, col %d", filaBusqueda - filaErrorMinimo, colBusqueda - colErrorMinimo);
        printf("\n ");
*/

        //cvShowImage("Frame Video", InFrameNew);
        //cvWaitKey(1);
        desplazarImagen(InFrameNew, filaBusqueda - filaErrorMinimo, colBusqueda - colErrorMinimo);
        //cvShowImage("Frame Centrado", InFrameNew);

        
        int r = cvWriteFrame(writer, InFrameNew);
        if (!r) {
            printf("Error\n");
        }
        //if (cvWaitKey(1) >= 0) break;
        
    }
    
    // Detenemos y mostramos cronómetro
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("\nTiempo transcurrido: %f", elapsed);

    printf("Pulsa una tecla para finalizar...\n");
    cvWaitKey(0);
    
    // Liberamos memoria
    cvReleaseCapture(&capture);
    cvReleaseVideoWriter(&writer);

    // Autoexplicativo
    cvDestroyWindow("Frame Video");
    cvDestroyWindow("Frame Centrado");

    return EXIT_SUCCESS;

}

