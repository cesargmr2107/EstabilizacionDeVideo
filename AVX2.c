/* 
 * File:   AVX2.c
 * Author: Cesar Marquez, Matias Garcia.
 *
 * Created on June 1, 2019, 5:51 PM
 */

#include <stdio.h>
#include <stdlib.h>

#define MARGENBUSQUEDA 100

#include <stdint.h> /* for uint64 definition */

#include <stdio.h>
#include <stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <time.h>  

#include <Windows.h>

#include <immintrin.h>

/*
 * 
 */

inline int compararBloques(IplImage* img1, int i, int j, IplImage* img2, int k, int l, int alto, int ancho) {

    int diferencia;
    __m256i difReg;
    difReg = _mm256_set1_epi32(0);
    __m256i aux, r1, r2;
    int f1, f2, cc;

    for (f1 = i, f2 = k; f1 < i + alto; f1++, f2++) {
        __m256i *pImg1 = (__m256i *) (img1->imageData + f1 * img1->widthStep + j * img1->nChannels);
        __m256i *pImg2 = (__m256i *) (img2->imageData + f2 * img2->widthStep + l * img2->nChannels);
        for (cc = 0; cc < img1->nChannels * ancho; cc += 32) {
            r1 = _mm256_loadu_si256(pImg1);
            pImg1++;
            r2 = _mm256_loadu_si256(pImg2);
            pImg2++;
            aux = _mm256_sad_epu8(r1, r2);
            difReg = _mm256_add_epi32(aux, difReg);
        }
    }

    int *truco = (int *) (&difReg);
    diferencia += *truco;

    truco++;
    truco++;
    diferencia += *truco;

    truco++;
    truco++;
    diferencia += *truco;

    truco++;
    truco++;
    diferencia += *truco;

    return diferencia;
}

inline void desplazarImagen(IplImage *image, int altoDespl, int anchoDespl) {

    int fila, cc;
    __m256i aux;

    if (altoDespl > 0) { //Mov hacia abajo
        for (fila = image->height - 1 - altoDespl; fila >= 0; fila--) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep);
            __m256i* pImgDestino = (__m256i*) (image->imageData + (fila + altoDespl) * image->widthStep);
            for (cc = 0; cc < image->widthStep; cc += 32) {
                aux = _mm256_loadu_si256(pImgOrigen++);
                _mm256_storeu_si256(pImgDestino++, aux);
            }
        }
    }

    if (altoDespl < 0) { //Mov hacia arriba
        for (fila = -altoDespl; fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep);
            __m256i* pImgDestino = (__m256i*) (image->imageData + (fila + altoDespl) * image->widthStep);
            for (cc = 0; cc < image->widthStep; cc += 32) {
                aux = _mm256_loadu_si256(pImgOrigen++);
                _mm256_storeu_si256(pImgDestino++, aux);
            }
        }
    }

    if (anchoDespl > 0) { //Mov hacia derecha
        for (fila = 0; fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep + (image->width - anchoDespl - 1) * image->nChannels);
            __m256i* pImgDestino = (__m256i*) (image->imageData + fila * image->widthStep + (image->width - 1) * image->nChannels);
            for (cc = anchoDespl * image->nChannels; cc < image->widthStep - 32; cc += 32) {
                aux = _mm256_loadu_si256(pImgOrigen--);
                _mm256_storeu_si256(pImgDestino--, aux);
            }
        }
    }

    if (anchoDespl < 0) { //Mov hacia izquierda
        for (fila = 0; fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep + (-anchoDespl) * image->nChannels);
            __m256i* pImgDestino = (__m256i*) (image->imageData + fila * image->widthStep);
            for (cc = abs(anchoDespl) * image->nChannels; cc < image->widthStep - 32; cc += 32) {
                aux = _mm256_loadu_si256(pImgOrigen++);
                _mm256_storeu_si256(pImgDestino++, aux);
            }
        }
    }

    if (altoDespl > 0) {
        for (fila = 0; fila < altoDespl; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep);
            for (cc = 0; cc < image->widthStep; cc += 32) {
                _mm256_storeu_si256(pImgOrigen++, _mm256_set1_epi32(0));
            }
        }
    }
    if (altoDespl < 0) {
        for (fila = image->height - abs(altoDespl); fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep);
            for (cc = 0; cc < image->widthStep; cc += 32) {
                _mm256_storeu_si256(pImgOrigen++, _mm256_set1_epi32(0));
            }
        }
    }

    if (anchoDespl > 0) {
        for (fila = 0; fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep);
            for (cc = 0; cc < anchoDespl * image->nChannels; cc += 32) {
                _mm256_storeu_si256(pImgOrigen++, _mm256_set1_epi32(0));
            }
        }
    }

    if (anchoDespl < 0) {
        for (fila = 0; fila < image->height; fila++) {
            __m256i* pImgOrigen = (__m256i*) (image->imageData + fila * image->widthStep + (image->width - abs(anchoDespl)) * image->nChannels);
            for (cc = 0; cc < abs(anchoDespl) * image->nChannels; cc += 32) {

                _mm256_storeu_si256(pImgOrigen++, _mm256_set1_epi32(0));
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
    CvCapture* capture = cvCreateFileCapture(ofn.lpstrFile);

    // Comprobamos que se ha podido crear el archivo
    if (!capture) {
        printf("Error: fichero no leido\n");
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

    CvVideoWriter *writer = cvCreateVideoWriter("videoSIMD.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, size, 1);
    if (!writer) {
        fprintf(stderr, "Cannot create video file!\n");
        return (1);
    }

    int filaErrorMinimo, colErrorMinimo;

    IplImage *InFrameFirst = cvCloneImage(cvQueryFrame(capture)),
            *InFrameOld = cvCloneImage(InFrameFirst),
            *InFrameNew;


    /*
        // PRUEBAS DE DESPLAZAMIENTO DE IMAGEN
     
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
                diferenciaProv = compararBloques(InFrameFirst, filaBusqueda, colBusqueda, InFrameNew, fila, col, altoObjeto, anchoObjeto);

                if (diferenciaProv < diferencia) {
                    diferencia = diferenciaProv;
                    filaErrorMinimo = fila;
                    colErrorMinimo = col;
                }

            }
        }
        /*
                printf("\n Fila %d Col %d comienzo busqueda", filaBusqueda, colBusqueda);
                printf("\n El error en el frame X es en fila %d, col %d, error %u", filaErrorMinimo, colErrorMinimo, diferencia);
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

    printf("\nPulsa una tecla para finalizar...\n");
    cvWaitKey(0);

    // Liberamos memoria
    cvReleaseCapture(&capture);
    cvReleaseVideoWriter(&writer);

    // Autoexplicativo
    cvDestroyWindow("Frame Video");
    cvDestroyWindow("Frame Centrado");

    return EXIT_SUCCESS;

}