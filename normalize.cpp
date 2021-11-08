#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

using namespace std;

#define IMG_FILE_SIZE_BYTE 512
#define IMG_NUM 20


void printImg(vector<vector<char>> const& img){
    /*
        画像データを表示する
    */

    for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
            cout << img[row][col];
        }
        cout << endl;
    }
    cout << endl;
}


vector<unsigned char> readImgFile(string const& file_path){
    /*
        バイナリファイルからunsigned charでデータを読み取る
    */

    size_t size = filesystem::file_size(file_path);
    ifstream ifs(file_path, ios::in | ios::binary);
    if(!ifs){
        // ファイルを開けなかったら終了
        cout << "file open error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<unsigned char> data(size);
    ifs.read(reinterpret_cast<char *>(&data[0]), size);

    return data;
}


vector<vector<char>> getImgFromImgRawArr(vector<unsigned char> const& img_raw_arr, int index){
    /*
        画像を表すunsigned charの配列から，*とoで表す視覚的に理解できる画像データを生成する
    */

    // 画像サイズは64*64
    vector<vector<char>> img_char_arr(
        IMG_FILE_SIZE_BYTE/8,
        vector<char>(
            IMG_FILE_SIZE_BYTE/8,
            '.'
        )
    );

    unsigned char mask;
    for(int row = index*IMG_FILE_SIZE_BYTE/8; row < (index+1)*IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8/8; col++){  // charは1byte(=8bit)なのでループは8回でいい(8*char8個 = 64bit)
            // ucharのバイナリを見て0,1を*,oとして取り出す
            mask = 0x80;  // 2進数で10000000
            for(int i = 0; i < 8; i++){
                if(img_raw_arr[row*8 + col] & mask){
                    img_char_arr[row - index*IMG_FILE_SIZE_BYTE/8][col*8 + i] = '*';
                }else{
                    img_char_arr[row - index*IMG_FILE_SIZE_BYTE/8][col*8 + i] = 'o';
                }
                // 1ビット右にずらす(unsignedなので右は0埋め)
                // これを8回ずらせば&ですべてのビットに対して1かどうか確認できる
                // ex) 10000000 -> 01000000
                mask = mask >> 1;
            }
        }
    }

    return img_char_arr;
}


void Normalize(vector<vector<char>>& img){
    /*
        画像を端までいっぱいに拡大するして"""上書きする""""
    */

    // オフセット: top(x, y), bottom(x, y)
    vector<pair<int, int>> offsets(
        2,
        make_pair(-1, -1)
    );
    pair<int, int> size;
    bool is_blank_line;
    int target_row, target_col;

    // 横方向のオフセットを探索
    for(int i = 0; i < 2; i++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
            is_blank_line = true;
            for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
                // これをすると上からと下からの両方に対応できる
                target_row = (IMG_FILE_SIZE_BYTE/8-1) * i + (i == 0 ? row : -1*row);
                target_col = (IMG_FILE_SIZE_BYTE/8-1) * i + (i == 0 ? col : -1*col);
                if(img[target_row][target_col] != 'o'){
                    is_blank_line = false;
                    break;
                }
            }
            if(!is_blank_line){
                offsets[i].first = col;
                break;
            }
        }
    }

    // 縦方向のオフセットを探索
    for(int i = 0; i < 2; i++){
        for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
            is_blank_line = true;
            for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
                // これをすると上からと下からの両方に対応できる
                target_row = (IMG_FILE_SIZE_BYTE/8-1) * i + (i == 0 ? row : -1*row);
                target_col = (IMG_FILE_SIZE_BYTE/8-1) * i + (i == 0 ? col : -1*col);
                if(img[target_row][target_col] != 'o'){
                    is_blank_line = false;
                    break;
                }
            }
            if(!is_blank_line){
                offsets[i].second = row;
                break;
            }
        }
    }

    size.first = IMG_FILE_SIZE_BYTE/8 - offsets[0].first - offsets[1].first;
    size.second = IMG_FILE_SIZE_BYTE/8 - offsets[0].second - offsets[1].second;

    vector<vector<char>> new_img(
        IMG_FILE_SIZE_BYTE/8,
        vector<char>(
            IMG_FILE_SIZE_BYTE/8,
            '.'
        )
    );
    int refer_row, refer_col;
    for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
            refer_row = static_cast<int>(
                static_cast<double>(row) * static_cast<double>(size.second) / static_cast<double>(IMG_FILE_SIZE_BYTE/8) + static_cast<double>(offsets[0].second) + 0.5
            );
            refer_col = static_cast<int>(
                static_cast<double>(col) * static_cast<double>(size.first) / static_cast<double>(IMG_FILE_SIZE_BYTE/8) + static_cast<double>(offsets[0].first) + 0.5
            );
            new_img[row][col] = img[refer_row][refer_col];
        }
    }

    img = new_img;
}


