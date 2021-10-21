#include <stdio.h>
#include <string.h>


void readImgFile(char *path, unsigned char *arr){
    FILE *fp = fopen(path, "rb");
    fread(arr, 10240, 1, fp);  // サイズ512の画像*20
    fclose(fp);
}


void printCharAsBinary(unsigned char img_byte){
    unsigned char mask = 0x80;  // 2進数で10000000
    for(int k = 0; k < 8; k++){
        if(img_byte & mask){
            printf("*");
        }else{
            printf("o");
        }
        // 1ビット右にずらす(unsignedなので右は0埋め)
        // これを8回ずらせば&ですべてのビットに対して1かどうか確認できる
        // ex) 10000000 -> 01000000
        mask = mask >> 1;
    }
}


void printImgArr(unsigned char *arr, int index){
    // 画像サイズは64*64
    for(int i = index*64; i < (index+1)*64; i++){
        for(int j = 0; j < 8; j++){  // charは1byte(=8bit)なのでループは8回でいい(8*8char = 64bit)
            printCharAsBinary(arr[i*8 + j]);
        }
        printf("\n");
    }
}


int main(){
    unsigned char *arr[10240];
    char buff[256];

    // 標準入力からファイルの番号を取得
    int file_num;
    printf("type file number: ");
    scanf("%d", &file_num);
    (void)getchar();  // 後々のgetsのためにscanfで拾えなかった改行を吸っておく

    // imgファイルへのpathを加工
    char *path[256];
    sprintf(path, "./samples/%02d.img", file_num);

    // imgファイルのバイナリを読み込む
    printf("path: %s\n", path);
    readImgFile(path, arr);

    // 文字画像を20個表示
    for(int i = 0; i < 20; i++){
        printImgArr(arr, i);
        if(i < 19){
            printf("\n");
            gets(buff);  // enterを押されるまで待つ
        }
    }
}


/*

oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooo**oooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooooooo***oooooo**oooooooooooooooooooooooooooo
ooooooooooooooooooooooooo****ooo*****ooooooooooooooooooooooooooo
oooooooooooooooooooooooo**************oooooooooooooooooooooooooo
ooooooooooooooooooooo****************ooooooooooooooooooooooooooo
oooooooooooooooo********************oooooooooooooooooooooooooooo
ooooooooooooo*******************oooooooooooooooooooooooooooooooo
oooooooooooo*****************ooooooooooooooooooooooooooooooooooo
oooooooooooo********oo*****ooooooooooooooooooooooooooooooooooooo
ooooooooooooo***oooooo****oooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooo****oooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooo****oooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooo***ooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooo***ooooooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooo****ooooooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooo****ooooooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooo****ooooooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooo****oooooooo*oooooooooooooooooooooooooooooo
oooooooooooooooooooo*****ooooooo***ooooooooooooooooooooooooooooo
oooooooooooooooooooo****oooooooo***ooooooooooooooooooooooooooooo
oooooooooooooooooooo****ooooooo****ooooooooooooooooooooooooooooo
oooooooooooooooooooo****ooooooo**********ooooooooooooooooooooooo
oooooooooooooooooooo****oooooo***************ooooooooooooooooooo
oooooooooooooooooooo****ooooo*****************oooooooooooooooooo
oooooooooooooooooooo****oo*********************ooooooooooooooooo
oooooooooooooooooooo****************************oooooooooooooooo
ooooooooooooooooooooo************ooooooooooo****oooooooooooooooo
ooooooooooooooooooooo***********ooooooooooooo****ooooooooooooooo
oooooooooooooooooooo***********ooooooooooooooo****oooooooooooooo
oooooooooooooooooooo***********ooooooooooooooo****oooooooooooooo
oooooooooooooooooo************oooooooooooooooo****oooooooooooooo
oooooooooooooooooo***********oooooooooooooooooo****ooooooooooooo
ooooooooooooooooo************oooooooooooooooooo****ooooooooooooo
oooooooooooooooo******o*****ooooooooooooooooooo****ooooooooooooo
ooooooooooooooo******oo*****ooooooooooooooooooo****ooooooooooooo
ooooooooooooooo*****ooo*****ooooooooooooooooooo****ooooooooooooo
oooooooooooooo******oo*******oooooooooooooooooo****ooooooooooooo
oooooooooooooo*****ooo********oooooooooooooooo****oooooooooooooo
oooooooooooooo****ooo***********oooooooooooooo****oooooooooooooo
oooooooooooooo****oo*****oo******oooooooooooo*****oooooooooooooo
oooooooooooooo**********oooo******ooooooooooo****ooooooooooooooo
oooooooooooooo**********ooooo*****oooooooooo*****ooooooooooooooo
oooooooooooooo*********oooooooo**ooooooooooo****oooooooooooooooo
ooooooooooooooo******oooooooooooooooooooooo*****oooooooooooooooo
oooooooooooooooo****oooooooooooooooooooooo*****ooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooo*****ooooooooooooooooo
ooooooooooooooooooooooooooooooooooooooooo*****oooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooo*****ooooooooooooooooooo
ooooooooooooooooooooooooooooooooooooooo*****oooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooo*****ooooooooooooooooooooo
ooooooooooooooooooooooooooooooooooooo*****oooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooo*****ooooooooooooooooooooooo
ooooooooooooooooooooooooooooooooooo****ooooooooooooooooooooooooo
ooooooooooooooooooooooooooooooooooo**ooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

*/
