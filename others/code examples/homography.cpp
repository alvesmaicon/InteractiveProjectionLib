#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"


#include <stdio.h>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;


const char* liveCaptureHelp =
    "The following hot-keys may be used:\n"
    "  <ESC>, 'q' - quit the program\n"
    "  'c' - start calibrate perspective\n"
    "  'p' - start photometric calibration\n"
    "  'd' - start detecting touch\n"
    "  'u' - switch undistortion on/off\n";


void setPixelCv(cv::Mat& dataMatrix, int i, int j, uchar r, uchar g, uchar b)
{
    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[0] = b;
    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[1] = g;
    dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[2] = r;
}

uchar getRedCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[2];
}

uchar getGreenCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[1];

}

uchar getBlueCv(cv::Mat& dataMatrix, int i, int j)
{
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    return dataMatrix.at<cv::Vec3b>(j%dataMatrix.rows, i%dataMatrix.cols)[0];

}


enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2};





int main( int argc, char** argv )
{


    double V[3][3], A[3][3], F[3], C[3];
    double vetR[3], vetG[3], vetB[3];
    double AV[3][3], AF[3];

    double Baverage = 0, Raverage = 0, Gaverage = 0;
    double S = 0.5;

    Size boardSize, imageSize;
    Mat cameraMatrix, chessboardMatrix, H, cameraView, predictedImage, predictedImageGray, projectedImage, img1, handRegion;
    int i;
    bool undistortImage = false, detectingTouch = false;
    VideoCapture capture;
    int mode = DETECTION, photometric = DETECTION;

    /* Selected camera is usb camera by default*/
    int cameraId = 1;

    int msec = 0, trigger = 2000; /* 2000ms */
    clock_t before, difference;
    int stage = 0;

    vector<Point2f> cornersPattern, cornersCameraView;



    boardSize.width = 9;
    boardSize.height = 6;


    capture.open(cameraId);

    capture >> cameraView;

    /**setting the camera resolution**/

    capture.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 800);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 600);



    if( !capture.isOpened())
        return fprintf( stderr, "Could not initialize video (%d) capture\n",cameraId ), -2;


    /* Code for open chessboard image and find corners*/
    chessboardMatrix = imread("pattern-800x600.png", CV_LOAD_IMAGE_COLOR);
    if (chessboardMatrix.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;


    /* Code for open image*/
    img1 = imread("img2.jpg", CV_LOAD_IMAGE_COLOR);
    if (img1.empty())
        return fprintf( stderr, "Failed imread(): image not found\n"), -2;



    bool foundCornersPattern = findChessboardCorners( chessboardMatrix, boardSize, cornersPattern,
                               CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);

    predictedImage = chessboardMatrix;
    projectedImage = chessboardMatrix;


    namedWindow("Projection", WINDOW_NORMAL);
    imshow("Projection", projectedImage);
    namedWindow("Camera View", WINDOW_NORMAL);


    /* Starting camera loop */
    for(i = 0;; i++)
    {
        Mat view, viewGray;

        if(capture.isOpened())
        {
            capture >> view;
        }

        imageSize = view.size();


        if( mode == CAPTURING )
        {

            bool foundCornersView = findChessboardCorners( view, boardSize, cornersCameraView,
                                    CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);


            if(foundCornersView)
            {
                drawChessboardCorners( view, boardSize, Mat(cornersCameraView), foundCornersView );


                H = findHomography(cornersCameraView, cornersPattern);
                mode = CALIBRATED;

            }

        }

        if( mode == CALIBRATED && undistortImage)
        {
            Mat temp = view.clone();
            /*undistorting camera image view to top center perspective*/
            warpPerspective(temp, view, H, temp.size());

        }

        if ( photometric == CAPTURING )
        {


            capture.set(CAP_PROP_AUTO_EXPOSURE, 0.25); // setting exposure to manual
            capture.set(CAP_PROP_AUTOFOCUS, 0); // turn the autofocus off




            switch (stage)
            {


            // WHITE
            case 1:
            {


                Mat white(600, 800, CV_8UC3, Scalar(255, 255, 255));
                //imshow("Projection", white);

                difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;

                //wait 5 sec
                if ( msec >= trigger )
                {
                    imwrite( "WHITE.jpg", view);

                    for(int i = 0; i < view.size().height; i++)
                    {
                        for(int j = 0; j < view.size().width; j++)
                        {
                            Raverage += getRedCv(view, j, i);
                            Gaverage += getGreenCv(view, j, i);
                            Baverage += getBlueCv(view, j, i);
                        }
                    }

                    double tam = view.size().width * view.size().height;
                    A[0][0] = Raverage/tam;
                    A[1][1] = Gaverage/tam;
                    A[2][2] = Baverage/tam;

                    // Normalizing

                    A[0][0] /= 255;
                    A[1][1] /= 255;
                    A[2][2] /= 255;

                    printf("The matrix A is:\n");
                    for(int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j<3; j++)
                        {
                            printf("%f\t", A[i][j]);
                        }
                        printf("\n");
                    }


                    // reseting averages

                    Raverage = 0;
                    Gaverage = 0;
                    Baverage = 0;

                    printf("Stage 1 finalized\n__________________________________\n");


                    before = clock();
                    stage = 2;


                }
            }
            break;


            // RED
            case 2:
            {

                Mat red(600, 800, CV_8UC3, Scalar(0, 0, 255));
                //imshow("Projection", red);

                difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;

                //wait 5 sec
                if ( msec >= trigger )
                {


                    imwrite( "RED.jpg", view);
                    for(int i = 0; i < view.size().height; i++)
                    {
                        for(int j = 0; j < view.size().width; j++)
                        {
                            Raverage += getRedCv(view, j, i);
                            Gaverage += getGreenCv(view, j, i);
                            Baverage += getBlueCv(view, j, i);



                        }
                    }

                    double tam = view.size().width * view.size().height;
                    vetR[0] = Raverage/tam;
                    vetR[1] = Gaverage/tam;
                    vetR[2] = Baverage/tam;
                    /*
                    printf("The vector vetR is:\n");
                    for(int i = 0; i < 3; i++){
                            printf("%f\t", vetR[i]);
                        }
                    printf("\n");

                    */


                    // reseting averages

                    Raverage = 0;
                    Gaverage = 0;
                    Baverage = 0;

                    printf("Stage 2 finalized\n__________________________________\n");


                    before = clock();
                    stage = 3;


                }
            }
            break;

            // GREEN
            case 3:
            {

                Mat green(600, 800, CV_8UC3, Scalar(0, 255, 0));
                //imshow("Projection", green);

                difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;

                //wait 5 sec
                if ( msec >= trigger )
                {
                    imwrite( "GREEN.jpg", view);
                    for(int i = 0; i < view.size().height; i++)
                    {
                        for(int j = 0; j < view.size().width; j++)
                        {
                            Raverage += getRedCv(view, j, i);
                            Gaverage += getGreenCv(view, j, i);
                            Baverage += getBlueCv(view, j, i);



                        }
                    }

                    double tam = view.size().width * view.size().height;
                    vetG[0] = Raverage/tam;
                    vetG[1] = Gaverage/tam;
                    vetG[2] = Baverage/tam;

                    /*

                    printf("The vector vetG is:\n");
                    for(int i = 0; i < 3; i++){
                            printf("%f\t", vetG[i]);
                        }
                    printf("\n");


                    */

                    // reseting averages

                    Raverage = 0;
                    Gaverage = 0;
                    Baverage = 0;

                    printf("Stage 3 finalized\n__________________________________\n");


                    before = clock();
                    stage = 4;


                }
            }
            break;


            // BLUE
            case 4:
            {

                Mat blue(600, 800, CV_8UC3, Scalar(255, 0, 0));
                //imshow("Projection", blue);

                difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;

                //wait 5 sec
                if ( msec >= trigger )
                {

                    imwrite( "BLUE.jpg", view);
                    for(int i = 0; i < view.size().height; i++)
                    {
                        for(int j = 0; j < view.size().width; j++)
                        {
                            Raverage += getRedCv(view, j, i);
                            Gaverage += getGreenCv(view, j, i);
                            Baverage += getBlueCv(view, j, i);



                        }
                    }

                    double tam = view.size().width * view.size().height;
                    vetB[0] = Raverage/tam;
                    vetB[1] = Gaverage/tam;
                    vetB[2] = Baverage/tam;

                    /*

                    printf("The vector vetB is:\n");
                    for(int i = 0; i < 3; i++){
                            printf("%f\t", vetB[i]);
                        }
                    printf("\n");

                    */


                    // reseting averages

                    Raverage = 0;
                    Gaverage = 0;
                    Baverage = 0;

                    // Normalizing values

                    /*
                    V[0][0] = vetR[0]/vetR[0];
                    V[1][0] = vetR[1]/vetR[0];
                    V[2][0] = vetR[2]/vetR[0];
                    V[0][1] = vetG[0]/vetG[1];
                    V[1][1] = vetG[1]/vetG[1];
                    V[2][1] = vetG[2]/vetG[1];
                    V[0][2] = vetB[0]/vetB[2];
                    V[1][2] = vetB[1]/vetB[2];
                    V[2][2] = vetB[2]/vetB[2];
                    */

                    
                    V[0][0] = vetR[0]/255;
                    V[1][0] = vetR[1]/255;
                    V[2][0] = vetR[2]/255;
                    V[0][1] = vetG[0]/255;
                    V[1][1] = vetG[1]/255;
                    V[2][1] = vetG[2]/255;
                    V[0][2] = vetB[0]/255;
                    V[1][2] = vetB[1]/255;
                    V[2][2] = vetB[2]/255;
                    


                    printf("\nThe matrix V is:\n");
                    for(int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j<3; j++)
                        {
                            printf("%f\t", V[i][j]);
                        }
                        printf("\n");
                    }

                    printf("Stage 4 finalized\n__________________________________\n");


                    /*
                    applying the photometric model to generate predicted image based on projected image
                    C = A(VP + F)
                    firs we need AV matrix and AF vector
                    */


                    for (int i = 0; i < 3; i++)
                    {
                        for(int j = 0; j < 3; j++)
                        {
                            for(int k = 0; k < 3; k++)
                            {
                                AV[i][j] += A[i][k] * V[k][j]; // Getting AV matrix
                            }
                        }
                    }


                    printf("\nThe matrix AV is:\n");
                    for(int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j<3; j++)
                        {
                            printf("%f\t", AV[i][j]);
                        }
                        printf("\n");
                    }


                    for (int i = 0; i < 3; i++)
                    {
                        for(int j = 0; j < 3; j++)
                        {
                            AF[i] += F[i] * A[i][j]; // Getting AF Vector
                        }
                    }


                    printf("The vector AF is:\n");
                    for(int i = 0; i < 3; i++)
                    {
                        printf("%f\t", AF[i]);
                    }
                    printf("\n");



                    photometric = CALIBRATED;

                    projectedImage = img1;
                    imshow("Projection", projectedImage);
                    

                }
            }
            break;

            // BLACK
            default:
            {
                Mat black(600, 800, CV_8UC3, Scalar(0, 0, 0));
                imshow("Projection", img1);

                difference = clock() - before;
                msec = difference * 1000 / CLOCKS_PER_SEC;

                //wait 5 sec
                if ( msec >= trigger )
                {
                    imwrite( "BLACK.jpg", view);
                    for(int i = 0; i < view.size().height; i++)
                    {
                        for(int j = 0; j < view.size().width; j++)
                        {
                            Raverage += getRedCv(view, j, i);
                            Gaverage += getGreenCv(view, j, i);
                            Baverage += getBlueCv(view, j, i);



                        }
                    }

                    double tam = view.size().width * view.size().height;
                    F[0] = Raverage/tam;
                    F[1] = Gaverage/tam;
                    F[2] = Baverage/tam;

                    printf("The vector F is:\n");
                    for(int i = 0; i < 3; i++)
                    {
                        printf("%f\t", F[i]);
                    }
                    printf("\n");

                    // reseting averages

                    Raverage = 0;
                    Gaverage = 0;
                    Baverage = 0;


                    printf("Stage 0 finalized\n__________________________________\n");


                    before = clock();
                    stage = 1;


                }
            }


            } // END SWITCH



        }

        if(detectingTouch)
        {

            // GETTING THE PREDICTED IMAGE

            for(int i = 0; i < view.size().width; i++)
            {
                for(int j = 0; j < view.size().height; j++)
                {

                    double AVP[3] = {0};

                    for(int l = 0; l < 3; l++)
                    {
                        for(int c = 0; c < 3; c++)
                        {
                            AVP[l] += projectedImage.at<Vec3b>(j%projectedImage.rows, i%projectedImage.cols)[2 - l] * AV[l][c];

                        }
                    }

                    int r = int(AVP[0] + AF[0]);
                    int g = int(AVP[1] + AF[1]);
                    int b = int(AVP[2] + AF[2]);

                    if (r > 250)
                        r = 250;
                    if (g > 250)
                        g = 250;
                    if (b > 250)
                        b = 250;



                    predictedImage.at<Vec3b>(j%predictedImage.rows, i%predictedImage.cols)[0] = b;
                    predictedImage.at<Vec3b>(j%predictedImage.rows, i%predictedImage.cols)[1] = g;
                    predictedImage.at<Vec3b>(j%predictedImage.rows, i%predictedImage.cols)[2] = r;

                    //setPixelCv(predictedImage, i, j, r, g, b);



                }
            }

            imshow("PredictedImage", predictedImage);
            /*

            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cvtColor(predictedImage, predictedImageGray, COLOR_BGR2GRAY);

            // GETTING HAND REGION
            
            handRegion = predictedImage;

            double sumColorPredictedImage;
            

            for(int i = 0; i < predictedImageGray.size().height; i++)
                    {
                        for(int j = 0; j < predictedImageGray.size().width; j++)
                        {
                            Raverage += getRedCv(predictedImageGray, j, i);
                            Gaverage += getGreenCv(predictedImageGray, j, i);
                            Baverage += getBlueCv(predictedImageGray, j, i);



                        }
                    }

                    double tam = predictedImageGray.size().width * predictedImageGray.size().height;
                    sumColorPredictedImage = (Raverage/tam + Gaverage/tam + Baverage/tam);


            double aR, aG, aB;




            for(int i = 0; i < viewGray.size().width; i++)
            {
                for(int j = 0; j < viewGray.size().height; j++)
                {

                    int r = predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[2];
                    int g = predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[1];
                    int b = predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[0];

                    if(r != 0){
                        aR = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[2] / r;
                    }
                    else{
                        aR = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[2];
                    }

                    if(g != 0){
                        aG = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[1] / g;
                    }
                    else{
                        aR = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[1];
                    }

                    if(b != 0){
                        aB = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[0] / b;
                    }
                    else{
                       aB = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[0];
                    }


                    /*
                    aR = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[2] / predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[2];
                    aG = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[1] / predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[1];
                    aB = viewGray.at<Vec3b>(j%viewGray.rows, i%viewGray.cols)[0] / predictedImageGray.at<Vec3b>(j%predictedImageGray.rows, i%predictedImageGray.cols)[0];
                    

                    
                    if((aR + aG + aB) < (100)){

                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[0] = 255;
                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[1] = 255;
                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[2] = 255;

                    }
                    else{

                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[0] = 0;
                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[1] = 0;
                        handRegion.at<Vec3b>(j%handRegion.rows, i%handRegion.cols)[2] = 0;
                    }
                }
            }
            


            namedWindow("Hand Region", WINDOW_NORMAL);

            imshow("Hand Region", handRegion);
            */


            // calc the difference
            Mat diff;
            absdiff(view, predictedImage, diff);

            // Get the mask if difference greater than th
            int th = 100;  // 0
            Mat mask(view.size(), CV_8UC1);
            for(int i=0; i<diff.rows; ++i)
            {
                for(int j=0; j<diff.cols; ++j)
                {
                    Vec3b pix = diff.at<Vec3b>(i,j);
                    int val = (pix[0] + pix[1] + pix[2]);
                    if(val>th)
                    {
                        mask.at<unsigned char>(i,j) = 255;
                    }
                }
            }

            // get the foreground
            Mat res;
            bitwise_and(predictedImage, predictedImage, res, mask);

            threshold(res, res, 0, 255, THRESH_BINARY);
            // display
            imshow("res", res);

            
        }




        imshow("Camera View", view);

        char key = (char)waitKey(capture.isOpened() ? 50 : 500);


        if( key == 27 )
            break;

        if( key == 'p' && mode == CALIBRATED && undistortImage && photometric == DETECTION)
        {
            photometric = CAPTURING;
            before = clock();
        }

        if(key == 'd' && photometric == CALIBRATED)
        {
            detectingTouch = !detectingTouch;
        }

        if( key == 'w')
        {
            imwrite( "capturedImage-.jpg", view);
        }

        if( key == 'u' && mode == CALIBRATED )
            undistortImage = !undistortImage;


        if( capture.isOpened() && key == 'c' && mode == DETECTION)
        {
            mode = CAPTURING;
        }



    }
}
/*
-O sistema da projeção interativa é composto por uma câmera, um projetor e uma superfície de projeção
-Para que o sistema funcione corretamente é necessário que a imagem capturada pela câmera esteja com a mesma forma e tamanho que a imagem projetada
-Para isto é usado um método para encontrar pontos correspondentes na imagem projetada e na imagem capturada.
-É projetado um tabuleiro de xadrez 9x6, encontrados os cantos do tabuleiro e da imagem capturada,
e então é feito um ajuste na imagem capturada para que ela fique com a mesma forma e tamanho da imagem projetada
-também é preciso usar uma imagem prevista para fazer comparações, pra isso é necessário fazer uns ajustes de cor em uma cópia da imagem projetada:
de acordo com um modelo fotométrico, a partir da imagem projetada é possível obter uma previsão de como a imagem projetada será capturada pela camera
com os seguintes passos podemos calibrar a imagem prevista:
-projetar uma imagem preta para computar F
    Projetar imagem branca para computar A
    Projetar uma imagem RED e capturar red, green, blue e colocar na coluna 1 da matrix V
    projetar GREEN e capturar red, green e vlue colocar na coluna 2 da matriz V
    projetar BLUE e capturar red green e blue e colocar na coluna 3 da matrz V

Depois de obter as matrizes e vetores, preciso percorrer pixel por pixel cada frame para gerar a imagem prevista
O Modelo fotométrico é C = A(VP + F)
    C é o pixel na imagem prevista,
    A é uma matriz diagonal,
    V é uma matriz com todas as correspondencias de cores,
    P é o ponto no frame,
    e F é a média de luz ambiente
Ainda não sei se eu preciso capturar para cada ponto da imagem ou é possível fazer uma média (ainda vou descobrir).


Preciso de uma tecla para indicar o proximo passo, ou simplesmente dar um tempo x para o programa calcular e passar para o proximo passo
E depois de calcular tudo, a imagem prevista será exibida lado a lado com a imagem capturada da câmera para comparações.
Em seguida preciso fazer a diferença de captura para encontrar a região da mão,
    calcular o contorno,
    encontrar a ponta do dedo,
    separar uma região de interesse em ambas imagens
    desenhar um circulo na ponta do dedo na projeção,
    e comparar as regiões de interesse com metodo orb ou brisk para descobrir se ocorreu um toque ou não
        averiguando a distancia média dos pontos de interesse. tomara que exista uma função pronta pra isso no opencv

    E fim.




Planejamento fotometria
preciso ter um gatilho para trocar de imagem e passar para o proximo passo
seria interessante dizer que o colorido já está calibrado

projetar a cor, esperar 5 segundos para iniciar a calibração atual, pós 5 segundos iniciar captura da cor
quando a captura estiver terminada, projetar a proxima cor e esperar mais 5 segundos até a câmera se adaptar com a nova cor
capturar e passar para o proximo passo







*/