int main(){
    int n;
    cout << "type file number: ";
    cin >> n;
    cin.get();  // enter吸収

    // file pathの加工
    ios::fmtflags current_flag = std::cout.flags();  // 現在のcoutの設定を保管
    ostringstream oss;
    oss << "./samples/" << setw(2) << setfill('0') << n << ".img";  // 2桁の0埋め
    string file_path = oss.str();
    cout.flags(current_flag);  // coutの設定を戻す

    // ファイルを読み込み
    vector<unsigned char> img_raw_arr = readImgFile(file_path);
    if(img_raw_arr.size() != IMG_FILE_SIZE_BYTE * IMG_NUM){
        // 画像20枚分のデータを読み込めていなかったら終了
        cout << "file size error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<vector<vector<char>>> images(
        IMG_NUM,
        vector<vector<char>>(
            IMG_FILE_SIZE_BYTE/8,
            vector<char>(
                IMG_FILE_SIZE_BYTE/8,
                '.'
            )
        )
    );
    for(int i = 0; i < IMG_NUM; i++){
        images[i] = getImgFromImgRawArr(img_raw_arr, i);
        Normalize(images[i]);
    }

    for(int i = 0; i < IMG_NUM; i++) printImg(images[i]);

    return 0;
}

/*
ooooooooooooooooooooooo***oooooooooooooooooooooooooooooooooooooo
ooooooooooooooooooooo*****oooooooooo***ooooooooooooooooooooooooo
ooooooooooooooooooooo*******oooo*********ooooooooooooooooooooooo
ooooooooooooooooooo***********************oooooooooooooooooooooo
ooooooooooooooooooo***********************oooooooooooooooooooooo
oooooooooooooo***************************ooooooooooooooooooooooo
oooooo*********************************ooooooooooooooooooooooooo
o*******************************oooooooooooooooooooooooooooooooo
****************************oooooooooooooooooooooooooooooooooooo
*************ooo********oooooooooooooooooooooooooooooooooooooooo
*************ooo********oooooooooooooooooooooooooooooooooooooooo
o*****oooooooooo*******ooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooo*****ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooooo*****ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooo*******ooooooooooooooooooooooooooooooooooooooooooo
oooooooooooooo*******ooooooooooooo**oooooooooooooooooooooooooooo
ooooooooooooo********ooooooooooo*****ooooooooooooooooooooooooooo
ooooooooooooo******ooooooooooooo*****ooooooooooooooooooooooooooo
ooooooooooooo******ooooooooooooo*****ooooooooooooooooooooooooooo
ooooooooooooo******oooooooooooo******ooooooooooooooooooooooooooo
ooooooooooooo******oooooooooooo****************ooooooooooooooooo
ooooooooooooo******oooooooooo*************************oooooooooo
ooooooooooooo******ooooooooo***************************ooooooooo
ooooooooooooo******oooo**********************************ooooooo
ooooooooooooo******oooo**********************************ooooooo
ooooooooooooo**********************************************ooooo
oooooooooooooo********************oooooooooooooooooo*******ooooo
oooooooooooooo******************oooooooooooooooooooooo******oooo
ooooooooooooo******************oooooooooooooooooooooooo*******oo
ooooooooooooo******************oooooooooooooooooooooooo*******oo
oooooooooo*******************oooooooooooooooooooooooooo*******oo
oooooooooo*******************oooooooooooooooooooooooooo*******oo
oooooooooo******************ooooooooooooooooooooooooooooo*******
oooooooo********************ooooooooooooooooooooooooooooo*******
oooooo**********oo********ooooooooooooooooooooooooooooooo*******
ooooo*********oooo********ooooooooooooooooooooooooooooooo*******
ooooo********ooooo********ooooooooooooooooooooooooooooooo*******
ooooo********ooooo********ooooooooooooooooooooooooooooooo*******
ooo**********ooo************ooooooooooooooooooooooooooooo*******
ooo********ooooo*************oooooooooooooooooooooooooo*******oo
ooo*******oooo******************ooooooooooooooooooooooo*******oo
ooo*******ooo********ooo**********oooooooooooooooooooo********oo
ooo****************ooooooo**********oooooooooooooooooo******oooo
ooo****************ooooooooo********oooooooooooooooo********oooo
ooo****************ooooooooo********oooooooooooooooo********oooo
ooo***************ooooooooooooo***oooooooooooooooooo*******ooooo
ooooo*********ooooooooooooooooooooooooooooooooooooo********ooooo
oooooo*******oooooooooooooooooooooooooooooooooooo********ooooooo
ooooooooooooooooooooooooooooooooooooooooooooooooo********ooooooo
ooooooooooooooooooooooooooooooooooooooooooooooo********ooooooooo
ooooooooooooooooooooooooooooooooooooooooooooooo********ooooooooo
oooooooooooooooooooooooooooooooooooooooooooooo********oooooooooo
oooooooooooooooooooooooooooooooooooooooooooo********oooooooooooo
oooooooooooooooooooooooooooooooooooooooooo*********ooooooooooooo
ooooooooooooooooooooooooooooooooooooooooo********ooooooooooooooo
ooooooooooooooooooooooooooooooooooooooo********ooooooooooooooooo
ooooooooooooooooooooooooooooooooooooooo********ooooooooooooooooo
ooooooooooooooooooooooooooooooooooooo*******oooooooooooooooooooo
ooooooooooooooooooooooooooooooooooooo****ooooooooooooooooooooooo
*/