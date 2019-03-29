#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

bool Porownanie_konturow(vector<Point> kontur1, vector<Point> kontur2){
    double i = fabs(contourArea(Mat(kontur1)));
    double j = fabs(contourArea(Mat(kontur2)));
    return(i<j);
}

int main()
{
    //Wczytanie zdjecia
    string zmienna = "Zdjecia/1.jpg";

    Mat zdjecie;
    zdjecie = imread(zmienna);

    //Utworzenie kopi zapasowej zdjecia
    Mat kopia_zapasowa;
    kopia_zapasowa = zdjecie.clone();

    //Utworzenie kopi na ktorej bede pracowal
    Mat robocze;
    robocze = zdjecie.clone();

    //Przejscie na skale szarosci
    cvtColor(robocze,robocze,CV_BGR2GRAY);

    //Progowanie adaptacyjne
    adaptiveThreshold(robocze,robocze,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,231,5);

    //konstrukcja kolowej maski do otwarcia morfologicznego
    Mat maska;
    maska = getStructuringElement(CV_SHAPE_ELLIPSE,Size(7,7));

    //Utworzenie kopi sprogowanego obrazu
    Mat robocze2;
    robocze2 = robocze.clone();

    //Otwarcie morfologiczne
    morphologyEx(robocze2,robocze2,MORPH_OPEN,maska);

    // pomocnicze
    Mat pomocnicze;
    pomocnicze = robocze2.clone();

    //Znajdowanie konturow
    vector<vector<Point> > kontur;
    findContours(robocze2,kontur,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    //Stworzenie licznikow i ich incjalizacja
    int licznik_konturow;
    int licznik_diod;
    int liczba_diod_zielonych, liczba_diod_czerwonych, liczba_diod_zoltych, liczba_diod_niebieskich;
    int liczba_rezystorow, liczba_uscalonych, liczba_stabilizatorow, liczba_kondensatorow;
    licznik_konturow=0;
    licznik_diod=0;

    liczba_diod_czerwonych=0;
    liczba_diod_niebieskich=0;
    liczba_diod_zielonych=0;
    liczba_diod_zoltych=0;

    liczba_rezystorow=0;
    liczba_stabilizatorow=0;
    liczba_uscalonych=0;
    liczba_kondensatorow=0;

    //Rysowanie wszystkich konturow ,niezbedne do stworzenia maski na potrzeby wykrycia kondesatorow
    for(int i=1; i<kontur.size(); i++){
        double powierzchnia_konturu = contourArea(kontur[i]);
        if(powierzchnia_konturu>300){
            //drawContours(zdjecie,kontur,i,Scalar(0,0,255),2,7);  // rysowanie wszystkich wykrytych konturow
            licznik_konturow++;
        }
    }

    //Sortowanie konturow
    sort(kontur.begin(),kontur.end(),Porownanie_konturow);

    //tworzenie maski na podstawie konturow na obrazie do wykrywania kondesatorow
    Mat mask = Mat::zeros(pomocnicze.rows,pomocnicze.cols,CV_8UC1);
    for(int i=2;i<licznik_konturow+2;i++){
        drawContours( mask, kontur,kontur.size()-i, Scalar(255,0,0), CV_FILLED );
    }
    //tworzenie kopi obrazu na ktory wklejam elementy, biale tlo
    Mat kopia(pomocnicze.rows,pomocnicze.cols,CV_8UC3);
    kopia.setTo(Scalar(255,255,255));

    //Wklejenie obrazu do kopi
    kopia_zapasowa.copyTo(kopia,mask);

    //indeks konturu 5zl
    int indeks;
    indeks=0;

    //Analiza kontorow przy uzyciu wspolczynnika Malinowskiej znajdowanie 5zl, skali
    for(int i=0; i<kontur.size(); i++){
        double S = contourArea(kontur[i]);
        double L = arcLength(kontur[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&(M<=0.25)&&(i!=kontur.size()-1)){
            indeks=i;
        }
    }

    //Za skale obieram pole 5zl
    double skala;
    skala = contourArea(kontur[indeks]);

    //Rysowanie konturu 5zl
    drawContours(zdjecie,kontur,indeks,Scalar(255,0,0),2,7);

    for(int i=0; i<kontur.size(); i++){
        double S = contourArea(kontur[i]);
        double L = arcLength(kontur[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&((M*1000)<=5500)&&(4000<=(M*1000)&&(90000<skala*10000/S)&&(skala*10000/S<1500000))){
            drawContours(zdjecie,kontur,i,Scalar(255,255,255),2,7);  //rezystory
            liczba_rezystorow++;
        }
        if((S>100)&&((M*1000)<=2200)&&(500<=(M*1000))&&(27000<skala*10000/S)&&(skala*10000/S<37000)){
            drawContours(zdjecie,kontur,i,Scalar(0,255,255),2,7); //uklady scalone
            liczba_uscalonych++;
        }
    }

    //utworznie okna
    string nazwa_okna[] = {"Wyswietlacz1","Wyswielacz2","Wyswietlacz3"};
    namedWindow(nazwa_okna[0],CV_WINDOW_AUTOSIZE);


    //Zlczanie niebieskich diod
    Mat zdjecieHSV_diody_niebieskie;
    Mat diody_niebieskie,diody_niebieskie2;
    zdjecieHSV_diody_niebieskie=kopia_zapasowa.clone();
    cvtColor(kopia_zapasowa,zdjecieHSV_diody_niebieskie,COLOR_BGR2HSV);
    inRange(zdjecieHSV_diody_niebieskie,Scalar(83,80,25),Scalar(120,255,255),diody_niebieskie);

    vector<vector<Point> > kontury_diody_niebieskie;
    diody_niebieskie2 = diody_niebieskie.clone();
    findContours(diody_niebieskie,kontury_diody_niebieskie,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_diody_niebieskie.size(); i++){
        double S = contourArea(kontury_diody_niebieskie[i]);
        double L = arcLength(kontury_diody_niebieskie[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&(70000<skala*10000/S)&&(skala*10000/S<150000)&&!(skala*10000/S<60000)&&((150<=(M*1000)&&((M*1000)<=2000)))){
            drawContours(zdjecie,kontury_diody_niebieskie,i,Scalar(150,150,255),2,7);
            liczba_diod_niebieskich++;
        }
    }
    cout << "Diod NIEBIESKICH na zdjeciu jest: " << liczba_diod_niebieskich <<endl;


    //Zlczanie zielonych diod
    Mat zdjecieHSV_diody_zielone;
    Mat diody_zielone,diody_zielone2;
    zdjecieHSV_diody_zielone=kopia_zapasowa.clone();
    cvtColor(kopia_zapasowa,zdjecieHSV_diody_zielone,COLOR_BGR2HSV);
    inRange(zdjecieHSV_diody_zielone,Scalar(50,90,55),Scalar(80,255,200),diody_zielone);

    vector<vector<Point> > kontury_diody_zielone;
    diody_zielone2 = diody_zielone.clone();
    findContours(diody_zielone,kontury_diody_zielone,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_diody_zielone.size(); i++){
        double S = contourArea(kontury_diody_zielone[i]);
        double L = arcLength(kontury_diody_zielone[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&(70000<skala*10000/S)&&(skala*10000/S<150000)&&!(skala*10000/S<60000)&&((150<=(M*1000)&&((M*1000)<=2000)))){
            drawContours(zdjecie,kontury_diody_zielone,i,Scalar(150,150,255),2,7);
            liczba_diod_zielonych++;
        }
    }
    cout << "Diod ZIELONYCH na zdjeciu jest: " << liczba_diod_zielonych <<endl;


    //Zlczanie ZOLTYCH diod
    Mat zdjecieHSV_diody_zolte;
    Mat diody_zolte,diody_zolte2;
    zdjecieHSV_diody_zolte=kopia_zapasowa.clone();
    cvtColor(kopia_zapasowa,zdjecieHSV_diody_zolte,COLOR_BGR2HSV);
    inRange(zdjecieHSV_diody_zolte,Scalar(20,130,85),Scalar(45,255,220),diody_zolte);

    vector<vector<Point> > kontury_diody_zolte;
    diody_zolte2 = diody_niebieskie.clone();
    findContours(diody_zolte,kontury_diody_zolte,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_diody_zolte.size(); i++){
        double S = contourArea(kontury_diody_zolte[i]);
        double L = arcLength(kontury_diody_zolte[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&(70000<skala*10000/S)&&(skala*10000/S<150000)&&!(skala*10000/S<60000)&&((150<=(M*1000)&&((M*1000)<=2000)))){
            drawContours(zdjecie,kontury_diody_zolte,i,Scalar(150,150,255),2,7);
            liczba_diod_zoltych++;
        }
    }
    cout << "Diod ZOLTYCH na zdjeciu jest: " << liczba_diod_zoltych <<endl;


    //Zlczanie CZERWONYCH diod
    Mat zdjecieHSV_diody_czerwone;
    Mat diody_czerwone,diody_czerwone2;
    zdjecieHSV_diody_czerwone=kopia_zapasowa.clone();
    cvtColor(kopia_zapasowa,zdjecieHSV_diody_czerwone,COLOR_BGR2HSV);
    inRange(zdjecieHSV_diody_czerwone,Scalar(10,5,10),Scalar(150,255,190),diody_czerwone);

    vector<vector<Point> > kontury_diody_czerwone;
    diody_czerwone2 = diody_czerwone.clone();
    findContours(diody_czerwone,kontury_diody_czerwone,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_diody_czerwone.size(); i++){
        double S = contourArea(kontury_diody_czerwone[i]);
        double L = arcLength(kontury_diody_czerwone[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&(70000<skala*10000/S)&&(skala*10000/S<150000)&&!(skala*10000/S<60000)&&((150<=(M*1000)&&((M*1000)<=1500)))){
            drawContours(zdjecie,kontury_diody_czerwone,i,Scalar(150,150,255),2,7);
            liczba_diod_czerwonych++;
        }
    }
    cout << "Diod CZERWONYCH na zdjeciu jest: " << liczba_diod_czerwonych <<endl;

    //zliczanie stabilizatorow
    Mat zdjecieHSV_stabilizator;
    Mat stabilizator,stabilizator2;
    zdjecieHSV_stabilizator=kopia_zapasowa.clone();
    cvtColor(kopia_zapasowa,zdjecieHSV_stabilizator,COLOR_BGR2HSV);
    inRange(zdjecieHSV_stabilizator,Scalar(0,0,50),Scalar(255,85,255),stabilizator);

    vector<vector<Point> > kontury_stabilizator;
    stabilizator2 = stabilizator.clone();
    findContours(stabilizator,kontury_stabilizator,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_stabilizator.size(); i++){
        double S = contourArea(kontury_stabilizator[i]);
        double L = arcLength(kontury_stabilizator[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&((M*1000)<=2300)&&(300<=(M*1000)&&(40000<skala*10000/S)&&(skala*10000/S<80000))){
            drawContours(zdjecie,kontury_stabilizator,i,Scalar(150,150,255),2,7);  //stabilizator
            liczba_stabilizatorow++;
        }
    }
    cout << "STABILIZATOROW na zdjeciu jest: " << liczba_stabilizatorow << endl;
    //pyrDown(stabilizator2,stabilizator2);
    //imshow("czerowne",stabilizator2);


    //Zlczanie kondensatorow
    Mat zdjecieHSV_kondesatory;
    Mat kondensatory,kondesatory2;
    zdjecieHSV_kondesatory=kopia.clone();
    cvtColor(kopia,zdjecieHSV_kondesatory,COLOR_BGR2HSV);
    inRange(zdjecieHSV_kondesatory,Scalar(0,0,90),Scalar(100,10,255),kondensatory);

    vector<vector<Point> > kontury_kondensatory;
    kondesatory2 = kondensatory.clone();
    findContours(kondensatory,kontury_kondensatory,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

    for(int i=1; i<kontury_kondensatory.size(); i++){
        double S = contourArea(kontury_kondensatory[i]);
        double L = arcLength(kontury_kondensatory[i], true);
        double M;
        M = L/(2*sqrt(M_PI*S))-1;
        if((S>100)&&((M*1000)<=3000)&&(1000<=(M*1000))&&(5000<skala*10000/S)&&(skala*10000/S<21000)){
            drawContours(zdjecie,kontury_kondensatory,i,Scalar(0,0,255),2,7);
            liczba_kondensatorow++;
        }
    }
    //pyrDown(kondesatory2,kondesatory2);
    //imshow("kondesatory",kondesatory2);

    cout << "KONDENSATOW na zdjeciu jest: " << liczba_kondensatorow <<endl;
    cout << "UKLADOW SCALONYCH na zdjeciu jest: " << liczba_uscalonych <<endl;
    cout << "REZYSTOROW na zdjeciu jest: " << liczba_rezystorow <<endl;

    //zmiana polozenia okna
    moveWindow(nazwa_okna[0],10,10);

    //Dwukrotne zmniejszenie
    pyrDown(zdjecie,zdjecie);
    //pyrDown(zdjecie,zdjecie);

    //Wyswietlenie wyniku
    imshow(nazwa_okna[0],zdjecie);

    int liczba_konturow;
    liczba_konturow=0;
    liczba_konturow=liczba_diod_czerwonych+liczba_diod_niebieskich+liczba_diod_zielonych+liczba_diod_zoltych+liczba_kondensatorow+liczba_uscalonych+liczba_rezystorow+liczba_stabilizatorow;
    cout << "Liczba elementow na zdjeciu: " << liczba_konturow << endl;

    //Czekanie na klawisz
    waitKey(0);
    cout <<endl;

    return 0;
}
